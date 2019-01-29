/*
MIT License

Copyright(c) 2018-2019 megai2

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#include "stdafx.h"

d912pxy_gpu_que::d912pxy_gpu_que(d912pxy_device * dev, UINT iQueues, UINT iMaxCleanupPerSync, UINT iMaxRefernecedObjs, UINT iGrowReferences) : d912pxy_noncom(dev, L"GPU queue"), d912pxy_thread()
{
	mLists = new d912pxy_ringbuffer<d912pxy_gpu_cmd_list*>(iQueues, 0);

	InitializeCriticalSection(&execLock);
		
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	LOG_ERR_THROW2(d912pxy_s(DXDev)->CreateCommandQueue(&desc, IID_PPV_ARGS(&mDXQue)), "can't create command queue on dx12 device");

	mGPUCleanupThread = new d912pxy_gpu_cleanup_thread();

	for (int i = 0; i != iQueues; ++i)	
		mLists->WriteElement(new d912pxy_gpu_cmd_list(dev, mDXQue, iMaxRefernecedObjs, iGrowReferences, iMaxCleanupPerSync, mGPUCleanupThread));	
	mCurrentGPUWork = NULL;

	d912pxy_s(GPUcl) = mLists->GetElement();
}

d912pxy_gpu_que::~d912pxy_gpu_que()
{
	//clear all command lists and exec pending work
	while (mLists->HaveElements()) //but do get->next cuz we using one that is current
	{
		delete mLists->GetElement();
		mLists->Next();
	}

	delete mLists;

	delete mGPUCleanupThread;
}

HRESULT d912pxy_gpu_que::ExecuteCurrentGPUWork(UINT doSwap)
{
	HRESULT ret;

	if (doSwap)
	{
		mSwp->CopyToDXGI(mCurrentGPUWork->GID(CLG_SEQ).Get());
	}

	mCurrentGPUWork->Execute();

	if (doSwap)
		ret = mSwp->AsyncSwapExec();
	else
		ret = D3D_OK;

	mCurrentGPUWork->Signal();

	return ret;
}

HRESULT d912pxy_gpu_que::ExecuteCommandsImm(UINT doSwap)
{
	HRESULT ret = 0;

	WaitForIssuedWorkCompletion();

	WaitForGPU();
	
	//we are commiting our commands list, so we must wait while new one is setup
	m_dev->LockAsyncThreads();

	//execute current command List
	//iterate to next
	//and write back this one to que
	//also make a mark that we executing something on gpu
	mCurrentGPUWork = mLists->GetElement();

	mLists->Next();
	mLists->WriteElement(mCurrentGPUWork);

	d912pxy_s(GPUcl) = mLists->GetElement();

	mGPUCleanupThread->SignalWork();

	ret = ExecuteCurrentGPUWork(doSwap);

	//we have a new list setted up, so we can continue to commit data
	m_dev->UnLockAsyncThreads();

	return ret;
}

HRESULT d912pxy_gpu_que::ExecuteCommands(UINT doSwap)
{
	HRESULT ret = 0;

	WaitForIssuedWorkCompletion();
	
	if (swapAsked != S_OK)
	{
		doSwap = 0;
	}

	//we are commiting our commands list, so we must wait while new one is setup
	m_dev->LockAsyncThreads();

	//execute current command List
	//iterate to next
	//and write back this one to que
	//also make a mark that we executing something on gpu
	mCurrentGPUWork = mLists->GetElement();
	
	mLists->Next();
	mLists->WriteElement(mCurrentGPUWork);

	d912pxy_s(GPUcl) = mLists->GetElement();
		
	mGPUCleanupThread->SignalWork();
		
	if (doSwap)
	{
		swapAsked = doSwap;
		if (mSwp)
		{
			ret = mSwp->AsyncSwapNote();
			if (ret != S_OK)
				swapAsked = 0;
		}
	}

	IssueWork();
	
	//ret = ExecuteCurrentGPUWork(doSwap);
		
	//we have a new list setted up, so we can continue to commit data
	m_dev->UnLockAsyncThreads();

	return ret;
}

void d912pxy_gpu_que::Flush(UINT doSwap)
{		
	ExecuteCommandsImm(doSwap); //exec 

	d912pxy_gpu_cmd_list* preCL = mCurrentGPUWork;

	WaitForGPU(); //and wait

	mGPUCleanupThread->SignalWork();

	//clean all referenced objects from prev frame
	preCL->CleanupAllReferenced();

	//and from this set too
	CleanAllReferenced();
}

void d912pxy_gpu_que::WaitForGPU()
{
	if (mCurrentGPUWork)
	{
		mCurrentGPUWork->Wait();
		mCurrentGPUWork = NULL;
	}
}

void d912pxy_gpu_que::SetPresenter(d912pxy_swapchain * iSwapper)
{	
	mSwp = iSwapper;
}

void d912pxy_gpu_que::EnqueueCleanup(d912pxy_comhandler * obj)
{
	d912pxy_s(GPUcl)->EnqueueCleanup(obj);
}

d912pxy_gpu_cmd_list * d912pxy_gpu_que::GetCommandList()
{
	return mLists->GetElement();
}

void d912pxy_gpu_que::CleanAllReferenced()
{
	mLists->GetElement()->CleanupAllReferenced();
}

void d912pxy_gpu_que::ThreadJob()
{
	swapAsked = ExecuteCurrentGPUWork(swapAsked);
	
	WaitForGPU();

	SignalWorkCompleted();
}

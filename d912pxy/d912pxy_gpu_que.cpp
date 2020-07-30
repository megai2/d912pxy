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

d912pxy_gpu_que::d912pxy_gpu_que() 
{

}

d912pxy_gpu_que::~d912pxy_gpu_que()
{

}

void d912pxy_gpu_que::Init(UINT iMaxCleanupPerSync, UINT iMaxRefernecedObjs, UINT iGrowReferences)
{
	NonCom_Init(L"GPU queue");

	InitThread("d912pxy gpu exec", 0);
	
	gpuExecuteTimeout = d912pxy_s.config.GetValueUI32(PXY_CFG_MISC_GPU_TIMEOUT);
	mLists = new d912pxy_ringbuffer<d912pxy_gpu_cmd_list*>(PXY_INNER_GPU_QUEUE_BUFFER_COUNT, 0);

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	LOG_ERR_THROW2(d912pxy_s.dx12.dev->CreateCommandQueue(&desc, IID_PPV_ARGS(&mDXQue)), "can't create command queue on dx12 device");

	mGPUCleanupThread = new d912pxy_gpu_cleanup_thread();

	for (int i = 0; i != PXY_INNER_GPU_QUEUE_BUFFER_COUNT; ++i)
	{
		mListsArr[i] = new d912pxy_gpu_cmd_list(mDXQue.Get(), iMaxRefernecedObjs, iGrowReferences, iMaxCleanupPerSync, mGPUCleanupThread);
		mLists->WriteElement(mListsArr[i]);
	}
	mCurrentGPUWork = NULL;

	EnableGID(CLG_TOP, PXY_INNER_CLG_PRIO_FIRST);
	EnableGID(CLG_SEQ, PXY_INNER_CLG_PRIO_LAST);

	d912pxy_s.dx12.cl = mLists->GetElement();
}

void d912pxy_gpu_que::UnInit()
{
	//clear all command lists and exec pending work

	mGPUCleanupThread->SignalWork();
	mGPUCleanupThread->IssueItems(NULL, 0);

	LOG_INFO_DTDM("cleanup thread finished");

	while (mLists->HaveElements()) //but do get->next cuz we using one that is current
	{
		delete mLists->GetElement();
		mLists->Next();
	}

	LOG_INFO_DTDM("elements freed");

	delete mLists;

	delete mGPUCleanupThread;

	mDXQue = nullptr;

	Stop();

	d912pxy_noncom::UnInit();
}

HRESULT d912pxy_gpu_que::ExecuteCurrentGPUWork(UINT doSwap)
{
	HRESULT ret;

	if (doSwap)
	{
		mSwp->CopyFrameToDXGI(mCurrentGPUWork->GID(CLG_SEQ));
	}

	mCurrentGPUWork->Execute();

	if (doSwap)
		ret = mSwp->Swap();
	else
		ret = D3D_OK;

	mCurrentGPUWork->Signal();
	
		
	return ret;
}

HRESULT d912pxy_gpu_que::ExecuteCommandsImm(UINT doSwap)
{
	HRESULT ret = 0;

	if (!WaitForExecuteCompletion())
		return S_OK;

	WaitForGPU();

	SwitchCurrentCL();

	ret = ExecuteCurrentGPUWork(doSwap);

	return ret;
}

HRESULT d912pxy_gpu_que::ExecuteCommands(UINT doSwap)
{
	HRESULT ret = 0;
	FRAME_METRIC_EXEC(1)

	if (!WaitForExecuteCompletion())
		return S_OK;
	
	if (swapAsked != S_OK)	
		doSwap = 0;	

	SwitchCurrentCL();

	if (doSwap)
	{
		swapAsked = doSwap;
		if (mSwp)
		{
			ret = mSwp->SwapCheck();
			if (ret != S_OK)
				swapAsked = 0;
		}
	}
		
	IssueWork();

	FRAME_METRIC_EXEC(0)

	return ret;
}

void d912pxy_gpu_que::Flush(UINT doSwap)
{		
	ExecuteCommandsImm(doSwap); //exec 

	d912pxy_gpu_cmd_list* preCL = mCurrentGPUWork;

	WaitForGPU(); //and wait

	//megai2: as DXGI cleanup is not using default com-based cleanup, we kinda safe to skip extra cleanup code
	//but if this needed again for some reason, it must be done with proper d912pxy_s.dx12.cl swapping
	/*
	mGPUCleanupThread->SignalWork();

	//clean all referenced objects from prev frame
	preCL->CleanupAllReferenced();

	//and from this set too
	CleanAllReferenced();
	*/
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
	d912pxy_s.dx12.cl->EnqueueCleanup(obj);
}

d912pxy_gpu_cmd_list * d912pxy_gpu_que::GetCommandList()
{
	return mLists->GetElement();
}

void d912pxy_gpu_que::ThreadJob()
{
	swapAsked = ExecuteCurrentGPUWork(swapAsked);
	
	WaitForGPU();

	SignalWorkCompleted();
}

void d912pxy_gpu_que::EnableGID(d912pxy_gpu_cmd_list_group id, UINT32 prio)
{
	for (int i = 0; i != PXY_INNER_GPU_QUEUE_BUFFER_COUNT; ++i)
	{
		mListsArr[i]->EnableGID(id, prio);
	}
}

void d912pxy_gpu_que::SwitchCurrentCL()
{
	//we are commiting our commands list, so we must wait while new one is setup
	d912pxy_s.dev.LockAsyncThreads();
		
	//execute current command List
	//iterate to next
	//and write back this one to que
	//also make a mark that we executing something on gpu
	mCurrentGPUWork = mLists->GetElement();

	mLists->Next();
	mLists->WriteElement(mCurrentGPUWork);

	d912pxy_s.dx12.cl = mLists->GetElement();

	mGPUCleanupThread->SignalWork();

	//we have a new list setted up, so we can continue to commit data
	d912pxy_s.dev.UnLockAsyncThreads();
}

UINT d912pxy_gpu_que::WaitForExecuteCompletion()
{
	//megai2: this happens if we hit DXGI deadlock or merely draw something more then 5 seconds by default
	//or if we use RenderDoc to capture something
	if (!WaitForIssuedWorkCompletionTimeout(gpuExecuteTimeout))
	{			
		LOG_ERR_DTDM("WaitForExecuteCompletion timeout/deadlock");

		RestartThread();

		SignalWorkCompleted();
		mCurrentGPUWork->Signal();
		WaitForGPU();

		mSwp->ReanimateDXGI();		
	}

	return 1;
}

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

d912pxy_gpu_cmd_list::d912pxy_gpu_cmd_list(d912pxy_device * dev, ID3D12CommandQueue* que, UINT iMaxRefernecedObjs, UINT iGrowReferences, UINT iMaxCleanupPerSync, d912pxy_gpu_cleanup_thread* cleanupThread) : d912pxy_noncom(dev, L"GPU command list")
{
	ID3D12Device* dx12dev = d912pxy_s(DXDev);

	mDXQue = que;
	
	for (int i = 0; i != PXY_INNER_MAX_GPU_CMD_LIST_GROUPS; ++i)
	{
		LOG_ERR_THROW2(dx12dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mALC[i])), "gpu cmd list allocator error");
		LOG_ERR_THROW2(dx12dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mALC[i], NULL, IID_PPV_ARGS(&mCL[i])), "gpu cmd list allocator error");

		wchar_t buf[256];
		wsprintf(buf, L"gpu cmd list %u", i);

		mCL[i]->SetName(buf);
	}
	
	LOG_ERR_THROW2(dx12dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "can't create fence");
	fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	fenceId = 1;

	mGpuRefs = new d912pxy_ringbuffer<d912pxy_comhandler*>(iMaxRefernecedObjs, iGrowReferences);
	mGpuRefsCleanupDepth = iMaxCleanupPerSync;

	mCleanupThread = cleanupThread;
}

d912pxy_gpu_cmd_list::~d912pxy_gpu_cmd_list()
{
	//megai2: be shure to perform all pending ops
	Execute();
	Signal();
	WaitNoCleanup();
	
	CleanupAllReferenced();

	delete mGpuRefs;

	for (int i = 0; i != PXY_INNER_MAX_GPU_CMD_LIST_GROUPS; ++i)
	{
		mCL[i]->Release();
		mALC[i]->Release();
	}
	fence->Release();
}

void d912pxy_gpu_cmd_list::Execute()
{	
	LOG_DBG_DTDM("exec %016llX", this);

	for (int i = 0; i != PXY_INNER_MAX_GPU_CMD_LIST_GROUPS; ++i)
		LOG_ERR_THROW2(mCL[i]->Close(), "can't close command list");

	ID3D12CommandList* const commandLists[PXY_INNER_MAX_GPU_CMD_LIST_GROUPS] = {
		mCL[0],
		mCL[1],
		mCL[2],
		mCL[3],
		mCL[4],
		mCL[5],
		mCL[6],
		mCL[7]
	};

	mDXQue->ExecuteCommandLists(PXY_INNER_MAX_GPU_CMD_LIST_GROUPS, commandLists);
}

void d912pxy_gpu_cmd_list::Wait()
{
	WaitNoCleanup();

	//we have some objects that was possibly referenced by this command list and they was deleted, so we must delete them when this list is finished
	mCleanupThread->IssueItems(this, mGpuRefs->TotalElements());
}

void d912pxy_gpu_cmd_list::WaitNoCleanup()
{
	LOG_DBG_DTDM("wait %016llX", this);

	if (fence->GetCompletedValue() < fenceId)
	{
		LOG_ERR_THROW2(fence->SetEventOnCompletion(fenceId, fenceEvent), "can't set wait event on fence");
		::WaitForSingleObject(fenceEvent, INFINITE);
	}
	++fenceId;

	//all data is processed, reset lists and allocator
	for (int i = 0; i != PXY_INNER_MAX_GPU_CMD_LIST_GROUPS; ++i)
	{
		mALC[i]->Reset();
		mCL[i]->Reset(mALC[i], 0);
	}
}

void d912pxy_gpu_cmd_list::Signal()
{
	const UINT64 fenceVal = fenceId;
	LOG_ERR_THROW2(mDXQue->Signal(fence, fenceVal), "can't set signal on fence");
	
}

void d912pxy_gpu_cmd_list::EnqueueCleanup(d912pxy_comhandler * obj)
{
	mGpuRefs->WriteElement(obj);
}

void d912pxy_gpu_cmd_list::CleanupAllReferenced()
{
	while (mGpuRefs->HaveElements())
	{
		d912pxy_comhandler* obj = mGpuRefs->GetElement();		
		if (!obj->FinalRelease())
		{
			//for multithread holded objects
			mGpuRefs->WriteElement(obj);
		}
		mGpuRefs->Next();
	}
}

void d912pxy_gpu_cmd_list::CleanupReferenced(UINT items)
{
	FRAME_METRIC_CLEANUPS(mGpuRefs->TotalElements())

	UINT cleaned = 0;
	while (mGpuRefs->HaveElements())
	{
		d912pxy_comhandler* obj = mGpuRefs->GetElement();
		if (!obj->FinalRelease())
		{
			//for multithread holded objects			
			mGpuRefs->WriteElement(obj);
		}
		mGpuRefs->Next();
		++cleaned;
		
		if (cleaned >= items)
			return;
	}
}

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

d912pxy_gpu_cmd_list::d912pxy_gpu_cmd_list(d912pxy_device * dev, ComPtr<ID3D12CommandQueue> que, UINT iMaxRefernecedObjs, UINT iGrowReferences, UINT iMaxCleanupPerSync, d912pxy_gpu_cleanup_thread* cleanupThread) : d912pxy_noncom(dev, L"GPU command list")
{
	ID3D12Device1* dx12dev = d912pxy_s(DXDev);

	mDXQue = que;
	
	for (int i = 0; i != PXY_INNER_MAX_GPU_CMD_LIST_GROUPS; ++i)
	{
		LOG_ERR_THROW2(dx12dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mALC[i])), "gpu cmd list allocator error");
		LOG_ERR_THROW2(dx12dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mALC[i].Get(), NULL, IID_PPV_ARGS(&mCL[i])), "gpu cmd list allocator error");

		wchar_t buf[256];
		wsprintf(buf, L"gpu cmd list %u", i);

		mCL[i]->SetName(buf);
	}
	
	LOG_ERR_THROW2(dx12dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "can't create fence");
	fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	fenceId = 1;

	mGpuRefs = new d912pxy_ringbuffer<d912pxy_comhandler*>(iMaxRefernecedObjs, iGrowReferences);
	mGpuRefsCleanupDepth = iMaxCleanupPerSync;

	m_logMetrics = P7_Get_Shared_Telemetry(L"iframe_stats");

	wchar_t buf[255];
	wsprintf(buf, L"gpu cmd refs / %X", this);
	
	m_logMetrics->Create(buf, 0, 7000, iMaxRefernecedObjs, 1, &m_cleanupCntMetrics);

	mCleanupThread = cleanupThread;
}

d912pxy_gpu_cmd_list::~d912pxy_gpu_cmd_list()
{
	//megai2: be shure to perform all pending ops
	Execute();
	Signal();
	Wait();
	mCleanupThread->SignalWork();

	mCleanupThread->IssueItems(this,-1);
	mCleanupThread->SignalWork();
	mCleanupThread->IssueItems(this, 0);
	mCleanupThread->SignalWork();

	delete mGpuRefs;
}

void d912pxy_gpu_cmd_list::Execute()
{	
	LOG_DBG_DTDM("exec %016llX", this);

	for (int i = 0; i != PXY_INNER_MAX_GPU_CMD_LIST_GROUPS; ++i)
		LOG_ERR_THROW2(mCL[i]->Close(), "can't close command list");

	ID3D12CommandList* const commandLists[PXY_INNER_MAX_GPU_CMD_LIST_GROUPS] = {
		mCL[0].Get(),
		mCL[1].Get(),
		mCL[2].Get(),
		mCL[3].Get(),
		mCL[4].Get()
	/*	mCL[5].Get(),
		mCL[6].Get(),
		mCL[7].Get()*/
	};

	mDXQue->ExecuteCommandLists(PXY_INNER_MAX_GPU_CMD_LIST_GROUPS, commandLists);
}

void d912pxy_gpu_cmd_list::Wait()
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
		mCL[i]->Reset(mALC[i].Get(), 0);
	}

	/*
	D3D12_SAMPLE_POSITION spos;
	spos.X = -7;
	spos.Y = -7;

	for (int i = 0; i != PXY_INNER_MAX_GPU_CMD_LIST_GROUPS; ++i)
		mCL[i]->SetSamplePositions(1, 1, &spos);*/

	//we have some objects that was possibly referenced by this command list and they was deleted, so we must delete them when this list is finished
	mCleanupThread->IssueItems(this, mGpuRefs->TotalElements());
}

void d912pxy_gpu_cmd_list::Signal()
{
	const UINT64 fenceVal = fenceId;
	LOG_ERR_THROW2(mDXQue->Signal(fence.Get(), fenceVal), "can't set signal on fence");
	
}

void d912pxy_gpu_cmd_list::EnqueueCleanup(d912pxy_comhandler * obj)
{
	mGpuRefs->WriteElement(obj);
}

void d912pxy_gpu_cmd_list::CleanupAllReferenced()
{
	mCleanupThread->IssueItems(this,-1);
	mCleanupThread->SignalWork();
	mCleanupThread->IssueItems(this, 0);
	mCleanupThread->SignalWork();

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
#ifdef FRAME_METRIC_CLEANUPS
	m_logMetrics->Add(m_cleanupCntMetrics, mGpuRefs->TotalElements());
#endif

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

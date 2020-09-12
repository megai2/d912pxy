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

d912pxy_gpu_cmd_list::d912pxy_gpu_cmd_list(ID3D12CommandQueue* que, UINT iMaxRefernecedObjs, UINT iGrowReferences, UINT iMaxCleanupPerSync, d912pxy_gpu_cleanup_thread* cleanupThread) : d912pxy_noncom( L"GPU command list")
{
	ID3D12Device* dx12dev = d912pxy_s.dx12.dev;

	mDXQue = que;
	
	for (int i = 0; i != PXY_INNER_MAX_GPU_CMD_LIST_GROUPS; ++i)
	{
		LOG_ERR_THROW2(dx12dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCL[i].alc)), "gpu cmd list allocator error");
		LOG_ERR_THROW2(dx12dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCL[i].alc, NULL, IID_PPV_ARGS(&mCL[i].cl)), "gpu cmd list allocator error");

		wchar_t buf[256];
		wsprintf(buf, L"disabled gpu cmd list %u", i);
		LOG_DX_SET_NAME(mCL[i].cl, buf);

		mCLPrio[i] = 99999;
	}

	totalActCLs = 0;
	
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
	
	FinalReferenceCleanup();

	delete mGpuRefs;

	for (int i = 0; i != PXY_INNER_MAX_GPU_CMD_LIST_GROUPS; ++i)
	{
		mCL[i].cl->Release();
		mCL[i].alc->Release();
	}
	fence->Release();
}

void d912pxy_gpu_cmd_list::Execute()
{	
	LOG_DBG_DTDM("exec %016llX", this);

	ID3D12CommandList* execArr[PXY_INNER_MAX_GPU_CMD_LIST_GROUPS];

	for (int i = 0; i != totalActCLs; ++i)
	{
		ID3D12GraphicsCommandList* ccl = mActCL[i].cl;
		LOG_ERR_THROW2(ccl->Close(), "can't close command list");
		execArr[i] = ccl;
	}

	gpuTime.Reset();

	mDXQue->ExecuteCommandLists(totalActCLs, (ID3D12CommandList* const*)execArr);
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

	UINT64 gpuTimeSpent = gpuTime.Elapsed();

	if (gpuTimeSpent > 2000000)
		LOG_ERR_DTDM("GPU cl execution took %lu ms time! Probably TDR crash", gpuTimeSpent / 1000);

#ifdef ENABLE_METRICS
	d912pxy_s.log.metrics.TrackGPUTime(gpuTimeSpent);
#endif

	LOG_DBG_DTDM("GPU wait finished");

	//all data is processed, reset lists and allocator
	for (int i = 0; i != totalActCLs; ++i)
	{
		mActCL[i].alc->Reset();
		mActCL[i].cl->Reset(mActCL[i].alc, 0);
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

void d912pxy_gpu_cmd_list::FinalReferenceCleanup()
{
	auto totalRefs = mGpuRefs->TotalElements();
	while (mGpuRefs->HaveElements())
	{
		d912pxy_comhandler* obj = mGpuRefs->GetElement();		
		LOG_ASSERT(obj->FinalRelease(), "obj->FinalRelease() wrong cleanup");

		if (!totalRefs && mGpuRefs->TotalElements())
			LOG_WARN_DTDM("%u elements still pending in cmd list references", mGpuRefs->TotalElements());
		else
			--totalRefs;

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
		LOG_ASSERT(obj->FinalRelease(), "obj->FinalRelease() wrong cleanup");
		mGpuRefs->Next();
		++cleaned;
		
		if (cleaned >= items)
			return;
	}
}

void d912pxy_gpu_cmd_list::EnableGID(d912pxy_gpu_cmd_list_group id, UINT32 prio)
{
	mActCL[totalActCLs] = mCL[id];
	
	LOG_DX_SET_NAME(mCL[totalActCLs].cl, d912pxy_gpu_cmd_list_group_name[id]);
	mCLPrio[totalActCLs] = prio;

	++totalActCLs;
	

	for (int i = 0; i != totalActCLs; ++i)
	{
		for (int j = i + 1; j != totalActCLs; ++j)
		{
			if (mCLPrio[i] > mCLPrio[j])
			{
				using std::swap;

				swap(mActCL[i], mActCL[j]);
				swap(mCLPrio[i], mCLPrio[j]);
			}
		}
	}
}

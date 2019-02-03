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
#pragma once
#include "stdafx.h"

class d912pxy_gpu_cmd_list :
	public d912pxy_noncom
{

public:
	d912pxy_gpu_cmd_list(d912pxy_device* dev, ComPtr<ID3D12CommandQueue> que, UINT iMaxRefernecedObjs, UINT iGrowReferences, UINT iMaxCleanupPerSync, d912pxy_gpu_cleanup_thread* cleanupThread);
	~d912pxy_gpu_cmd_list();

	void Execute();
	void Wait();
	void Signal();	
	void EnqueueCleanup(d912pxy_comhandler* obj);

	void CleanupAllReferenced();
	void CleanupReferenced(UINT items);

	ComPtr<ID3D12GraphicsCommandList> GID(d912pxy_gpu_cmd_list_group id) { return mCL[id]; }

private:	
	ComPtr<ID3D12GraphicsCommandList> mCL[PXY_INNER_MAX_GPU_CMD_LIST_GROUPS];
	ComPtr<ID3D12CommandAllocator> mALC[PXY_INNER_MAX_GPU_CMD_LIST_GROUPS];
	ComPtr<ID3D12CommandQueue> mDXQue;

	d912pxy_ringbuffer<d912pxy_comhandler*>* mGpuRefs;
	UINT mGpuRefsCleanupDepth;

	ComPtr<ID3D12Fence> fence;
	HANDLE fenceEvent;
	UINT64 fenceId;

	d912pxy_gpu_cleanup_thread* mCleanupThread;
};
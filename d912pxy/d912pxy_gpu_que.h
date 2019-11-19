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

class d912pxy_gpu_que :
	public d912pxy_noncom, public d912pxy_thread
{
public:
	d912pxy_gpu_que();
	~d912pxy_gpu_que();

	void Init(UINT iMaxCleanupPerSync, UINT iMaxRefernecedObjs, UINT iGrowReferences);
	void UnInit();

	HRESULT ExecuteCurrentGPUWork(UINT doSwap);

	HRESULT ExecuteCommandsImm(UINT doSwap);
	HRESULT ExecuteCommands(UINT doSwap);
	void Flush(UINT doSwap);
	void WaitForGPU();

	void SetPresenter(d912pxy_swapchain* iSwapper);

	void EnqueueCleanup(d912pxy_comhandler* obj);

	d912pxy_gpu_cmd_list* GetCommandList();
	
	ComPtr<ID3D12CommandQueue> GetDXQue() { return mDXQue; };

	void ThreadJob();

	void EnableGID(d912pxy_gpu_cmd_list_group id, UINT32 prio);

	void SwitchCurrentCL();

	UINT WaitForExecuteCompletion();

private:
	UINT swapAsked;
	
	d912pxy_swapchain* mSwp;
	d912pxy_gpu_cmd_list* mCurrentGPUWork;
	d912pxy_ringbuffer<d912pxy_gpu_cmd_list*>* mLists;	
	d912pxy_gpu_cmd_list* mListsArr[PXY_INNER_GPU_QUEUE_BUFFER_COUNT];
	
	ComPtr<ID3D12CommandQueue> mDXQue;

	d912pxy_gpu_cleanup_thread* mGPUCleanupThread;

	UINT gpuExecuteTimeout;
};


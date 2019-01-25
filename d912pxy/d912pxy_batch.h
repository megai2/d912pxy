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

class d912pxy_batch : public d912pxy_noncom
{
public:
	d912pxy_batch(d912pxy_device* dev);
	~d912pxy_batch();

	UINT ExecReplay2();
	void ReplayRSIG(UINT64 i1, UINT64 i2, ID3D12GraphicsCommandList* cl);

	void Cleanup();

	void SetShaderConstF(UINT type, UINT start, UINT cnt4, float* data);
	void* SetShaderConstFRewritable(UINT type, UINT start, UINT cnt4, float* data);

	void FrameStart();
	void FrameEnd();

	void GPUCSCpy(intptr_t parBind, ID3D12GraphicsCommandList* cl);

	void PreDIP(ID3D12GraphicsCommandList* cl, UINT bid);
	void PostDIP(ID3D12GraphicsCommandList* cl);

	void SetRSigOnList(d912pxy_gpu_cmd_list_group lstID);
	void GPUWrite(void* src, UINT size, UINT offset);

private:
	void InitCopyCS();

	d912pxy_dheap* heap;

	d912pxy_cbuffer* drawCBuf;
	d912pxy_cbuffer* stream;

	ID3D12Resource* streamR;
	ID3D12Resource* drawCBufR;

	intptr_t mDrawCBufferGPUPtr;

	intptr_t mStreamBaseGPUPtr;
	intptr_t mStreamPointBase;
	void* mStreamBase;
	UINT32 mStreamDivPoint;
	UINT32 stateTransfer[PXY_INNER_BATCH_BUFSZ / 4];

	UINT mStreamOfDlt[2];

	UINT32 mCStreamCnt;

	UINT32* mDataDltRef;
	UINT32 batchNum;
	UINT32 lastBatchCount;

	UINT32 oddFrame;

	ComPtr<ID3D12PipelineState> copyPSO;
	ComPtr<ID3D12RootSignature> copyRS;
};

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

#define PXY_BATCH_GPU_ELEMENT_OFFSET_TEXBINDS 0
#define PXY_BATCH_GPU_ELEMENT_OFFSET_SAMPLERS 8
#define PXY_BATCH_GPU_ELEMENT_OFFSET_SHADER_VARS_VERTEX 16
#define PXY_BATCH_GPU_ELEMENT_OFFSET_SHADER_VARS_PIXEL 272

#define PXY_BATCH_GPU_ELEMENT_COUNT 544
#define PXY_BATCH_GPU_ELEMENT_SIZE 16

#define PXY_BATCH_GPU_DRAW_BUFFER_SIZE (PXY_BATCH_GPU_ELEMENT_COUNT * PXY_BATCH_GPU_ELEMENT_SIZE)
#define PXY_BATCH_GPU_BUFFER_SIZE (PXY_INNER_MAX_IFRAME_BATCH_COUNT * PXY_BATCH_GPU_DRAW_BUFFER_SIZE)

#define PXY_BATCH_STREAM_DATA_SIZE 16
#define PXY_BATCH_STREAM_CONTROL_SIZE 16
#define PXY_BATCH_STREAM_CONTROL_PART_SIZE (PXY_BATCH_GPU_ELEMENT_COUNT * PXY_BATCH_STREAM_CONTROL_SIZE * PXY_INNER_MAX_IFRAME_BATCH_COUNT)
#define PXY_BATCH_STREAM_DATA_PART_SIZE (PXY_BATCH_GPU_ELEMENT_COUNT * PXY_BATCH_STREAM_DATA_SIZE * PXY_INNER_MAX_IFRAME_BATCH_COUNT)
#define PXY_BATCH_STREAM_CONTROL_OFFSET PXY_BATCH_STREAM_DATA_PART_SIZE
#define PXY_BATCH_STREAM_PER_FRAME_SIZE (PXY_BATCH_STREAM_CONTROL_PART_SIZE + PXY_BATCH_STREAM_DATA_PART_SIZE)
#define PXY_BATCH_STREAM_SIZE (PXY_BATCH_STREAM_PER_FRAME_SIZE*2)

#define PXY_BATCH_GPU_THREAD_BLOCK_SHIFT 5
#define PXY_BATCH_GPU_THREAD_BLOCK_FIX 0x20
#define PXY_BATCH_GPU_THREAD_BLOCK_MASK 0x1F

#pragma pack(push, 1)

typedef struct d912pxy_batch_stream_control_entry {
	UINT32 dstOffset;
	union {
		struct {
			UINT32 startBatch;
			UINT32 endBatch;
		};
		UINT64 batchNums;
	};
	UINT32 unused;
} d912pxy_batch_stream_control_entry;

typedef struct d912pxy_batch_stream_data_entry {
	float f0;
	float f1;
	float f2;
	float f3;
} d912pxy_batch_stream_data_entry;

#pragma pack(pop)

class d912pxy_batch : public d912pxy_noncom
{
public:
	d912pxy_batch();
	~d912pxy_batch();

	void Init();

	UINT NextBatch();

	void SetShaderConstF(UINT type, UINT start, UINT cnt4, float* data);
	void GPUWrite(void* src, UINT size, UINT offset);
	void GPUWriteControl(UINT64 si, UINT64 of, UINT64 cnt, UINT64 bn);
	void GPUWriteControlMT(UINT64 si, UINT64 of, UINT64 cnt, UINT64 bn);
	
	void FrameStart();
	void FrameEnd();
	void GPUCSCpy();
	
	void PreDIP(ID3D12GraphicsCommandList* cl, UINT bid);

	void ClearShaderVars();

private:
	void InitCopyCS();

	ID3D12GraphicsCommandList* topCl;
	ID3D12PipelineState* copyPSO;
	ID3D12RootSignature* copyRS;
	UINT copyPSOtype;
		
	d912pxy_cbuffer* buffer;
	d912pxy_cbuffer* stream;

	d912pxy_batch_stream_data_entry* streamData;
	d912pxy_batch_stream_control_entry* streamControl;
	
	UINT streamOfDlt[2];
	UINT32 oddFrame;
		
	UINT32 streamIdx;
	UINT32 batchNum;
	UINT32 lastBatchCount;

	UINT32 mDataDltRef[PXY_BATCH_GPU_ELEMENT_COUNT];
	BYTE stateTransfer[PXY_BATCH_GPU_DRAW_BUFFER_SIZE];	
};

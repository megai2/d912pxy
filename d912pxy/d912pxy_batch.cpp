/*
MIT License

Copyright(c) 2018-2020 megai2

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

d912pxy_batch_buffer::d912pxy_batch_buffer() : d912pxy_folded_buffer<d912pxy_batch_buffer_element, d912pxy_batch_buffer_sub_element>(PXY_BATCH_GPU_TRANSIT_ELEMENTS)
{
}

d912pxy_batch_buffer::~d912pxy_batch_buffer()
{
}

void d912pxy_batch_buffer::Init()
{
	NonCom_Init(L"draw batch buffer");

	doNewBatch = 0;	
	batchNum = 0;	
	forceNewBatch = d912pxy_s.config.GetValueUI32(PXY_CFG_BATCHING_FORCE_NEW_BATCH);

	int maxBatches = d912pxy_s.config.GetValueUI32(PXY_CFG_BATCHING_MAX_BATCHES_PER_IFRAME);
	int maxWritesPerBatch = d912pxy_s.config.GetValueUI32(PXY_CFG_BATCHING_MAX_WRITES_PER_BATCH);

	d912pxy_folded_buffer<d912pxy_batch_buffer_element, d912pxy_batch_buffer_sub_element>::Init(		
		maxBatches,
		maxBatches * maxWritesPerBatch,
		d912pxy_s.config.GetValueUI32(PXY_CFG_REPLAY_THREADS)
	);
}

void d912pxy_batch_buffer::UnInit()
{
	d912pxy_folded_buffer<d912pxy_batch_buffer_element, d912pxy_batch_buffer_sub_element>::UnInit();
	d912pxy_noncom::UnInit();
}

void d912pxy_batch_buffer::SetShaderConstF(UINT type, UINT start, UINT cnt4, float * data)
{
	GPUWrite(data, cnt4, start + ((type != 0) ? PXY_BATCH_GPU_ELEMENT_OFFSET_SHADER_VARS_PIXEL : PXY_BATCH_GPU_ELEMENT_OFFSET_SHADER_VARS_VERTEX));
}

void d912pxy_batch_buffer::GPUWrite(void * src, UINT size, UINT offset)
{
	if (doNewBatch)
	{
		doNewBatch = 0;
		++batchNum;
	}

	AddWrite(
		write_info(batchNum, offset, size), 
		(d912pxy_batch_buffer_sub_element*)src
	);
}

UINT d912pxy_batch_buffer::FinishCurrentDraw()
{	
	if (forceNewBatch)
		return batchNum++;
	else {
		doNewBatch = 1;
		return batchNum;
	}
}

void d912pxy_batch_buffer::FrameStart()
{
	batchNum = 0;
	StartWrite();
	doNewBatch = !forceNewBatch;
}

void d912pxy_batch_buffer::FrameEnd()
{	
	EndWrite(batchNum);	
}

void d912pxy_batch_buffer::Bind(ID3D12GraphicsCommandList * cl, UINT batch)
{
	cl->SetGraphicsRootConstantBufferView(3, GetDevicePtr(batch));
}

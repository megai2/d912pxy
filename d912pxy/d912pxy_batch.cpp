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

d912pxy_batch::d912pxy_batch(d912pxy_device* dev): d912pxy_noncom(dev, L"draw batch")
{
	d912pxy_s(batch) = this;

	streamIdx = 0;
	batchNum = 0;
	lastBatchCount = 0;
	oddFrame = 0;
	
	stream = new d912pxy_cbuffer(dev, PXY_BATCH_STREAM_SIZE, 0, 0);
	buffer = new d912pxy_cbuffer(dev, PXY_BATCH_GPU_BUFFER_SIZE, 0, 0);
	
	streamOfDlt[0] = 0;
	streamOfDlt[1] = PXY_BATCH_STREAM_PER_FRAME_SIZE;
	intptr_t streamBase = stream->HostPtr();

	streamData = (d912pxy_batch_stream_data_entry*)streamBase;
	streamControl = (d912pxy_batch_stream_control_entry*)(streamBase + PXY_BATCH_STREAM_CONTROL_OFFSET);

	ZeroMemory(stateTransfer, PXY_BATCH_GPU_DRAW_BUFFER_SIZE);
	memset(mDataDltRef, 0, PXY_BATCH_GPU_ELEMENT_COUNT * 4);

	InitCopyCS();
}


d912pxy_batch::~d912pxy_batch()
{
	buffer->Release();
	stream->Release();

	copyPSO->Release();
	copyRS->Release();
}

UINT d912pxy_batch::NextBatch()
{
	return batchNum++;
}

void d912pxy_batch::SetShaderConstF(UINT type, UINT start, UINT cnt4, float * data)
{
	GPUWrite(data, cnt4, start + ((type != 0) ? PXY_BATCH_GPU_ELEMENT_OFFSET_SHADER_VARS_PIXEL : PXY_BATCH_GPU_ELEMENT_OFFSET_SHADER_VARS_VERTEX));
}

void d912pxy_batch::FrameStart()
{
	topCl = d912pxy_s(GPUcl)->GID(CLG_TOP);

	memset(mDataDltRef, 0, PXY_BATCH_GPU_ELEMENT_COUNT * 4);
	GPUWrite(stateTransfer, PXY_BATCH_GPU_ELEMENT_COUNT, 0);

	topCl->SetComputeRootSignature(copyRS);
	topCl->SetComputeRootUnorderedAccessView(0, buffer->DevPtr());
}

void d912pxy_batch::FrameEnd()
{
	++batchNum;

	GPUCSCpy();

	oddFrame = !oddFrame;

	intptr_t streamPoint = stream->HostPtr() + streamOfDlt[oddFrame];
	streamData = (d912pxy_batch_stream_data_entry*)streamPoint;
	streamControl = (d912pxy_batch_stream_control_entry*)(streamPoint + PXY_BATCH_STREAM_CONTROL_OFFSET);
	
	lastBatchCount = batchNum - 1;
	batchNum = 0;
}

void d912pxy_batch::GPUCSCpy()
{	
	for (int i = 0; i != PXY_BATCH_GPU_ELEMENT_COUNT; ++i)
	{		
		streamControl[mDataDltRef[i]].endBatch = batchNum;
	}

	if (streamIdx & PXY_BATCH_GPU_THREAD_BLOCK_MASK)
	{
		UINT32 npb = (streamIdx & ~PXY_BATCH_GPU_THREAD_BLOCK_MASK) + PXY_BATCH_GPU_THREAD_BLOCK_FIX;

		for (intptr_t i = streamIdx; i != npb; ++i)
		{
			streamControl[i].batchNums = 0;
		}

		streamIdx = npb;
	}
		
	UINT ofDlt = streamOfDlt[oddFrame];

	stream->IFrameBarrierTrans(0, D3D12_RESOURCE_STATE_COPY_DEST, CLG_TOP);
	
	stream->UploadOffsetNB(topCl, ofDlt, streamIdx * PXY_BATCH_STREAM_DATA_SIZE);
	stream->UploadOffsetNB(topCl, ofDlt + PXY_BATCH_STREAM_CONTROL_OFFSET, streamIdx * PXY_BATCH_STREAM_CONTROL_SIZE);

	buffer->IFrameBarrierTrans2(0, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_GENERIC_READ, topCl);
	topCl->CopyBufferRegion(stream->GetD12Obj(), ofDlt, buffer->GetD12Obj(), PXY_BATCH_GPU_DRAW_BUFFER_SIZE * lastBatchCount, PXY_BATCH_GPU_DRAW_BUFFER_SIZE);
	buffer->IFrameBarrierTrans2(0, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE, topCl);

	stream->IFrameBarrierTrans(0, D3D12_RESOURCE_STATE_GENERIC_READ, CLG_TOP);

	topCl->SetPipelineState(copyPSO);
	topCl->SetComputeRootUnorderedAccessView(1, stream->DevPtr() + ofDlt);
	topCl->Dispatch(streamIdx >> PXY_BATCH_GPU_THREAD_BLOCK_SHIFT, 1, 1);

	buffer->IFrameBarrierTrans2(0, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, topCl);

	streamIdx = 0;
}

void d912pxy_batch::PreDIP(ID3D12GraphicsCommandList* cl, UINT bid)
{
	cl->SetGraphicsRootConstantBufferView(3, buffer->DevPtr()+ PXY_BATCH_GPU_DRAW_BUFFER_SIZE * bid);
}

void d912pxy_batch::GPUWrite(void * src, UINT size, UINT offset)
{
	UINT32* mDataDltRefL = mDataDltRef;
		
	UINT32 bn = batchNum;

	memcpy(&streamData[streamIdx], src, size << 4);

	UINT32 i = offset;
	while (i != (offset + size))
	{
		d912pxy_batch_stream_control_entry* ctl = &streamControl[streamIdx];

		ctl->dstOffset = i;
		ctl->startBatch = bn;

		streamControl[mDataDltRefL[i]].endBatch = bn;				
		mDataDltRefL[i] = streamIdx;

		++streamIdx;
		++i;
	}
	
}

void d912pxy_batch::InitCopyCS()
{

	//copy cs Root signature
	D3D12_ROOT_PARAMETER rootParameters[2] = {
		{ D3D12_ROOT_PARAMETER_TYPE_UAV, {0}, D3D12_SHADER_VISIBILITY_ALL},
		{ D3D12_ROOT_PARAMETER_TYPE_UAV, {0}, D3D12_SHADER_VISIBILITY_ALL}
	};

	rootParameters[0].Descriptor = { 0, 0 };
	rootParameters[1].Descriptor = { 1, 0 };

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = { 2, rootParameters, 0, 0, D3D12_ROOT_SIGNATURE_FLAG_NONE };

	copyRS = m_dev->ConstructRootSignature(&rootSignatureDesc);
	
	//copy cs hlsl code
	d912pxy_shader_replacer* CScodec = new d912pxy_shader_replacer(0, 0, 3);
	d912pxy_shader_code CScode = CScodec->GetCodeCS();
	delete CScodec;

	//copy cs PSO
	D3D12_COMPUTE_PIPELINE_STATE_DESC dsc = { 
		copyRS, 
		{CScode.code, CScode.sz}, 
		0, 
		{NULL, 0}, 
		D3D12_PIPELINE_STATE_FLAG_NONE 
	};

	LOG_ERR_THROW2(d912pxy_s(DXDev)->CreateComputePipelineState(&dsc, IID_PPV_ARGS(&copyPSO)), "CS pso creation err");

	//cleanup
	free(CScode.code);
}

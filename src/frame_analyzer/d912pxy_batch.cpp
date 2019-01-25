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

	mCStreamCnt = 0;
	batchNum = 0;
	lastBatchCount = 0;

	mDataDltRef = (UINT32*)malloc(PXY_INNER_BATCH_BUFSZ_D16 * 4);
	memset(mDataDltRef, 0xFF, PXY_INNER_BATCH_BUFSZ_D16 * 4);

	stream = new d912pxy_cbuffer(dev, 32 * PXY_INNER_BATCH_BUFSZ_D16  * PXY_INNER_MAX_IFRAME_BATCH_COUNT * 2, 0, 0);
	drawCBuf = new d912pxy_cbuffer(dev, PXY_INNER_BATCH_BUFSZ*PXY_INNER_MAX_IFRAME_BATCH_COUNT, 0, 0);

	streamR = stream->GetD12Obj().Get();
	drawCBufR = drawCBuf->GetD12Obj().Get();

	mStreamBaseGPUPtr = streamR->GetGPUVirtualAddress();
	mDrawCBufferGPUPtr = drawCBufR->GetGPUVirtualAddress();

	heap = dev->GetDHeap(PXY_INNER_HEAP_CBV);

	D3D12_GPU_VIRTUAL_ADDRESS basePtr = drawCBuf->GetD12Obj()->GetGPUVirtualAddress();

	oddFrame = 0;

	mStreamDivPoint = PXY_INNER_MAX_IFRAME_BATCH_COUNT * PXY_INNER_BATCH_BUFSZ_D16 * 32;
	mStreamOfDlt[0] = 0;
	mStreamOfDlt[1] = mStreamDivPoint;
	mStreamBase = stream->OffsetWritePoint(0);
	mStreamPointBase = (intptr_t)mStreamBase;

	ZeroMemory(stateTransfer, PXY_INNER_BATCH_BUFSZ);

	InitCopyCS();
}


d912pxy_batch::~d912pxy_batch()
{
	drawCBuf->Release();
	stream->Release();
}

UINT d912pxy_batch::ExecReplay2()
{
	return batchNum++;
}

void d912pxy_batch::ReplayRSIG(UINT64 i1, UINT64 i2, ID3D12GraphicsCommandList* cl)
{
	cl->SetGraphicsRootConstantBufferView(2, drawCBufR->GetGPUVirtualAddress());
	cl->SetGraphicsRootConstantBufferView(3, (drawCBufR->GetGPUVirtualAddress() + PXY_INNER_BATCH_CONSTANT_OFFSET));
	stream->IFrameBarrierTrans2(0, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_GENERIC_READ, cl);
}

void d912pxy_batch::Cleanup()
{

}

void* d912pxy_batch::SetShaderConstFRewritable(UINT type, UINT start, UINT cnt4, float * data)
{
	void* ret = (void*)(mStreamPointBase + mCStreamCnt * 16);

	GPUWrite(data, cnt4, start + type * PXY_INNER_MAX_SHADER_CONSTS_IDX + PXY_INNER_BATCH_CONSTANT_OFFSET_IDX);

	return 0;
}

void d912pxy_batch::SetShaderConstF(UINT type, UINT start, UINT cnt4, float * data)
{
	GPUWrite(data, cnt4, start + type * PXY_INNER_MAX_SHADER_CONSTS_IDX + PXY_INNER_BATCH_CONSTANT_OFFSET_IDX);
}

void d912pxy_batch::FrameStart()
{
	memset(mDataDltRef, 0, PXY_INNER_BATCH_BUFSZ_D16 * 4);
	GPUWrite(stateTransfer, PXY_INNER_BATCH_BUFSZ_D16, 0);
}

void d912pxy_batch::FrameEnd()
{
	++batchNum;

	GPUCSCpy(mCStreamCnt, d912pxy_s(GPUcl)->GID(CLG_TOP).Get());

	oddFrame = !oddFrame;

	mStreamPointBase = (intptr_t)mStreamBase + mStreamOfDlt[oddFrame];

	mCStreamCnt = 0;
	lastBatchCount = batchNum - 1;
	batchNum = 0;
}

void d912pxy_batch::GPUCSCpy(intptr_t parBind, ID3D12GraphicsCommandList * cl)
{
	UINT32* streamUI32 = (UINT32*)(mStreamPointBase);
	UINT bn = batchNum;

	for (int i = 0; i != PXY_INNER_BATCH_BUFSZ_D16; ++i)
	{
		UINT32 dltEp = mDataDltRef[i];
		streamUI32[dltEp + 2] = bn;
	}

	if (parBind & 0x1F)
	{
		UINT32 npb = (parBind & ~0x1F) + 0x20;

		for (intptr_t i = parBind; i != npb; ++i)
		{
			streamUI32[i * 4 + PXY_INNER_MAX_IFRAME_BATCH_COUNT * 4 * PXY_INNER_BATCH_BUFSZ_D16 + 1] = 0;
			streamUI32[i * 4 + PXY_INNER_MAX_IFRAME_BATCH_COUNT * 4 * PXY_INNER_BATCH_BUFSZ_D16 + 2] = 0;
			++mCStreamCnt;
		}

		parBind = npb;
	}

	parBind = parBind >> 5;

	UINT ofDlt = mStreamOfDlt[oddFrame];

	stream->IFrameBarrierTrans(0, D3D12_RESOURCE_STATE_COPY_DEST, CLG_TOP);

	stream->UploadOffsetNB(cl, ofDlt, mCStreamCnt << 4);
	stream->UploadOffsetNB(cl, ofDlt + (PXY_INNER_MAX_IFRAME_BATCH_COUNT * 16 * PXY_INNER_BATCH_BUFSZ_D16), (mCStreamCnt << 4));

#ifdef DX9_FRAME_SHADER_STATE_PERSISTENCY_GPUGPU_COPY	
	drawCBuf->IFrameBarrierTrans2(0, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_GENERIC_READ, cl);

	cl->CopyBufferRegion(streamR, ofDlt, drawCBufR, PXY_INNER_BATCH_BUFSZ * lastBatchCount, PXY_INNER_BATCH_BUFSZ);

	drawCBuf->IFrameBarrierTrans2(0, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE, cl);
#endif

	stream->IFrameBarrierTrans(0, D3D12_RESOURCE_STATE_GENERIC_READ, CLG_TOP);

	cl->SetPipelineState(copyPSO.Get());

	cl->SetComputeRootUnorderedAccessView(1, mStreamBaseGPUPtr + ofDlt);

	cl->Dispatch((UINT)parBind, 1, 1);

	drawCBuf->IFrameBarrierTrans2(0, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, cl);
}

void d912pxy_batch::PreDIP(ID3D12GraphicsCommandList* cl, UINT bid)
{
	cl->SetGraphicsRootConstantBufferView(3, mDrawCBufferGPUPtr + PXY_INNER_BATCH_BUFSZ * bid);
}

void d912pxy_batch::PostDIP(ID3D12GraphicsCommandList* cl)
{

}

void d912pxy_batch::SetRSigOnList(d912pxy_gpu_cmd_list_group lstID)
{
	ID3D12GraphicsCommandList* cl = d912pxy_s(GPUcl)->GID(lstID).Get();

	cl->SetComputeRootSignature(copyRS.Get());
	cl->SetComputeRootUnorderedAccessView(0, mDrawCBufferGPUPtr);
}

void d912pxy_batch::GPUWrite(void * src, UINT size, UINT offset)
{
	intptr_t streamAc = mStreamPointBase;
	UINT32* mDataDltRefL = mDataDltRef;

	UINT32 CStreamCntM4 = mCStreamCnt << 2;
	mCStreamCnt += size;
	UINT32 bn = batchNum;

	memcpy(&((float*)streamAc)[CStreamCntM4], src, size << 4);
#ifdef DX9_FRAME_SHADER_STATE_PERSISTENCY_CPU_TRACKING
	memcpy(&stateTransfer[offset * 4], src, size << 4);
#endif

	CStreamCntM4 += PXY_INNER_MAX_IFRAME_BATCH_COUNT * 4 * PXY_INNER_BATCH_BUFSZ_D16;

	UINT32 i = offset;
	while (i != (offset + size))
	{
		((UINT32*)streamAc)[CStreamCntM4 + 0] = i;
		((UINT32*)streamAc)[CStreamCntM4 + 1] = bn;

		UINT32 oldDlt = mDataDltRefL[i];
		((UINT32*)streamAc)[oldDlt + 2] = bn;

		mDataDltRefL[i] = CStreamCntM4;

		CStreamCntM4 += 4;
		++i;
	}
}

void d912pxy_batch::InitCopyCS()
{

	//copy cs Root signature

	D3D12_DESCRIPTOR_RANGE ranges[1];

	ranges[0].BaseShaderRegister = 0;
	ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	ranges[0].NumDescriptors = 1;//device_heap_config[PXY_INNER_HEAP_CBV].NumDescriptors;
	ranges[0].OffsetInDescriptorsFromTableStart = 0;
	ranges[0].RegisterSpace = 0;

	D3D12_ROOT_PARAMETER rootParameters[4];

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
	rootParameters[0].Descriptor.RegisterSpace = 0;
	rootParameters[0].Descriptor.ShaderRegister = 0;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
	rootParameters[1].Descriptor.RegisterSpace = 0;
	rootParameters[1].Descriptor.ShaderRegister = 1;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
	rootParameters[2].Descriptor.RegisterSpace = 0;
	rootParameters[2].Descriptor.ShaderRegister = 2;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[3].Constants.Num32BitValues = 1;
	rootParameters[3].Constants.RegisterSpace = 0;
	rootParameters[3].Constants.ShaderRegister = 0;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	rootSignatureDesc.NumParameters = 2;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.pStaticSamplers = 0;

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	LOG_ERR_THROW2(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error), "srs batch2");
	LOG_ERR_THROW2(d912pxy_s(DXDev)->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&copyRS)), "crs batch2");

	//copy cs PSO
	D3D12_COMPUTE_PIPELINE_STATE_DESC dsc;

	dsc.pRootSignature = copyRS.Get();

	dsc.CachedPSO.CachedBlobSizeInBytes = 0;
	dsc.CachedPSO.pCachedBlob = NULL;

	d912pxy_shader_replacer* CScodec = new d912pxy_shader_replacer(0, 0, 3);
	d912pxy_shader_code CScode = CScodec->GetCodeCS();

	dsc.CS.BytecodeLength = CScode.sz;
	dsc.CS.pShaderBytecode = CScode.code;

	dsc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	dsc.NodeMask = 0;

	delete CScodec;

	LOG_ERR_THROW2(d912pxy_s(DXDev)->CreateComputePipelineState(&dsc, IID_PPV_ARGS(&copyPSO)), "CS pso creation err");
}

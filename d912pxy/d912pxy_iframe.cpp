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

d912pxy_iframe::d912pxy_iframe(d912pxy_device * dev, d912pxy_dheap** heaps) : d912pxy_noncom(dev, L"iframe")
{
	halfPixelFixWriteAdr = 0;

	mHeaps = heaps;
	
	new d912pxy_texstage_cache(dev);

	new d912pxy_batch(dev);

	new d912pxy_pso_cache(dev);
	
	new d912pxy_sampler_cache(dev, mHeaps[PXY_INNER_HEAP_SPL], PXY_INNER_MAX_CACHE_NODES_SAMPLERS);	

	//mBatches->WriteElement(new d912pxy_batch(m_dev, mGPUque));

	mRBarrierStkPointer = 0;
	batchesIssued = 0;

	streamsActive = 0;

	InitRootSignature();

	mSetHeapArrCnt = 0;

	batchCommisionDF = 7;

	instanceCount = 0;

	for (int i = 0; i != PXY_INNER_MAX_DSC_HEAPS; ++i)
	{
		if (mHeaps[i]->GetDesc()->Type > 1)
			continue;

		mSetHeapArr[mSetHeapArrCnt] = mHeaps[i]->GetHeapObj().Get();
		++mSetHeapArrCnt;
	}

	mCurrentFrameIndex = 0;
}

d912pxy_iframe::~d912pxy_iframe()
{
	delete d912pxy_s(batch);

	delete d912pxy_s(samplerState);
	delete d912pxy_s(textureState);
	delete d912pxy_s(psoCache);
}

void d912pxy_iframe::RBarrierImm(D3D12_RESOURCE_BARRIER * bar)
{
	d912pxy_s(GPUcl)->GID(CLG_SEQ)->ResourceBarrier(0, bar);
}

void d912pxy_iframe::RBarrierStk(UINT cnt, D3D12_RESOURCE_BARRIER * bar)
{	
	d912pxy_s(GPUcl)->GID(CLG_TOP)->ResourceBarrier(cnt, bar);
	//wait, we need 3 stacks for prep-copy-done, so just pass this to top list for now
	/*
	for (int i = 0; i != cnt; ++i)
	{
		mRBarrierStkData[mRBarrierStkPointer] = bar[i];
		++mRBarrierStkPointer;

		if (mRBarrierStkPointer >= PXY_INNER_RBSTACK_SIZE)
			LOG_ERR_THROW2(-1, "(mRBarrierStkPointer > PXY_INNER_RBSTACK_SIZE)");
	}*/	
}

void d912pxy_iframe::SetStreamFreq(UINT StreamNumber, UINT Divider)
{
	streamBinds[StreamNumber].divider = Divider;
	
	/*if (Divider == 1)
	{
		instanceCount = 0;
	}

	if (Divider & D3DSTREAMSOURCE_INDEXEDDATA)
	{
		instanceCount = (0x3FFFFFFF & Divider);
		//streamBinds[StreamNumber].divider = 1;
	}
	else if (Divider & D3DSTREAMSOURCE_INSTANCEDATA)
	{
		instanceDataStream = StreamNumber;

		//streamBinds[StreamNumber].divider = 2;
		
		useInstanced = d912pxy_s(psoCache)->GetIAFormat()->GetInstancedModification();
		//i belive that unmasked value must be equal to binded stream stride, so we just check that our vdecl is ready for this call
		useInstanced->ModifyStreamElementType(StreamNumber, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA);

		d912pxy_s(psoCache)->IAFormat(useInstanced);
	}*/
}

void d912pxy_iframe::SetVBuf(d912pxy_vbuf * vb, UINT StreamNumber, UINT OffsetInBytes, UINT Stride)
{
	if (vb && !streamBinds[StreamNumber].buffer)
		++streamsActive;
	else if (!vb && streamsActive)
	{
		//d912pxy_s(GPUcl)->GID(CLG_SEQ)->IASetVertexBuffers(StreamNumber, 0, 0);
		--streamsActive;
	}

	batchCommisionDF |= 1;

	streamBinds[StreamNumber].buffer = (d912pxy_vbuf*)vb;
	streamBinds[StreamNumber].offset = OffsetInBytes;
	streamBinds[StreamNumber].stride = Stride;

	if (vb)
		d912pxy_s(CMDReplay)->VBbind(vb, Stride, StreamNumber, OffsetInBytes);

	//do this later and handle resource change midframe 
	//streamBinds[StreamNumber].buffer->IFrameBind(Stride, StreamNumber);
}

void d912pxy_iframe::SetIBuf(d912pxy_ibuf * ib)
{
	indexBind = ib;
	//batchCommisionDF |= 2;
	if (ib)
		d912pxy_s(CMDReplay)->IBbind((d912pxy_ibuf*)ib);
}

d912pxy_ibuf * d912pxy_iframe::GetIBuf()
{
	return indexBind;
}

d912pxy_device_streamsrc d912pxy_iframe::GetStreamSource(UINT StreamNumber)
{
	return streamBinds[StreamNumber];
}

void d912pxy_iframe::CommitBatch(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	DWORD pperprim[] = {
		0,
		1,//point
		2,//linelist
		1,//linestrip
		3,//trilist
		1//tristrip		
	};

	DWORD primsubs[] = {
		0,
		0,//point
		0,//linelist
		1,//linestrip
		0,//trilist
		2//tristrip		
	};

	//bind vb/ib
	UINT32 batchDF = batchCommisionDF;
	batchCommisionDF = 0;

	if (batchesIssued >= (PXY_INNER_MAX_IFRAME_BATCH_COUNT - 1))
	{
		m_log->P7_ERROR(LGC_DEFAULT, L"batches in one frame exceeded PXY_INNER_MAX_IFRAME_BATCH_COUNT, performing queued commands now");

		if (indexBind)
			indexBind->GetBase()->ThreadRef(1);

		for (int i = 0; i != streamsActive; ++i)
			if (streamBinds[i].buffer)
				streamBinds[i].buffer->GetBase()->ThreadRef(1);
						
		End();
		d912pxy_s(GPUque)->Flush(0);

		Start();

		if (indexBind)
		{
			indexBind->GetBase()->ThreadRef(-1);
			SetIBuf(indexBind);
		}

		for (int i = 0; i != streamsActive; ++i)
			if (streamBinds[i].buffer)
			{
				streamBinds[i].buffer->GetBase()->ThreadRef(-1);
				SetVBuf(streamBinds[i].buffer, i, streamBinds[i].offset, streamBinds[i].stride);
			}
				
		//megai2: force dirty to rebind all states
		batchDF = 7;
	}
	
	d912pxy_s(textureState)->Use();

	if (batchDF & 1)
	{
		for (int i = 0; i != streamsActive; ++i)
		{
			d912pxy_vbuf* sb = streamBinds[i].buffer;
			if (sb)
			{
				if (streamBinds[i].divider & D3DSTREAMSOURCE_INDEXEDDATA)
				{
					instanceCount = 0x3FFFFFFF & streamBinds[i].divider;
				}

				if (streamBinds[i].divider & D3DSTREAMSOURCE_INSTANCEDATA)
				{
					d912pxy_vdecl* useInstanced = d912pxy_s(psoCache)->GetIAFormat()->GetInstancedModification();
					//i belive that unmasked value must be equal to binded stream stride, so we just check that our vdecl is ready for this call
					useInstanced->ModifyStreamElementType(i, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA);
					d912pxy_s(psoCache)->IAFormatInstanced(useInstanced);
					batchDF |= 8;
				}
			}
		}
	}

	if (batchDF & 4)
	{
		bindedRTV = &bindedSurfacesDH[1];
		bindedDSV = &bindedSurfacesDH[0];

		if (bindedSurfaces[1])
		{
			d912pxy_s(psoCache)->RTVFormat(bindedSurfaces[1]->GetSRVFormat(), 0);
			d912pxy_s(CMDReplay)->ViewTransit(bindedSurfaces[1], D3D12_RESOURCE_STATE_RENDER_TARGET);
			bindedRTVcount = 1;
		}
		else {
			d912pxy_s(psoCache)->RTVFormat(DXGI_FORMAT_UNKNOWN, 0);
			bindedRTVcount = 0;
			bindedRTV = 0;
		}

		if (bindedSurfaces[0])
		{
			d912pxy_s(psoCache)->DSVFormat(bindedSurfaces[0]->GetDSVFormat());
			d912pxy_s(CMDReplay)->ViewTransit(bindedSurfaces[0], D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}
		else {
			bindedDSV = 0;
		}

		d912pxy_s(psoCache)->OMReflect(bindedRTVcount, bindedDSV);

		if (bindedRTV && bindedDSV)
			d912pxy_s(CMDReplay)->RT(bindedSurfaces[1], bindedSurfaces[0]);
		else if (bindedRTV)
			d912pxy_s(CMDReplay)->RT(bindedSurfaces[1], 0);
		else if (bindedDSV)
			d912pxy_s(CMDReplay)->RT(0, bindedSurfaces[0]);
	}

	d912pxy_s(psoCache)->Use();

	d912pxy_s(CMDReplay)->DIIP(primCount*pperprim[PrimitiveType] + primsubs[PrimitiveType], instanceCount, startIndex, BaseVertexIndex, d912pxy_s(batch)->ExecReplay2());

	//halfPixelFixWriteAdr = 0;

	instanceCount = 1;

	++batchesIssued;

	if (batchDF & 8)
	{
		d912pxy_s(psoCache)->IAFormat(d912pxy_s(psoCache)->GetIAFormat());
	}

	d912pxy_s(CMDReplay)->IssueWork(batchesIssued);
}

void d912pxy_iframe::TransitZBufferRW(int write)
{

}

void d912pxy_iframe::BindSurface(UINT index, d912pxy_surface* obj)
{
	//TODO check DSV+RTV compatibility

	if ((index > 0) && obj)
	{
		D3DSURFACE_DESC rtDsc = obj->GetDX9DescAtLevel(0);

		if (rtDsc.Format == D3DFMT_NULL)
			obj = NULL;
	/*	else 
		{
			D3D12_VIEWPORT newVP;
			newVP = main_viewport;
			newVP.TopLeftX = 0;
			newVP.TopLeftY = 0;
			newVP.Height = rtDsc.Height;
			newVP.Width = rtDsc.Width;

			SetViewport(&newVP);
		}*/
	}

	if (obj)
		bindedSurfacesDH[index] = obj->GetDHeapHandle();
	
	bindedSurfaces[index] = obj;
	batchCommisionDF |= 4;

/*	bindedRTV = &bindedSurfacesDH[1];
	bindedDSV = &bindedSurfacesDH[0];

	if (obj)
	{
		if (index > 0)
		{
			obj = obj->CheckRTV();
			D3DSURFACE_DESC rtDs = obj->GetDX9DescAtLevel(0);

			//dx9 fourcc NULL fmt to trick API on allowing setting no RT
			if (rtDs.Format == 0x4C4C554E)
			{
				bindedRTV = 0;
				bindedRTVcount = 0;
				d912pxy_s(psoCache)->RTVFormat(DXGI_FORMAT_UNKNOWN, 0);
				LOG_ERR_THROW2(-1 * (index > 1), "FOURCC NULL fmt for RT with idx > 0");
				obj = NULL;
				goto dx9null_trick;
			} else if (bindedSurfaces[0])
			{
				//we are setting RTV so DSV must comply it or go away				
				D3DSURFACE_DESC dsDs = bindedSurfaces[0]->GetDX9DescAtLevel(0);
				if ((rtDs.Width > dsDs.Width) || (rtDs.Height > dsDs.Height))
					bindedDSV = 0;//GOAWAY =(
				//but we do it in a way that old DSV will pop back in when it comply to setted RT
				//maybe DX9 don't have such a thing about depth buffers, so maybe we need to create and replace old one? duh			
			}	

			if (rtDs.Format != 0x4C4C554E)
			{
				//mRPL->ViewTransit(obj, D3D12_RESOURCE_STATE_RENDER_TARGET);
				//obj->PerformViewTypeTransit(D3D12_RESOURCE_STATE_RENDER_TARGET);

				d912pxy_s(psoCache)->RTVFormat(obj->GetSRVFormat(), index - 1);

				bindedRTVcount = index;
			}
		} else {
			//we are setting DSV that must comply with RT
			//but guess we remove RT if it not comply

			D3DSURFACE_DESC dsDs = obj->GetDX9DescAtLevel(0);

			//check it with zero
			if (bindedSurfaces[1])
			{
				//we are setting DSV so RTV must comply it or go away				
				D3DSURFACE_DESC rtDs = bindedSurfaces[1]->GetDX9DescAtLevel(0);
				if ((rtDs.Width > dsDs.Width) || (rtDs.Height > dsDs.Height))
				{
					bindedRTV = 0;//GOAWAY =(
					bindedRTVcount = 0;
				}
			}

			//mRPL->ViewTransit(obj, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			//obj->PerformViewTypeTransit(D3D12_RESOURCE_STATE_DEPTH_WRITE);			

			d912pxy_s(psoCache)->DSVFormat(obj->GetDSVFormat());
		}
				
		bindedSurfacesDH[index] = obj->GetDHeapHandle();		
	}
	else {
dx9null_trick:
		if (!index)
			bindedDSV = 0;
		else {
			bindedRTVcount = index-1;
			if (bindedRTVcount == 0)
				bindedRTV = 0;
		}
	}

	bindedSurfaces[index] = obj;

	if (!bindedSurfaces[0])
	{
		bindedDSV = 0;
	}
	else {		
		d912pxy_s(CMDReplay)->ViewTransit(bindedSurfaces[0], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}

	if (!bindedSurfaces[1])
	{
		bindedRTV = 0;
		bindedRTVcount = 0;
	}
	else {
		d912pxy_s(CMDReplay)->ViewTransit(bindedSurfaces[1], D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
	
	d912pxy_s(psoCache)->OMReflect(bindedRTVcount, bindedDSV);

	if (bindedRTV && bindedDSV)
		d912pxy_s(CMDReplay)->RT(bindedSurfaces[1], bindedSurfaces[0]);
	else if (bindedRTV)
		d912pxy_s(CMDReplay)->RT(bindedSurfaces[1], 0);
	else if (bindedDSV)
		d912pxy_s(CMDReplay)->RT(0, bindedSurfaces[0]);

		

	//mGPUcl->GID(CLG_SEQ)->OMSetRenderTargets(bindedRTVcount, bindedRTV, 0, bindedDSV);*/
}

void d912pxy_iframe::ClearBindedSurfaces()
{
	bindedSurfaces[0] = 0;
	bindedSurfaces[1] = 0;
}

void d912pxy_iframe::InstancedVDecl(d912pxy_vdecl * src)
{
	if (instanceCount)
	{
		d912pxy_vdecl* useInstanced = src->GetInstancedModification();
		//i belive that unmasked value must be equal to binded stream stride, so we just check that our vdecl is ready for this call
		//useInstanced->ModifyStreamElementType(instanceDataStream, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA);

		src = useInstanced;
	}
	
	d912pxy_s(psoCache)->IAFormat(src);	
}

void d912pxy_iframe::Start()
{
	LOG_DBG_DTDM2("Start Frame %u", mCurrentFrameIndex);

	for (int i = 0; i != PXY_INNER_MAX_DSC_HEAPS; ++i)
		mHeaps[i]->CleanupSlots(PXY_INNER_MAX_DHEAP_CLEANUP_PER_SYNC);

	d912pxy_s(psoCache)->MarkDirty(0);

	d912pxy_s(CMDReplay)->Start();

	if (mSwapChain)
		mSwapChain->StartFrame();

	d912pxy_s(batch)->FrameStart();

	SetViewport(&main_viewport);
	//SetScissors(&main_scissor);

	SetRSigOnList(CLG_TOP);
	SetRSigOnList(CLG_SEQ);

	batchesIssued = 0;
}

void d912pxy_iframe::End()
{
	//d912pxy_s(CMDReplay)->Finish();

	/*if (mRBarrierStkPointer)
		d912pxy_s(GPUcl)->GID(CLG_TOP)->ResourceBarrier(mRBarrierStkPointer, mRBarrierStkData);
	mRBarrierStkPointer = 0;*/

	/*for (int i = 0; i != streamsActive; ++i)
		streamBinds[i].buffer = NULL;

	indexBind = NULL;*/

	d912pxy_s(batch)->FrameEnd();

	if (mSwapChain)
		mSwapChain->EndFrame();

	LOG_DBG_DTDM2("End Frame %u", mCurrentFrameIndex);
	++mCurrentFrameIndex;
	
}

void d912pxy_iframe::EndSceneReset()
{
	//SetViewport(&main_viewport);
}

void d912pxy_iframe::SetViewport(D3D12_VIEWPORT * pViewport)
{
	if ((pViewport->Width != main_viewport.Width) || (pViewport->Height != main_viewport.Height))
	{
		float fixupfv[4] = {
			1.0f / pViewport->Width,
			-1.0f / pViewport->Height,
			0,
			0
		};

		if ((pViewport->Height == 1) || (pViewport->Width == 1))
		{
			fixupfv[0] = 0;
			fixupfv[1] = 0;
		}

		//if (!halfPixelFixWriteAdr)
		halfPixelFixWriteAdr = (float*)d912pxy_s(batch)->SetShaderConstFRewritable(0, PXY_INNER_MAX_SHADER_CONSTS_IDX * 2 - 1, 1, fixupfv);
		//else {
			//halfPixelFixWriteAdr[0] = fixupfv[0];
			//halfPixelFixWriteAdr[1] = fixupfv[1];
		//}
	}

	main_viewport = *pViewport;
	main_scissor.left = (UINT)pViewport->TopLeftX;
	main_scissor.top = (UINT)pViewport->TopLeftY;
	main_scissor.bottom = (UINT)pViewport->Height + (UINT)pViewport->TopLeftY;
	main_scissor.right = (UINT)pViewport->Width + (UINT)pViewport->TopLeftX;

	d912pxy_s(CMDReplay)->RSViewScissor(main_viewport, main_scissor);
}

void d912pxy_iframe::SetScissors(D3D12_RECT * pRect)
{
	main_scissor = *pRect;
	d912pxy_s(CMDReplay)->RSViewScissor(main_viewport, main_scissor);
}

void d912pxy_iframe::RestoreScissor()
{
	//megai2: it will work only if app do zero modification to scissor rect when it disabled
	d912pxy_s(CMDReplay)->RSViewScissor(main_viewport, main_scissor);
}

void d912pxy_iframe::IgnoreScissor()
{
	D3D12_RECT r;
	r.left = (UINT)main_viewport.TopLeftX;
	r.top = (UINT)main_viewport.TopLeftY;
	r.bottom = (UINT)main_viewport.Height + (UINT)main_viewport.TopLeftY;
	r.right = (UINT)main_viewport.Width + (UINT)main_viewport.TopLeftX;

	d912pxy_s(CMDReplay)->RSViewScissor(main_viewport, r);
}

void d912pxy_iframe::SetRSigOnList(d912pxy_gpu_cmd_list_group lstID)
{
	ID3D12GraphicsCommandList* cl = d912pxy_s(GPUcl)->GID(lstID).Get();
		
	cl->SetDescriptorHeaps(mSetHeapArrCnt, mSetHeapArr);

	cl->SetGraphicsRootSignature(mRootSignature.Get());

	cl->SetGraphicsRootDescriptorTable(0, mHeaps[PXY_INNER_HEAP_SRV]->GetGPUDHeapHandle(0));
	cl->SetGraphicsRootDescriptorTable(1, mHeaps[PXY_INNER_HEAP_SRV]->GetGPUDHeapHandle(0));
	cl->SetGraphicsRootDescriptorTable(2, mHeaps[PXY_INNER_HEAP_SPL]->GetGPUDHeapHandle(0));
	cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	d912pxy_s(batch)->SetRSigOnList(lstID);
}

void d912pxy_iframe::TransitStates(d912pxy_gpu_cmd_list_group tgtList)
{
	d912pxy_s(GPUcl)->GID(tgtList)->OMSetRenderTargets(bindedRTVcount, bindedRTV, 0, bindedDSV);
}

void d912pxy_iframe::NoteBindedSurfaceTransit(d912pxy_surface * surf, UINT slot)
{
	if (bindedSurfaces[slot] == surf)
		batchCommisionDF |= 4;
}

void d912pxy_iframe::InitRootSignature()
{
	D3D12_DESCRIPTOR_RANGE ranges[3];

	//zero of ROOTDESC is PXY_INNER_HEAP_TEX2D where all our textures are
	ranges[0].BaseShaderRegister = 0;
	ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	ranges[0].NumDescriptors = mHeaps[PXY_INNER_HEAP_SRV]->GetDesc()->NumDescriptors;
	ranges[0].OffsetInDescriptorsFromTableStart = 0;
	ranges[0].RegisterSpace = 0;

	ranges[1].BaseShaderRegister = 0;
	ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	ranges[1].NumDescriptors = mHeaps[PXY_INNER_HEAP_SRV]->GetDesc()->NumDescriptors;
	ranges[1].OffsetInDescriptorsFromTableStart = 0;
	ranges[1].RegisterSpace = 1;

	ranges[2].BaseShaderRegister = 0;
	ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
	ranges[2].NumDescriptors = mHeaps[PXY_INNER_HEAP_SPL]->GetDesc()->NumDescriptors;
	ranges[2].OffsetInDescriptorsFromTableStart = 0;
	ranges[2].RegisterSpace = 0;

	/*ranges[2].BaseShaderRegister = 0;
	ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	ranges[2].NumDescriptors = 1;//device_heap_config[PXY_INNER_HEAP_CBV].NumDescriptors;
	ranges[2].OffsetInDescriptorsFromTableStart = 0;
	ranges[2].RegisterSpace = 0;

	ranges[3].BaseShaderRegister = 0;
	ranges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	ranges[3].NumDescriptors = 1;//device_heap_config[PXY_INNER_HEAP_CBV].NumDescriptors;
	ranges[3].OffsetInDescriptorsFromTableStart = 0;
	ranges[3].RegisterSpace = 1;*/

	D3D12_ROOT_PARAMETER rootParameters[5];

	for (int i = 0; i != 3; ++i)
	{
		rootParameters[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameters[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParameters[i].DescriptorTable.NumDescriptorRanges = 1;
		rootParameters[i].DescriptorTable.pDescriptorRanges = &ranges[i];
	}

/*	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[2].Descriptor.RegisterSpace = 0;
	rootParameters[2].Descriptor.ShaderRegister = 0;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;*/

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].Descriptor.RegisterSpace = 0;
	rootParameters[3].Descriptor.ShaderRegister = 0;	
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

/*	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[4].Descriptor.RegisterSpace = 2;
	rootParameters[4].Descriptor.ShaderRegister = 0;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;*/

	D3D12_STATIC_SAMPLER_DESC staticPCF;
	staticPCF.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticPCF.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticPCF.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticPCF.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	staticPCF.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	staticPCF.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	staticPCF.RegisterSpace = 1;
	staticPCF.ShaderRegister = 0;
	staticPCF.MaxAnisotropy = 1;
	staticPCF.MaxLOD = 0;
	staticPCF.MinLOD = 0;
	staticPCF.MipLODBias = 0;
	staticPCF.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.NumParameters = 4;
	rootSignatureDesc.NumStaticSamplers = 1;
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.pStaticSamplers = &staticPCF;

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	LOG_ERR_THROW(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	LOG_ERR_THROW(d912pxy_s(DXDev)->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));

	d912pxy_s(psoCache)->SetRootSignature(mRootSignature);
}

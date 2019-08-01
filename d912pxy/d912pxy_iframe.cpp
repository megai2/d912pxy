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

d912pxy_iframe::d912pxy_iframe() 
{
}

d912pxy_iframe::~d912pxy_iframe()
{
	d912pxy_s.render.batch.~d912pxy_batch();
	d912pxy_s.render.tex.~d912pxy_texture_state();
	d912pxy_s.render.db.pso.~d912pxy_pso_cache();
}

void d912pxy_iframe::Init(d912pxy_dheap ** heaps)
{
	NonCom_Init(L"iframe");

	mHeaps = heaps;
	
	d912pxy_s.render.tex.Init();
	d912pxy_s.render.batch.Init();
	d912pxy_s.render.db.pso.Init();

	//mBatches->WriteElement(new d912pxy_batch(d912pxy_s(dev), mGPUque));

	mRBarrierStkPointer = 0;
	batchesIssued = 0;

	streamsActive = 0;
	for (int i = 0; i != PXY_INNER_MAX_VBUF_STREAMS; ++i)
		streamBinds[i].buffer = 0;

	InitRootSignature();

	mSetHeapArrCnt = 0;

	batchCommisionDF = 7;

	instanceCount = 0;

	for (int i = 0; i != PXY_INNER_MAX_DSC_HEAPS; ++i)
	{
		if (mHeaps[i]->GetDesc()->Type > 1)
			continue;

		mSetHeapArr[mSetHeapArrCnt] = mHeaps[i]->GetHeapObj();
		++mSetHeapArrCnt;
	}

	mCurrentFrameIndex = 0;

	cuPrimType = (D3DPRIMITIVETYPE)-1;

	d912pxy_s.render.db.pso.LoadCachedData();

	zeroWriteRT = NULL;
}

void d912pxy_iframe::RBarrierImm(D3D12_RESOURCE_BARRIER * bar)
{
	d912pxy_s.dx12.cl->GID(CLG_SEQ)->ResourceBarrier(0, bar);
}

void d912pxy_iframe::RBarrierStk(UINT cnt, D3D12_RESOURCE_BARRIER * bar)
{	
	d912pxy_s.dx12.cl->GID(CLG_TOP)->ResourceBarrier(cnt, bar);
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
}

void d912pxy_iframe::SetVBuf(d912pxy_vstream * vb, UINT StreamNumber, UINT OffsetInBytes, UINT Stride)
{
	UpdateActiveStreams(vb, StreamNumber);

	batchCommisionDF |= 1;	
	streamBinds[StreamNumber].buffer = vb;
	streamBinds[StreamNumber].offset = OffsetInBytes;
	streamBinds[StreamNumber].stride = Stride;

	if (vb)		
		d912pxy_s.render.replay.VBbind(vb, Stride, StreamNumber, OffsetInBytes);
}

void d912pxy_iframe::SetIBuf(d912pxy_vstream* ib)
{
	indexBind = ib;	

	if (ib)
		d912pxy_s.render.replay.IBbind(ib);
}

void d912pxy_iframe::SetIBufIfChanged(d912pxy_vstream * ib)
{
	if (indexBind != ib)	
		SetIBuf(ib);	
}

void d912pxy_iframe::SetVBufIfChanged(d912pxy_vstream * vb, UINT StreamNumber, UINT OffsetInBytes, UINT Stride)
{
	if (
		(streamBinds[StreamNumber].buffer != vb) ||
		(streamBinds[StreamNumber].offset != OffsetInBytes) ||
		(streamBinds[StreamNumber].stride != Stride)
		)
		SetVBuf(vb, StreamNumber, OffsetInBytes, Stride);

}

void d912pxy_iframe::UpdateActiveStreams(d912pxy_vstream * vb, UINT StreamNumber)
{
	if (vb && !streamBinds[StreamNumber].buffer)
		++streamsActive;
	else if (!vb && streamsActive)
	{
		if (streamBinds[StreamNumber].buffer)
			--streamsActive;
	}
}

d912pxy_vstream* d912pxy_iframe::GetIBuf()
{
	return indexBind;
}

d912pxy_device_streamsrc d912pxy_iframe::GetStreamSource(UINT StreamNumber)
{
	return streamBinds[StreamNumber];
}


UINT d912pxy_iframe::CommitBatchPreCheck(D3DPRIMITIVETYPE PrimitiveType)
{
	if (PrimitiveType == D3DPT_TRIANGLEFAN)
	{
		LOG_DBG_DTDM3("DP TRIFAN skipping");
		return 0;
	}

#ifdef PER_DRAW_FLUSH
	if (batchesIssued >= 1)
		StateSafeFlush(0);
#else
	if (batchesIssued >= (PXY_INNER_MAX_IFRAME_BATCH_COUNT - 2))
	{
		LOG_ERR_DTDM("batches in one frame exceeded PXY_INNER_MAX_IFRAME_BATCH_COUNT, performing queued commands now");

		StateSafeFlush(0);
	}
#endif

	return 1;
}

//megai2: this CommitBatch is made in assumption that app API stream is clean from dx9 intrisic resets / runtime checks / well hid kittens
void d912pxy_iframe::CommitBatch(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	if (!CommitBatchPreCheck(PrimitiveType))
		return;
	
	UINT32 batchDF = batchCommisionDF;
	batchCommisionDF = 0;
	
	d912pxy_s.render.tex.Use();

	//bind vb/ib
	
	if (batchDF & 1)
	{
		for (int i = 0; i != streamsActive; ++i)
		{
			if (streamBinds[i].divider & D3DSTREAMSOURCE_INDEXEDDATA)
			{
				instanceCount = 0x3FFFFFFF & streamBinds[i].divider;
			}

			if (streamBinds[i].divider & D3DSTREAMSOURCE_INSTANCEDATA)
			{
				d912pxy_vdecl* useInstanced = d912pxy_s.render.db.pso.GetIAFormat()->GetInstancedModification();
				//i belive that unmasked value must be equal to binded stream stride, so we just check that our vdecl is ready for this call
				useInstanced->ModifyStreamElementType(i, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA);
				d912pxy_s.render.db.pso.IAFormatInstanced(useInstanced);
				batchDF |= 8;
			}						
		}
	}

	if (batchDF & 4)
	{
		ProcessSurfaceBinds(0);
	}

	d912pxy_s.render.db.pso.Use();

	d912pxy_s.render.replay.DIIP(GetIndexCount(primCount,PrimitiveType), instanceCount, startIndex, BaseVertexIndex, MinVertexIndex, d912pxy_s.render.batch.NextBatch());

	instanceCount = 1;

	++batchesIssued;

	d912pxy_s.render.replay.IssueWork(batchesIssued);

	if (batchDF & 8)
	{
		d912pxy_s.render.db.pso.IAFormat(d912pxy_s.render.db.pso.GetIAFormat());
	}
}


//megai2: this CommitBatch is made in assumption that app API stream is garbadge
void d912pxy_iframe::CommitBatch2(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	if (!CommitBatchPreCheck(PrimitiveType))
		return;

	UINT32 batchDF = batchCommisionDF;
	batchCommisionDF = 0;

	d912pxy_s.render.tex.Use();

	DWORD usedStreams = 0xFF;

	if (!d912pxy_s.render.db.pso.GetCurrentCPSO())
	{
		d912pxy_vdecl* vdecl = d912pxy_s.render.db.pso.GetIAFormat();
		usedStreams = vdecl->GetUsedStreams();
	}

	if (batchDF & 1)
	{
		for (int i = 0; i != streamsActive; ++i)
		{
			if (!(usedStreams & (1 << i)))
				continue;

			if (!streamBinds[i].buffer)
				continue;

			if (streamBinds[i].divider & D3DSTREAMSOURCE_INDEXEDDATA)
			{
				instanceCount = 0x3FFFFFFF & streamBinds[i].divider;
			}

			if (streamBinds[i].divider & D3DSTREAMSOURCE_INSTANCEDATA)
			{
				d912pxy_vdecl* useInstanced = d912pxy_s.render.db.pso.GetIAFormat()->GetInstancedModification();
				//i belive that unmasked value must be equal to binded stream stride, so we just check that our vdecl is ready for this call
				useInstanced->ModifyStreamElementType(i, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA);
				d912pxy_s.render.db.pso.IAFormatInstanced(useInstanced);
				batchDF |= 8;
			}
		}
	}

	if (instanceCount > 1)
	{
		if ((batchDF & 8) == 0)
		{
			instanceCount = 1;
			batchDF &= ~8;
		}
	}

	if (batchDF & 4)
	{
		ProcessSurfaceBinds(0);
	}

	d912pxy_s.render.db.pso.Use();

	if (PrimitiveType != cuPrimType)
	{
		cuPrimType = PrimitiveType;
		d912pxy_s.render.replay.PrimTopo(cuPrimType);
	}

	d912pxy_s.render.replay.DIIP(GetIndexCount(primCount, PrimitiveType), instanceCount, startIndex, BaseVertexIndex, MinVertexIndex, d912pxy_s.render.batch.NextBatch());

	instanceCount = 1;

	++batchesIssued;

	d912pxy_s.render.replay.IssueWork(batchesIssued);

	if (batchDF & 8)
	{
		d912pxy_s.render.db.pso.IAFormat(d912pxy_s.render.db.pso.GetIAFormat());
	}
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
	}

	if (obj)
		bindedSurfacesDH[index] = obj->GetDHeapHandle();
	
	bindedSurfaces[index] = obj;
	batchCommisionDF |= 4;
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
	
	d912pxy_s.render.db.pso.IAFormat(src);	
}

void d912pxy_iframe::Start()
{
	LOG_DBG_DTDM2("Start Frame %u", mCurrentFrameIndex);

	for (int i = 0; i != PXY_INNER_MAX_DSC_HEAPS; ++i)
		mHeaps[i]->CleanupSlots(PXY_INNER_MAX_DHEAP_CLEANUP_PER_SYNC);

	d912pxy_s.render.db.pso.MarkDirty(0);

	LOG_DBG_DTDM("CMDreplay iframe start called");

	d912pxy_s.render.replay.IFrameStart();

	if (mSwapChain)
		mSwapChain->StartFrame();

	LOG_DBG_DTDM("batch frame start called");

	d912pxy_s.render.batch.FrameStart();

	LOG_DBG_DTDM("SetViewport called");

	SetViewport(&main_viewport);
	//SetScissors(&main_scissor);

	//megai2: restore RS & blendfactor on replay thread command list
	d912pxy_s.render.db.pso.State(D3DRS_STENCILREF, d912pxy_s.render.db.pso.GetDX9RsValue(D3DRS_STENCILREF));
	d912pxy_s.render.db.pso.State(D3DRS_BLENDFACTOR, d912pxy_s.render.db.pso.GetDX9RsValue(D3DRS_BLENDFACTOR));

	SetRSigOnList(CLG_TOP);
	SetRSigOnList(CLG_SEQ);

	SetIBuf(NULL);

	UINT cleanupStreams = streamsActive;

	for (int i = 0; i != cleanupStreams; ++i)
		SetVBuf(NULL, i, 0, 0);

	SetStreamFreq(0, 1);

	if (cleanupStreams > 1)
		for (int i = 1; i != cleanupStreams; ++i)
			SetStreamFreq(i, 0);

	streamsActive = 0;

	batchesIssued = 0;

	cuPrimType = D3DPT_TRIANGLELIST;

	d912pxy_query_occlusion::OnIFrameStart();

}

void d912pxy_iframe::End()
{
	//d912pxy_s.render.replay.Finish();

	/*if (mRBarrierStkPointer)
		d912pxy_s.dx12.cl->GID(CLG_TOP)->ResourceBarrier(mRBarrierStkPointer, mRBarrierStkData);
	mRBarrierStkPointer = 0;*/

	/*for (int i = 0; i != streamsActive; ++i)
		streamBinds[i].buffer = NULL;

	indexBind = NULL;*/

	d912pxy_s.render.draw_up.OnFrameEnd();
	d912pxy_query_occlusion::OnIFrameEnd();

	if (mSwapChain) 
		mSwapChain->EndFrame();

	LOG_DBG_DTDM2("End Frame %u", mCurrentFrameIndex);
	++mCurrentFrameIndex;
}

void d912pxy_iframe::EndSceneReset()
{
	//d912pxy_s.render.batch.ClearShaderVars();
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
		
		d912pxy_s.render.batch.SetShaderConstF(1, PXY_INNER_EXTRA_SHADER_CONST_HALFPIXEL_FIX, 1, fixupfv);
	}

	main_viewport = *pViewport;
	main_scissor.left = (UINT)pViewport->TopLeftX;
	main_scissor.top = (UINT)pViewport->TopLeftY;
	main_scissor.bottom = (UINT)pViewport->Height + (UINT)pViewport->TopLeftY;
	main_scissor.right = (UINT)pViewport->Width + (UINT)pViewport->TopLeftX;

	d912pxy_s.render.replay.RSViewScissor(main_viewport, main_scissor);
}

void d912pxy_iframe::SetScissors(D3D12_RECT * pRect)
{
	main_scissor = *pRect;
	d912pxy_s.render.replay.RSViewScissor(main_viewport, main_scissor);
}

void d912pxy_iframe::SetViewportIfChanged(D3D12_VIEWPORT * pViewport)
{
	if (memcmp(pViewport, &main_viewport, sizeof(D3D12_VIEWPORT)) == 0)
		return;

	SetViewport(pViewport);
}

void d912pxy_iframe::SetScissorsIfChanged(D3D12_RECT * pRect)
{
	if (memcmp(pRect, &main_scissor, sizeof(D3D12_RECT)) == 0)
		return;

	SetScissors(pRect);
}

void d912pxy_iframe::RestoreScissor()
{
	//megai2: it will work only if app do zero modification to scissor rect when it disabled
	d912pxy_s.render.replay.RSViewScissor(main_viewport, main_scissor);
}

void d912pxy_iframe::IgnoreScissor()
{
	D3D12_RECT r;
	r.left = (UINT)main_viewport.TopLeftX;
	r.top = (UINT)main_viewport.TopLeftY;
	r.bottom = (UINT)main_viewport.Height + (UINT)main_viewport.TopLeftY;
	r.right = (UINT)main_viewport.Width + (UINT)main_viewport.TopLeftX;

	d912pxy_s.render.replay.RSViewScissor(main_viewport, r);
}

void d912pxy_iframe::SetSwapper(d912pxy_swapchain * iSwp)
{
	mSwapChain = iSwp;
	d912pxy_s.dx12.que.SetPresenter(iSwp);
}

void d912pxy_iframe::SetRSigOnList(d912pxy_gpu_cmd_list_group lstID)
{
	ID3D12GraphicsCommandList* cl = d912pxy_s.dx12.cl->GID(lstID);
		
	cl->SetDescriptorHeaps(mSetHeapArrCnt, mSetHeapArr);

	cl->SetGraphicsRootSignature(mRootSignature.Get());

	cl->SetGraphicsRootDescriptorTable(0, mHeaps[PXY_INNER_HEAP_SRV]->GetGPUDHeapHandle(0));
	cl->SetGraphicsRootDescriptorTable(1, mHeaps[PXY_INNER_HEAP_SRV]->GetGPUDHeapHandle(0));
	cl->SetGraphicsRootDescriptorTable(2, mHeaps[PXY_INNER_HEAP_SPL]->GetGPUDHeapHandle(0));
	cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//megai2: keep this commented out 
	/*static const float white[4] = { 1,1,1,1 };
	
	cl->OMSetBlendFactor(white);*/

}

void d912pxy_iframe::NoteBindedSurfaceTransit(d912pxy_surface * surf, UINT slot)
{
	if (bindedSurfaces[slot] == surf)
		batchCommisionDF |= 4;
}

void d912pxy_iframe::StateSafeFlush(UINT fullFlush)
{
	d912pxy_vstream* indTransfer = indexBind;

	D3D12_VIEWPORT transVW = main_viewport;
	D3D12_RECT transSR = main_scissor;

	DWORD transSRef = d912pxy_s.render.db.pso.GetDX9RsValue(D3DRS_STENCILREF);

	if (indexBind)
		indexBind->ThreadRef(1);

	d912pxy_device_streamsrc vstreamTransfer[PXY_INNER_MAX_VBUF_STREAMS];
	UINT savedActiveStreams = streamsActive;

	for (int i = 0; i != savedActiveStreams; ++i)
	{
		if (streamBinds[i].buffer)
		{
			streamBinds[i].buffer->ThreadRef(1);
			vstreamTransfer[i] = streamBinds[i];
		}
	}

	d912pxy_surface* refSurf[2];

	for (int i = 0; i != 2; ++i)
	{
		refSurf[i] = bindedSurfaces[i];
		if (refSurf[i])
			refSurf[i]->ThreadRef(1);
	}

	End();
	if (fullFlush)
		d912pxy_s.dx12.que.Flush(0);
	else 
		d912pxy_s.dx12.que.ExecuteCommands(0);
	Start();

	//megai2: rebind surfaces as they are resetted to swapchain back buffers by Start()
	for (int i = 0; i != 2; ++i)
	{
		BindSurface(i, refSurf[i]);

		if (refSurf[i])
			refSurf[i]->ThreadRef(-1);
	}

	//megai2: rebind buffers too as commitdraw is optimized out for buffer bindings
	if (indTransfer)
	{
		SetIBuf(indTransfer);
		indTransfer->ThreadRef(-1);
	}

	for (int i = 0; i != savedActiveStreams; ++i)
	{
		if (vstreamTransfer[i].buffer)
		{
			SetVBuf(vstreamTransfer[i].buffer, i, vstreamTransfer[i].offset, vstreamTransfer[i].stride);			
			vstreamTransfer[i].buffer->ThreadRef(-1);
		}

		SetStreamFreq(i, vstreamTransfer[i].divider);
	}

	//megai2: rebind viewport & scissor too

	SetViewport(&transVW);
	SetScissors(&transSR);

	d912pxy_s.dev.SetRenderState(D3DRS_STENCILREF, transSRef);
	
	ForceStateRebind();	
}

void d912pxy_iframe::ForceStateRebind()
{
	//megai2: force dirty to rebind all states
	batchCommisionDF |= 7;
}

UINT d912pxy_iframe::ForceActiveStreams(UINT forceValue)
{
	UINT ret = streamsActive;

	streamsActive = forceValue;

	return ret;
}

UINT d912pxy_iframe::GetIndexCount(UINT PrimitiveCount, D3DPRIMITIVETYPE PrimitiveType)
{
	DWORD pperprim[] = {
		0,
		1,//point
		2,//linelist
		1,//linestrip
		3,//trilist
		1,//tristrip		
		0//trifan
	};

	DWORD primsubs[] = {
		0,
		0,//point
		0,//linelist
		1,//linestrip
		0,//trilist
		2,//tristrip		
		1//trifan
	};

	return PrimitiveCount * pperprim[PrimitiveType] + primsubs[PrimitiveType];
}

void d912pxy_iframe::OptimizeZeroWriteRT(UINT writeFlag)
{
	if (writeFlag == 0)
	{
		if (bindedSurfaces[1])
			zeroWriteRT = bindedSurfaces[1];
		BindSurface(1, NULL);
	}
	else {
		if (bindedSurfaces[1] == NULL)
		{
			BindSurface(1, zeroWriteRT);
		}

		zeroWriteRT = NULL;
	}


}

void d912pxy_iframe::ProcessSurfaceBinds(UINT psoOnly)
{
	bindedRTV = &bindedSurfacesDH[1];
	bindedDSV = &bindedSurfacesDH[0];

	if (bindedSurfaces[1])
	{
		d912pxy_s.render.db.pso.RTVFormat(bindedSurfaces[1]->GetSRVFormat(), 0);
		d912pxy_s.render.replay.StateTransit(bindedSurfaces[1], D3D12_RESOURCE_STATE_RENDER_TARGET);
		bindedRTVcount = 1;
	}
	else {
		d912pxy_s.render.db.pso.RTVFormat(DXGI_FORMAT_UNKNOWN, 0);
		bindedRTVcount = 0;
		bindedRTV = 0;
	}

	if (bindedSurfaces[0])
	{
		d912pxy_s.render.db.pso.DSVFormat(bindedSurfaces[0]->GetDSVFormat());
		d912pxy_s.render.replay.StateTransit(bindedSurfaces[0], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}
	else {
		bindedDSV = 0;
	}

	d912pxy_s.render.db.pso.OMReflect(bindedRTVcount, bindedDSV);

	if (!psoOnly)
	{
		if (bindedRTV && bindedDSV)
			d912pxy_s.render.replay.RT(bindedSurfaces[1], bindedSurfaces[0]);
		else if (bindedRTV)
			d912pxy_s.render.replay.RT(bindedSurfaces[1], 0);
		else if (bindedDSV)
			d912pxy_s.render.replay.RT(0, bindedSurfaces[0]);
	}
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
	LOG_ERR_THROW(d912pxy_s.dx12.dev->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));

	d912pxy_s.render.db.pso.SetRootSignature(mRootSignature);
}

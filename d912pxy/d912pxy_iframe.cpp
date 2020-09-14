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

d912pxy_iframe::d912pxy_iframe() 
{
}

d912pxy_iframe::~d912pxy_iframe()
{

}

void d912pxy_iframe::Init(d912pxy_dheap ** heaps)
{
	NonCom_Init(L"iframe");

	batchLimit = d912pxy_s.config.GetValueUI32(PXY_CFG_BATCHING_MAX_BATCHES_PER_IFRAME);

	mHeaps = heaps;
	
	d912pxy_s.render.state.tex.Init();
	d912pxy_s.render.batch.Init();
	d912pxy_s.render.state.pso.Init();

	InitRootSignature();
	d912pxy_trimmed_pso_desc::SetupBaseFullPSO(mRootSignature);

	d912pxy_s.render.db.pso.Init();

	memset((void*)&streamBinds, 0, sizeof(StreamBinds));
	streamsActive = 0;	

	mSetHeapArrCnt = 0;

	batchCommisionDF = 7;

	batchCommitData.instanceCount = 0;

	for (int i = 0; i != PXY_INNER_MAX_DSC_HEAPS; ++i)
	{
		if (mHeaps[i]->GetDesc()->Type > 1)
			continue;

		mSetHeapArr[mSetHeapArrCnt] = mHeaps[i]->GetHeapObj();
		++mSetHeapArrCnt;
	}

	mCurrentFrameIndex = 0;

	cuPrimType = (D3DPRIMITIVETYPE)-1;
	zeroWriteRT = NULL;
}

void d912pxy_iframe::UnInit()
{
	mRootSignature->Release();

	d912pxy_s.render.batch.UnInit();
	d912pxy_s.render.state.tex.UnInit();
	d912pxy_s.render.state.pso.UnInit();

	d912pxy_noncom::UnInit();
}

void d912pxy_iframe::SetStreamFreq(UINT StreamNumber, UINT Divider)
{
	streamBinds.vertex[StreamNumber].divider = Divider;
}

void d912pxy_iframe::SetVBuf(d912pxy_vstream * vb, UINT StreamNumber, UINT OffsetInBytes, UINT Stride)
{
	UpdateActiveStreams(vb, StreamNumber);

	batchCommisionDF |= 1;	

	streamBinds.vertex[StreamNumber].buffer = vb;
	streamBinds.vertex[StreamNumber].offset = OffsetInBytes;
	streamBinds.vertex[StreamNumber].stride = Stride;

	if (vb)		
		d912pxy_s.render.replay.DoVBbind(vb, Stride, StreamNumber, OffsetInBytes);
}

void d912pxy_iframe::SetIBuf(d912pxy_vstream* ib)
{
	streamBinds.index = ib;	

	if (ib)
		d912pxy_s.render.replay.DoIBbind(ib);
}

void d912pxy_iframe::SetIBufIfChanged(d912pxy_vstream * ib)
{
	if (streamBinds.index != ib)
		SetIBuf(ib);	
}

void d912pxy_iframe::SetVBufIfChanged(d912pxy_vstream * vb, UINT StreamNumber, UINT OffsetInBytes, UINT Stride)
{
	if (
		(streamBinds.vertex[StreamNumber].buffer != vb) ||
		(streamBinds.vertex[StreamNumber].offset != OffsetInBytes) ||
		(streamBinds.vertex[StreamNumber].stride != Stride)
		)
		SetVBuf(vb, StreamNumber, OffsetInBytes, Stride);

}

void d912pxy_iframe::UpdateActiveStreams(d912pxy_vstream * vb, UINT StreamNumber)
{
	if (vb && !streamBinds.vertex[StreamNumber].buffer)
		++streamsActive;
	else if (!vb && streamsActive)
	{
		if (streamBinds.vertex[StreamNumber].buffer)
			--streamsActive;
	}
}

d912pxy_vstream* d912pxy_iframe::GetIBuf()
{
	return streamBinds.index;
}

d912pxy_device_streamsrc d912pxy_iframe::GetStreamSource(UINT StreamNumber)
{
	return streamBinds.vertex[StreamNumber];
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

void d912pxy_iframe::Start()
{
	LOG_DBG_DTDM2("Start Frame %u", mCurrentFrameIndex);

	for (int i = 0; i != PXY_INNER_MAX_DSC_HEAPS; ++i)
		mHeaps[i]->CleanupSlots(PXY_INNER_MAX_DHEAP_CLEANUP_PER_SYNC);

	d912pxy_s.render.state.pso.MarkDirty();

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
	d912pxy_s.render.state.pso.SetDX9RS(D3DRS_STENCILREF, d912pxy_s.render.state.pso.GetDX9RsValue(D3DRS_STENCILREF));
	d912pxy_s.render.state.pso.SetDX9RS(D3DRS_BLENDFACTOR, d912pxy_s.render.state.pso.GetDX9RsValue(D3DRS_BLENDFACTOR));

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

	cuPrimType = D3DPT_TRIANGLELIST;

	//reset textures to 0
	//this is not noted in docs, but dx9 can do this by default
	//i keep it untouched for now as it showed working fine before
	//WARN: GPU crash here if api stream will have 3 sequental StateSafeFlushes(0) or simply frame changes
	//and non-cleared from stage, deleted, but used in shader texture
	//d912pxy_s.render.state.tex.ClearActiveTextures();

	d912pxy_query_occlusion::OnIFrameStart();

	if (d912pxy_s.render.replay.extras.enable)
		d912pxy_s.iframeMods.IFR_Start();
}

void d912pxy_iframe::End()
{
	d912pxy_s.render.draw_up.OnFrameEnd();
	d912pxy_query_occlusion::OnIFrameEnd();

	if (mSwapChain) 
		mSwapChain->EndFrame();

	if (d912pxy_s.render.replay.extras.enable)
		d912pxy_s.iframeMods.IFR_End();

	LOG_DBG_DTDM2("End Frame %u", mCurrentFrameIndex);
	++mCurrentFrameIndex;
}

void d912pxy_iframe::EndSceneReset()
{

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

	d912pxy_s.render.replay.DoRSViewScissor(main_viewport, main_scissor);
}

void d912pxy_iframe::SetScissors(D3D12_RECT * pRect)
{
	main_scissor = *pRect;
	d912pxy_s.render.replay.DoRSViewScissor(main_viewport, main_scissor);
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
	d912pxy_s.render.replay.DoRSViewScissor(main_viewport, main_scissor);
}

void d912pxy_iframe::IgnoreScissor()
{
	D3D12_RECT r;
	r.left = (UINT)main_viewport.TopLeftX;
	r.top = (UINT)main_viewport.TopLeftY;
	r.bottom = (UINT)main_viewport.Height + (UINT)main_viewport.TopLeftY;
	r.right = (UINT)main_viewport.Width + (UINT)main_viewport.TopLeftX;

	d912pxy_s.render.replay.DoRSViewScissor(main_viewport, r);
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

	cl->SetGraphicsRootSignature(mRootSignature);

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
	LOG_DBG_DTDM("State safe flush(%u)", fullFlush);

	D3D12_VIEWPORT transVW = main_viewport;
	D3D12_RECT transSR = main_scissor;

	DWORD transSRef = d912pxy_s.render.state.pso.GetDX9RsValue(D3DRS_STENCILREF);

	d912pxy_surface* refSurf[1 + PXY_INNER_MAX_RENDER_TARGETS];

	for (int i = 0; i != 1 + PXY_INNER_MAX_RENDER_TARGETS; ++i)
	{
		refSurf[i] = bindedSurfaces[i];
		if (refSurf[i])
			refSurf[i]->ThreadRef(1);
	}

	StreamBindsHolder savedStreamBinds;
		
	End();
	if (fullFlush)
		d912pxy_s.dx12.que.Flush(0);
	else 
		d912pxy_s.dx12.que.ExecuteCommands(0);
	Start();

	//megai2: rebind surfaces as they are resetted to swapchain back buffers by Start()
	for (int i = 0; i != 1 + PXY_INNER_MAX_RENDER_TARGETS; ++i)
	{
		BindSurface(i, refSurf[i]);

		if (refSurf[i])
			refSurf[i]->ThreadRef(-1);
	}

	//megai2: rebind scissor 
	SetScissors(&transSR);
	SetViewport(&transVW);	
	
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
			BindSurface(1, zeroWriteRT);		
		zeroWriteRT = NULL;
	}
}

void d912pxy_iframe::ProcessSurfaceBinds(UINT psoOnly)
{
	bindedRTVcount = 0;

	for (int i = 1; i < 1 + PXY_INNER_MAX_RENDER_TARGETS; ++i)
	{
		if (bindedSurfaces[i])
		{
			d912pxy_s.render.state.pso.RTVFormat(bindedSurfaces[i]->GetSRVFormat(), i-1);
			d912pxy_s.render.replay.DoBarrier(bindedSurfaces[i], D3D12_RESOURCE_STATE_RENDER_TARGET);
			bindedRTVcount = i;//count total by last active
		}
		else
			d912pxy_s.render.state.pso.RTVFormat(DXGI_FORMAT_UNKNOWN, i-1);
	}

	if (bindedSurfaces[0])
	{
		d912pxy_s.render.state.pso.DSVFormat(bindedSurfaces[0]->GetDSVFormat());
		d912pxy_s.render.replay.DoBarrier(bindedSurfaces[0], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}

	d912pxy_s.render.state.pso.OMReflect(bindedRTVcount, bindedSurfaces[0] ? &bindedSurfacesDH[0] : nullptr);

	if (!psoOnly)	
		d912pxy_s.render.replay.DoRT(&bindedSurfaces[1], bindedSurfaces[0]);
}

void d912pxy_iframe::OverrideRootSignature(ID3D12RootSignature* newRS)
{
	d912pxy::error::check(!mRootSignature, L"Can't double override root signature/root signature overriden too late");
	mRootSignature = newRS;
}

void d912pxy_iframe::FillPrimaryRSParameters(D3D12_ROOT_PARAMETER* rootParameters, D3D12_DESCRIPTOR_RANGE* ranges)
{
	for (int i = 0; i != 3; ++i)
	{
		rootParameters[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameters[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParameters[i].DescriptorTable.NumDescriptorRanges = 1;
		rootParameters[i].DescriptorTable.pDescriptorRanges = &ranges[i];
	}

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].Descriptor.RegisterSpace = 0;
	rootParameters[3].Descriptor.ShaderRegister = 0;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
}

void d912pxy_iframe::FillPrimaryRSstaticPCFSampler(D3D12_STATIC_SAMPLER_DESC& staticPCF)
{
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
}

void d912pxy_iframe::FillPrimaryRSDescriptorRanges(D3D12_DESCRIPTOR_RANGE* ranges)
{
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
}

void d912pxy_iframe::InitRootSignature()
{
	if (mRootSignature)
	{
		LOG_INFO_DTDM("using custom/overriden root signature");
		return;
	}

	D3D12_DESCRIPTOR_RANGE ranges[3];
	FillPrimaryRSDescriptorRanges(ranges);

	D3D12_ROOT_PARAMETER rootParameters[5];
	FillPrimaryRSParameters(rootParameters, ranges);

	D3D12_STATIC_SAMPLER_DESC staticPCF;
	FillPrimaryRSstaticPCFSampler(staticPCF);

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.NumParameters = 4;
	rootSignatureDesc.NumStaticSamplers = 1;
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.pStaticSamplers = &staticPCF;

	mRootSignature = d912pxy_s.dev.ConstructRootSignature(&rootSignatureDesc);
}

d912pxy_iframe::StreamBindsHolder::StreamBindsHolder() 
	: iframe(d912pxy_s.render.iframe)
{
	data = iframe.streamBinds;

	if (data.index)
		data.index->ThreadRef(1);

	for (int i = 0; i != PXY_INNER_MAX_VBUF_STREAMS; ++i)
	{
		auto& vbsEl = data.vertex[i];
		if (vbsEl.buffer)
		{
			vbsEl.buffer->ThreadRef(1);
		}		
	}
}

d912pxy_iframe::StreamBindsHolder::~StreamBindsHolder()
{	
	if (data.index)
	{
		iframe.SetIBuf(data.index);
		data.index->ThreadRef(-1);
	}

	for (int i = 0; i != PXY_INNER_MAX_VBUF_STREAMS; ++i)
	{
		auto& vbsEl = data.vertex[i];
		if (vbsEl.buffer)
		{
			iframe.SetVBuf(vbsEl.buffer, i, vbsEl.offset, vbsEl.stride);
			vbsEl.buffer->ThreadRef(-1);
		}

		iframe.SetStreamFreq(i, vbsEl.divider);
	}

}

UINT d912pxy_iframe::CommitBatchPreCheck(D3DPRIMITIVETYPE PrimitiveType)
{
	if (PrimitiveType == D3DPT_TRIANGLEFAN)
	{
		LOG_DBG_DTDM3("DP TRIFAN skipping");
		return 0;
	}

#ifdef PER_DRAW_FLUSH
	if (d912pxy_s.render.batch.GetBatchCount() >= 1)
		StateSafeFlush(0);
#else
	if (d912pxy_s.render.batch.GetBatchCount() >= (batchLimit - 2))
	{
		LOG_DBG_DTDM3("batches in one frame exceeded PXY_INNER_MAX_IFRAME_BATCH_COUNT, performing queued commands now");

		StateSafeFlush(0);
	}
#endif

	return 1;
}

void d912pxy_iframe::CommitBatchTailProc(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	if (batchCommitData.batchDF & 8)
	{
		d912pxy_vdecl* useInstanced = d912pxy_s.render.state.pso.GetIAFormat()->GetInstancedModification(batchCommitData.instancedModMask, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA);
		d912pxy_s.render.state.pso.IAFormatInstanced(useInstanced);
	}

	if (batchCommitData.batchDF & 4)
	{
		d912pxy_s.render.iframe.ProcessSurfaceBinds(0);
	}

	d912pxy_s.render.state.tex.Use();
	d912pxy_s.render.state.pso.Use();

	d912pxy_s.render.replay.DoDIIP(
		d912pxy_s.render.iframe.GetIndexCount(primCount, PrimitiveType), 
		batchCommitData.instanceCount,
		startIndex, 
		BaseVertexIndex, 
		MinVertexIndex, 
		d912pxy_s.render.batch.FinishCurrentDraw()
	);

	batchCommitData.instanceCount = 1;

	d912pxy_s.render.replay.IssueWork(d912pxy_s.render.batch.GetBatchCount());

	if (batchCommitData.batchDF & 8)
	{
		d912pxy_s.render.state.pso.IAFormat(d912pxy_s.render.state.pso.GetIAFormat());
	}
}

bool d912pxy_iframe::CommitBatchHeadProc(D3DPRIMITIVETYPE PrimitiveType)
{
	if (!CommitBatchPreCheck(PrimitiveType))
		return false;

	batchCommitData.batchDF = batchCommisionDF;
	batchCommisionDF = 0;
	batchCommitData.instancedModMask = 0;

	return true;
}

void d912pxy_iframe::ExtractInstanceCount()
{	
	if (batchCommitData.batchDF & 1)
	{
		for (int i = 0; i != streamsActive; ++i)
			ExtractBatchDataFromStream(i);
	}	
}

void d912pxy_iframe::ExtractInstanceCountExtra()
{
	DWORD usedStreams = 0xFF;

	if (!d912pxy_s.render.state.pso.GetCurrentCPSO())
	{
		d912pxy_vdecl* vdecl = d912pxy_s.render.state.pso.GetIAFormat();		
		usedStreams = vdecl ? vdecl->GetUsedStreams() : 1;
	}

	if (batchCommitData.batchDF & 1)
	{
		for (int i = 0; i != streamsActive; ++i)
		{
			if (!(usedStreams & (1 << i)))
				continue;

			if (!streamBinds.vertex[i].buffer)
				continue;

			ExtractBatchDataFromStream(i);
		}
	}

	if (batchCommitData.instanceCount > 1)
	{
		if ((batchCommitData.batchDF & 8) == 0)
		{
			batchCommitData.instanceCount = 1;
			batchCommitData.batchDF &= ~8;
		}
	}
}

void d912pxy_iframe::ExtractBatchDataFromStream(int stream)
{
	auto& streamRef = streamBinds.vertex[stream];

	if (streamRef.divider & D3DSTREAMSOURCE_INDEXEDDATA)
	{
		batchCommitData.instanceCount = 0x3FFFFFFF & streamRef.divider;
	}

	if (streamRef.divider & D3DSTREAMSOURCE_INSTANCEDATA)
	{
		batchCommitData.instancedModMask |= 1 << stream;
		batchCommitData.batchDF |= 8;
	}
}

void d912pxy_iframe::CommitBatch(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	if (!CommitBatchHeadProc(PrimitiveType))
		return;

	ExtractInstanceCount();

	CommitBatchTailProc(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

void d912pxy_iframe::CommitBatch2(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	if (!CommitBatchHeadProc(PrimitiveType))
		return;

	ExtractInstanceCountExtra();

	if (PrimitiveType != cuPrimType)
	{
		cuPrimType = PrimitiveType;
		d912pxy_s.render.replay.DoPrimTopo(cuPrimType);
	}

	CommitBatchTailProc(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

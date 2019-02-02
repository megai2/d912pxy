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

UINT16 pso_desc_memjmp_map[] = {
	8,0,0,0,0,3,0,6,0,0,0,0,3,0,0,93,7,3,3,3,3,3,7,291,15,0,0,0,20,3,3,3,15,3,23,0,0,0,0,3,15,3,31,44
};

D3D12_GRAPHICS_PIPELINE_STATE_DESC d912pxy_pso_cache::cDscBase;
UINT d912pxy_pso_cache::vsMaxVars;
UINT d912pxy_pso_cache::psMaxVars;

d912pxy_pso_cache::d912pxy_pso_cache(d912pxy_device * dev) : d912pxy_noncom(dev, L"PSO cache"), d912pxy_thread()
{
	d912pxy_s(psoCache) = this;

	cacheIndexes = new d912pxy_memtree2(sizeof(d912pxy_trimmed_dx12_pso) - d912pxy_trimmed_dx12_pso_hash_offset, PXY_INNER_MAX_PSO_CACHE_NODES, 2, 0);
	cacheIncID = 0;

	d912pxy_pso_cache::vsMaxVars = 0;
	d912pxy_pso_cache::psMaxVars = 0;
	
	shaderCleanupBuffer = new d912pxy_ringbuffer<d912pxy_shader*>(0xFFFF, 0);

	ZeroMemory(&d912pxy_pso_cache::cDscBase, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	cDsc.NumRenderTargets = 1;
	cDsc.RTVFormat0 = DXGI_FORMAT_B8G8R8A8_UNORM;
	cDsc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d912pxy_pso_cache::cDscBase.SampleDesc.Count = 1;
	d912pxy_pso_cache::cDscBase.SampleDesc.Quality = 0;
	d912pxy_pso_cache::cDscBase.SampleMask = 0xFFFFFFFF;
	cDsc.RasterizerState.DepthBiasClamp = 0;
	cDsc.RasterizerState.DepthClipEnable = 1;
	d912pxy_pso_cache::cDscBase.GS.pShaderBytecode = NULL;
	d912pxy_pso_cache::cDscBase.DS.pShaderBytecode = NULL;
	d912pxy_pso_cache::cDscBase.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	
	State(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	State(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
	State(D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_KEEP);
	State(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP);
	State(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_KEEP);
	State(D3DRS_CCW_STENCILFUNC, D3DCMP_NEVER);
	State(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	State(D3DRS_SRCBLEND, D3DBLEND_ONE);
	State(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	State(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
	State(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	State(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);
	State(D3DRS_FILLMODE, D3DFILL_SOLID);

	State(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);
	State(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
	State(D3DRS_STENCILMASK, 0xFFFFFFFF);
	State(D3DRS_STENCILFAIL, 1);
	State(D3DRS_STENCILPASS, 1);
	State(D3DRS_STENCILREF, 0);
	State(D3DRS_STENCILZFAIL, 1);
	State(D3DRS_STENCILFUNC, 8);
	
	dirty = 1;

	psoCompileBuffer = new d912pxy_ringbuffer<d912pxy_pso_cache_item*>(0xFFFF, 0);	
}

d912pxy_pso_cache::~d912pxy_pso_cache()
{
	Stop();
	delete psoCompileBuffer;
	delete shaderCleanupBuffer;

	delete cacheIndexes;

	/*UINT32 itemsToClean;

	d912pxy_memtree_node* mtnp = cacheData->GetNodePool(&itemsToClean);

	for (int i = 0; i != itemsToClean; ++i)
	{
		UINT64 cid = mtnp[i].contentId;
		if (cid)
		{
			d912pxy_pso_cache_item* item = (d912pxy_pso_cache_item*)cid;

			item->Release();
		}
	}

	delete cacheData;*/
}

void d912pxy_pso_cache::State(D3DRENDERSTATETYPE State, DWORD Value)
{
	if (State > D3DRS_BLENDOPALPHA)
		return;

	dirty |= 1;// (DX9RSvalues[State] != Value);
//	DX9RSvalues[State] = Value;

	switch (State)
	{
	case D3DRS_SCISSORTESTENABLE:
		if (Value)
			d912pxy_s(iframe)->RestoreScissor();
		else
			d912pxy_s(iframe)->IgnoreScissor();
		DX9RSvalues[State] = Value;
		break;
	case D3DRS_ZENABLE:
		cDsc.DepthStencilState.DepthEnable = (Value == D3DZB_TRUE);
		break; //7,    /* D3DZBUFFERTYPE (or TRUE/FALSE for legacy) */

	case D3DRS_FILLMODE:
		if (D3DFILL_SOLID == Value)
			cDsc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		else if (D3DFILL_WIREFRAME == Value)
			cDsc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		else {
			cDsc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
			LOG_DBG_DTDM("RS fillmode point unimpl");
		}
		break; //8,    /* D3DFILLMODE */

	case D3DRS_ZWRITEENABLE:
		if (Value)
			cDsc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		else
			cDsc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		break; //14,   /* TRUE to enable z writes */

	case D3DRS_SRCBLEND:
		cDsc.BlendStateRT0.SrcBlend = (D3D12_BLEND)Value;
		switch (Value)
		{
		case D3DBLEND_SRCCOLOR:
			Value = D3DBLEND_SRCALPHA;
			break;
		case D3DBLEND_INVSRCCOLOR:
			Value = D3DBLEND_INVSRCALPHA;
			break;
		case D3DBLEND_DESTCOLOR:
			Value = D3DBLEND_DESTALPHA;
			break;
		case D3DBLEND_INVDESTCOLOR:
			Value = D3DBLEND_INVDESTALPHA;
			break;
		default:
			;
		}
		cDsc.BlendStateRT0.SrcBlendAlpha = (D3D12_BLEND)Value;

		break; //19,   /* D3DBLEND */

	case D3DRS_DESTBLEND:
		cDsc.BlendStateRT0.DestBlend = (D3D12_BLEND)Value;
		switch (Value)
		{
		case D3DBLEND_SRCCOLOR:
			Value = D3DBLEND_SRCALPHA;
			break;
		case D3DBLEND_INVSRCCOLOR:
			Value = D3DBLEND_INVSRCALPHA;
			break;
		case D3DBLEND_DESTCOLOR:
			Value = D3DBLEND_DESTALPHA;
			break;
		case D3DBLEND_INVDESTCOLOR:
			Value = D3DBLEND_INVDESTALPHA;
			break;
		default:
			;
		}
		cDsc.BlendStateRT0.DestBlendAlpha = (D3D12_BLEND)Value;

		break; //20,   /* D3DBLEND */

	case D3DRS_CULLMODE:
		cDsc.RasterizerState.CullMode = (D3D12_CULL_MODE)Value;
		break; //22,   /* D3DCULL */

	case D3DRS_ZFUNC:
		cDsc.DepthStencilState.DepthFunc = (D3D12_COMPARISON_FUNC)Value;
		break; //23,   /* D3DCMPFUNC */

	case D3DRS_ALPHABLENDENABLE:
		cDsc.BlendStateRT0.BlendEnable = Value;
		//FIXME! must set this to all active RT's somewhere
		break; //27,   /* TRUE to enable alpha blending */

	case D3DRS_STENCILENABLE:
		cDsc.DepthStencilState.StencilEnable = Value;
		break; //52,   /* BOOL enable/disable stenciling */

	case D3DRS_STENCILFAIL:
		cDsc.DepthStencilState.FrontFace.StencilFailOp = (D3D12_STENCIL_OP)Value;
		break; //53,   /* D3DSTENCILOP to do if stencil test fails */

	case D3DRS_STENCILZFAIL:
		cDsc.DepthStencilState.FrontFace.StencilDepthFailOp = (D3D12_STENCIL_OP)Value;
		break; //54,   /* D3DSTENCILOP to do if stencil test passes and Z test fails */

	case D3DRS_STENCILPASS:
		cDsc.DepthStencilState.FrontFace.StencilPassOp = (D3D12_STENCIL_OP)Value;
		break; //55,   /* D3DSTENCILOP to do if both stencil and Z tests pass */

	case D3DRS_STENCILFUNC:
		cDsc.DepthStencilState.FrontFace.StencilFunc = (D3D12_COMPARISON_FUNC)Value;
		break; //56,   /* D3DCMPFUNC fn.  Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */

	case D3DRS_STENCILMASK:
		cDsc.DepthStencilState.StencilReadMask = Value & 0xFF;
		break; //58,   /* Mask value used in stencil test */

	case D3DRS_STENCILWRITEMASK:
		cDsc.DepthStencilState.StencilWriteMask = Value & 0xFF;
		break; //59,   /* Write mask applied to values written to stencil buffer */

	case D3DRS_COLORWRITEENABLE:
		cDsc.BlendStateRT0.RenderTargetWriteMask = Value & 0xF;
		break; //168,  // per-channel write enable
	case D3DRS_COLORWRITEENABLE1: 
	case D3DRS_COLORWRITEENABLE2: 
	case D3DRS_COLORWRITEENABLE3: 
		//cDsc.BlendStateRT0.RenderTarget[State - D3DRS_COLORWRITEENABLE1 + 1].RenderTargetWriteMask = Value & 0xF;
		break;

	case D3DRS_BLENDOP:
		cDsc.BlendStateRT0.BlendOp = (D3D12_BLEND_OP)Value;
		cDsc.BlendStateRT0.BlendOpAlpha = (D3D12_BLEND_OP)Value;
		break; //171,   // D3DBLENDOP setting

	case D3DRS_SLOPESCALEDEPTHBIAS:
		cDsc.RasterizerState.SlopeScaledDepthBias = *(float*)(&Value);
		break; //175,

	case D3DRS_ANTIALIASEDLINEENABLE:
		cDsc.RasterizerState.AntialiasedLineEnable = Value;
		break; //176,

	case D3DRS_TWOSIDEDSTENCILMODE:
		LOG_DBG_DTDM("RS twosided stencil %u / %u", cDsc.DepthStencilState.StencilEnable, Value);//megai2: default stencil uses 2 sides in dx12. tricky!
		break; //185,   /* BOOL enable/disable 2 sided stenciling */

	case D3DRS_CCW_STENCILFAIL:
		cDsc.DepthStencilState.BackFace.StencilFailOp = (D3D12_STENCIL_OP)Value;
		break; //186,   /* D3DSTENCILOP to do if ccw stencil test fails */

	case D3DRS_CCW_STENCILZFAIL:
		cDsc.DepthStencilState.BackFace.StencilDepthFailOp = (D3D12_STENCIL_OP)Value;
		break; //187,   /* D3DSTENCILOP to do if ccw stencil test passes and Z test fails */

	case D3DRS_CCW_STENCILPASS:
		cDsc.DepthStencilState.BackFace.StencilPassOp = (D3D12_STENCIL_OP)Value;
		break; //188,   /* D3DSTENCILOP to do if both ccw stencil and Z tests pass */

	case D3DRS_CCW_STENCILFUNC:
		cDsc.DepthStencilState.BackFace.StencilFunc = (D3D12_COMPARISON_FUNC)Value;
		break; //189,   /* D3DCMPFUNC fn.  ccw Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
	case D3DRS_SRGBWRITEENABLE:
#ifdef TRACK_SHADER_BUGS_PROFILE
		DX9RSvalues[State] = Value;
#endif
		;//d912pxy_s(iframe)->TST()->SetTexStage(29, Value);
		break;

	case D3DRS_DEPTHBIAS:
	{
		int depthMul;
		depthMul = (1 << 23) - 1;
		INT fixVal = (INT)(*(float*)&Value * depthMul);
		cDsc.RasterizerState.DepthBias = fixVal;
		break; //195,
	}

	case D3DRS_SEPARATEALPHABLENDENABLE:
		//LOG_DBG_DTDM("RS sep ablend %u / %u", cDsc.BlendState.RenderTarget[0].BlendEnable, Value); //megai2: same as with stencil. only one bool for blends, not twho. tricky!
		break; //206,  /* TRUE to enable a separate blending function for the alpha channel */

	case D3DRS_SRCBLENDALPHA:
		cDsc.BlendStateRT0.SrcBlendAlpha = (D3D12_BLEND)Value;
		break; //207,  /* SRC blend factor for the alpha channel when case D3DRS_SEPARATEDESTALPHAENABLE is TRUE */

	case D3DRS_DESTBLENDALPHA:
		cDsc.BlendStateRT0.DestBlendAlpha = (D3D12_BLEND)Value;
		break; //208,  /* DST blend factor for the alpha channel when case D3DRS_SEPARATEDESTALPHAENABLE is TRUE */

	case D3DRS_BLENDOPALPHA:
		cDsc.BlendStateRT0.BlendOpAlpha = (D3D12_BLEND_OP)Value;
		break; //209,  /* Blending operation for the alpha channel when case D3DRS_SEPARATEDESTALPHAENABLE is TRUE */

	case D3DRS_ALPHATESTENABLE:
	case D3DRS_ALPHAREF:
	case D3DRS_ALPHAFUNC:
		DX9RSvalues[State] = Value;
		d912pxy_s(textureState)->SetTexStage(31, (DX9RSvalues[D3DRS_ALPHATESTENABLE] & 1) | (DX9RSvalues[D3DRS_ALPHAFUNC] << 1) | (DX9RSvalues[D3DRS_ALPHAREF] << 5));
		break;
	case D3DRS_CLIPPLANEENABLE:
	{
#ifdef TRACK_SHADER_BUGS_PROFILE
		DX9RSvalues[State] = Value;
#endif
		d912pxy_s(textureState)->SetTexStage(29, Value);
		if (!Value)
		{
			float zvf4[4] = { 0, 0, 0, 0 };
			m_dev->SetPixelShaderConstantF(256 - 2, zvf4, 1);
		}
		break;
	}


	default:
		;
	}
}

void d912pxy_pso_cache::VShader(d912pxy_vshader * vs)
{
#ifdef _DEBUG
	if (vs)
		LOG_DBG_DTDM("vs = %016llX", vs->GetID());
#endif

	dirty |= 1;

	cDsc.VS = vs;	
}

void d912pxy_pso_cache::PShader(d912pxy_pshader * ps)
{
#ifdef _DEBUG
	if (ps)
		LOG_DBG_DTDM("ps = %016llX", ps->GetID());
#endif

	dirty |= 1;
	
	cDsc.PS = ps;
}

void d912pxy_pso_cache::IAFormat(d912pxy_vdecl * vertexDcl)
{
	mVDecl = vertexDcl;

	dirty |= 1;

	cDsc.InputLayout = vertexDcl;
}

void d912pxy_pso_cache::IAFormatInstanced(d912pxy_vdecl * vertexDcl)
{
	dirty |= 1;

	cDsc.InputLayout = vertexDcl;
}

void d912pxy_pso_cache::RTVFormat(DXGI_FORMAT fmt, UINT idx)
{
	dirty |= (fmt != cDsc.RTVFormat0/*[idx]*/);
	cDsc.RTVFormat0/*[idx]*/ = fmt;
}

void d912pxy_pso_cache::DSVFormat(DXGI_FORMAT fmt)
{
	dirty |= (fmt != cDsc.DSVFormat);
	cDsc.DSVFormat = fmt;
}

void d912pxy_pso_cache::OMReflect(UINT RTcnt, D3D12_CPU_DESCRIPTOR_HANDLE * dsv)
{
	cDsc.NumRenderTargets = RTcnt;

	if (!dsv)
		cDsc.DSVFormat = DXGI_FORMAT_UNKNOWN;
}

DWORD d912pxy_pso_cache::GetDX9RsValue(D3DRENDERSTATETYPE State)
{
	switch (State)
	{
	case D3DRS_ALPHABLENDENABLE:
		return cDsc.BlendStateRT0.BlendEnable;
		break;
	case D3DRS_DESTBLEND:
		return cDsc.BlendStateRT0.DestBlend;
		break;
	default:
		return DX9RSvalues[State];
	}

	return 0;	
}

UINT d912pxy_pso_cache::Use()
{
	if (dirty)
	{
		d912pxy_s(CMDReplay)->PSORaw(&cDsc);

		dirty = 0;
	} 


	return 1;
}

UINT d912pxy_pso_cache::UseCompiled(d912pxy_pso_cache_item * it)
{
	d912pxy_s(CMDReplay)->PSOCompiled(it);

	dirty = 0;

	return 1;
}

UINT d912pxy_pso_cache::UseWithFeedbackPtr(void ** feedback)
{
	d912pxy_s(CMDReplay)->PSORawFeedback(&cDsc, feedback);

	//megai2: force dirty to reset PSO to current state data 
	dirty = 1;	

	return 1;
}

d912pxy_pso_cache_item* d912pxy_pso_cache::UseByDesc(d912pxy_trimmed_dx12_pso* dsc, UINT32 frameStartTime)
{
	dsc->vdeclHash = dsc->InputLayout->GetHash();

	cacheIndexes->PointAt((void*)((intptr_t)dsc + d912pxy_trimmed_dx12_pso_hash_offset));

	UINT64 id = cacheIndexes->CurrentCID();

	if (id == 0)
	{
		cacheIndexes->SetValue(++cacheIncID);
		id = cacheIncID;
	}

	if (!dsc->VS || !dsc->PS || !dsc->InputLayout)
		return NULL;
			
	d912pxy_pso_cache_item* item = d912pxy_s(sdb)->GetPair(dsc->VS, dsc->PS)->GetPSOCacheData((UINT32)id, dsc);

	return item;
}

void d912pxy_pso_cache::SetRootSignature(ComPtr<ID3D12RootSignature> sig)
{
	dirty |= 1;
	d912pxy_pso_cache::cDscBase.pRootSignature = sig.Get();
}

void d912pxy_pso_cache::MarkDirty(UINT force)
{
	frameCl = d912pxy_s(GPUcl)->GID(CLG_SEQ).Get();

	dirty |= 1 | (force < 1);
}

d912pxy_pshader * d912pxy_pso_cache::GetPShader()
{
	return cDsc.PS;
}

d912pxy_vshader * d912pxy_pso_cache::GetVShader()
{
	return cDsc.VS;
}

void d912pxy_pso_cache::ThreadJob()
{
	while (psoCompileBuffer->HaveElements())
	{
		d912pxy_pso_cache_item* it = psoCompileBuffer->GetElement();

		it->Compile();

		psoCompileBuffer->Next();
	}

	ProcessShaderCleanup();

	/*if (m_dev->InterruptThreads())
	{
		m_dev->LockThread(PXY_INNER_THREADID_PSO_COMPILER);
	}*/
}

void d912pxy_pso_cache::QueueShaderCleanup(d912pxy_shader * v)
{
	shaderCleanupBuffer->WriteElement(v);
}

void d912pxy_pso_cache::ProcessShaderCleanup()
{
	while (shaderCleanupBuffer->HaveElements())
	{
		d912pxy_shader* it = shaderCleanupBuffer->GetElement();
		d912pxy_ringbuffer<UINT32>* pairs = it->GetPairs();
		shaderCleanupBuffer->Next();

		while (pairs->HaveElements())
		{
			UINT32 ha = pairs->GetElement();

			d912pxy_s(sdb)->DeletePair(ha);

			pairs->Next();
		}

		delete it;
	}
}

void d912pxy_pso_cache::CompileItem(d912pxy_pso_cache_item * item)
{
	psoCompileBuffer->WriteElement(item);
	SignalWork();
}

UINT d912pxy_pso_cache::IsCompileQueueFree()
{
	return (psoCompileBuffer->HaveElements() == 0);
}

d912pxy_pso_cache_item::d912pxy_pso_cache_item(d912pxy_device * dev, d912pxy_trimmed_dx12_pso* sDsc) : d912pxy_noncom(dev, L"PSO item")
{
	//m_status = 0;
	retPtr = NULL;
	obj = nullptr;
	desc = (d912pxy_trimmed_dx12_pso*)malloc(sizeof(d912pxy_trimmed_dx12_pso));
	*desc = *sDsc;

	desc->VS->ThreadRef(1);
	desc->PS->ThreadRef(1);
	desc->InputLayout->ThreadRef(1);
}

void d912pxy_pso_cache_item::Compile()
{	
	d912pxy_vshader* vsObj = desc->VS;
	d912pxy_pshader* psObj = desc->PS;
	d912pxy_vdecl* vdclObj = desc->InputLayout;
		
	try {
		d912pxy_pso_cache::cDscBase.VS = *vsObj->GetCode();
		d912pxy_pso_cache::cDscBase.PS = *psObj->GetCode();
	}
	catch (...) {
		m_log->P7_ERROR(LGC_DEFAULT, TM("final error compiling shader pair VS %016llX PS %016llX"), vsObj->GetID(), psObj->GetID());

		vsObj->ThreadRef(-1);
		psObj->ThreadRef(-1);
		vdclObj->ThreadRef(-1);

		free(desc);

		//m_status = 2;

		return;
	}
	d912pxy_pso_cache::cDscBase.InputLayout = vdclObj->GetD12IA_InputElementFmt();

	d912pxy_pso_cache::cDscBase.NumRenderTargets = desc->NumRenderTargets;
	d912pxy_pso_cache::cDscBase.BlendState.RenderTarget[0] = desc->BlendStateRT0;
	d912pxy_pso_cache::cDscBase.RasterizerState = desc->RasterizerState;
	d912pxy_pso_cache::cDscBase.DepthStencilState = desc->DepthStencilState;
	d912pxy_pso_cache::cDscBase.RTVFormats[0] = desc->RTVFormat0;
	d912pxy_pso_cache::cDscBase.DSVFormat = desc->DSVFormat;

	LOG_DBG_DTDM("Compiling PSO with vs = %016llX , ps = %016llX", vsObj->GetID(), psObj->GetID());

	/*d912pxy_shader_uid shaderBlacklist[] = {
		0xF080FCE66894DD82,
		0xE4749555EA1CA6AD,
		0x985FE509D86757E1,
		0x7814708E60D7D98A,
		0xDC863F8647D6B899,
		0x6C9683BF7AA4A47B,
		0
	};

	int sblId = 0;
	int ignoreShader = 0;
	while (shaderBlacklist[sblId] != 0)
	{
		if ((vsObj->GetID() == shaderBlacklist[sblId]) || (psObj->GetID() == shaderBlacklist[sblId]))
		{
			ignoreShader = 1;
			break;
		}
		++sblId;
	}

	if (!ignoreShader)*/
	{
		try {
			LOG_ERR_THROW2(d912pxy_s(DXDev)->CreateGraphicsPipelineState(&d912pxy_pso_cache::cDscBase, IID_PPV_ARGS(&obj)), "PSO item are not created");
		}
		catch (...)
		{
			m_log->P7_ERROR(LGC_DEFAULT, TM("CreateGraphicsPipelineState error for VS %016llX PS %016llX"), vsObj->GetID(), psObj->GetID());

			char dumpString[sizeof(d912pxy_trimmed_dx12_pso)*2 + 1];
			dumpString[0] = 0;

			for (int i = 0; i != sizeof(d912pxy_trimmed_dx12_pso); ++i)
			{
				char tmp[3];
				sprintf(tmp, "%02X", ((UINT8*)desc)[i]);
				dumpString[i * 2] = tmp[0];
				dumpString[i * 2+1] = tmp[1];
			}

			dumpString[sizeof(d912pxy_trimmed_dx12_pso) * 2] = 0;

			m_log->P7_ERROR(LGC_DEFAULT, TM("trimmed pso dump %S"), dumpString);

			vsObj->ThreadRef(-1);
			psObj->ThreadRef(-1);
			vdclObj->ThreadRef(-1);

			free(desc);

			//m_status = 2;

			return;
		}
		InterlockedExchange((unsigned long long *)&retPtr, (unsigned long long)obj.Get());
	}

	//m_status = 1;

	vsObj->ThreadRef(-1);
	psObj->ThreadRef(-1);
	vdclObj->ThreadRef(-1);

	free(desc);
}

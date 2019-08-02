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

D3D12_GRAPHICS_PIPELINE_STATE_DESC d912pxy_pso_cache::cDscBase;
UINT d912pxy_pso_cache::allowRealtimeChecks = 0;

d912pxy_pso_cache::d912pxy_pso_cache() 
{

}

d912pxy_pso_cache::~d912pxy_pso_cache()
{
	Stop();
	delete psoCompileBuffer;

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

void d912pxy_pso_cache::Init()
{
	NonCom_Init(L"PSO cache");

	InitThread("d912pxy pso compile", 0);

	cacheIndexes = new d912pxy_memtree2(sizeof(d912pxy_trimmed_dx12_pso) - d912pxy_trimmed_dx12_pso_hash_offset, PXY_INNER_MAX_PSO_CACHE_NODES, 2);
	cacheIncID = 0;

	fileCacheFlags = (UINT8)d912pxy_s.config.GetValueUI64(PXY_CFG_SDB_USE_PSO_KEY_CACHE);

	cCPSO = NULL;

	d912pxy_pso_cache::allowRealtimeChecks = d912pxy_s.config.GetValueUI32(PXY_CFG_SDB_ALLOW_REALTIME_CHECKS);

	ZeroMemory(&d912pxy_pso_cache::cDscBase, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	cDsc.NumRenderTargets = 1;
	cDsc.RTVFormat0 = DXGI_FORMAT_B8G8R8A8_UNORM;
	cDsc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d912pxy_pso_cache::cDscBase.SampleDesc.Count = 1;
	d912pxy_pso_cache::cDscBase.SampleDesc.Quality = 0;
	d912pxy_pso_cache::cDscBase.SampleMask = 0xFFFFFFFF;
	d912pxy_pso_cache::cDscBase.RasterizerState.DepthBiasClamp = 0;
	d912pxy_pso_cache::cDscBase.RasterizerState.DepthClipEnable = 1;
	d912pxy_pso_cache::cDscBase.GS.pShaderBytecode = NULL;
	d912pxy_pso_cache::cDscBase.DS.pShaderBytecode = NULL;
	d912pxy_pso_cache::cDscBase.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	for (int i = 0; i!= 226;++i)
		DX9RSvalues[i] = 7;

	SetStateTracked(D3DRS_ZENABLE, 1);
	SetStateTracked(D3DRS_FILLMODE, D3DFILL_SOLID);
	SetStateTracked(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	SetStateTracked(D3DRS_ZWRITEENABLE, 1);
	SetStateTracked(D3DRS_ALPHATESTENABLE, 0);
	SetStateTracked(D3DRS_LASTPIXEL, 1);
	SetStateTracked(D3DRS_SRCBLEND, D3DBLEND_ONE);
	SetStateTracked(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	SetStateTracked(D3DRS_CULLMODE, D3DCULL_CCW);
	SetStateTracked(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	SetStateTracked(D3DRS_ALPHAREF, 0);
	SetStateTracked(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);
	SetStateTracked(D3DRS_DITHERENABLE, 0);
	SetStateTracked(D3DRS_ALPHABLENDENABLE, 0);
	SetStateTracked(D3DRS_FOGENABLE, 0);
	SetStateTracked(D3DRS_SPECULARENABLE, 0);
	SetStateTracked(D3DRS_FOGCOLOR, 0);
	SetStateTracked(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
	SetStateTracked(D3DRS_FOGSTART, 0);

	float fValue = 1.0f;
	SetStateTracked(D3DRS_FOGEND, *((DWORD*)(&fValue)));
	SetStateTracked(D3DRS_FOGDENSITY, *((DWORD*)(&fValue)));
	SetStateTracked(D3DRS_RANGEFOGENABLE, 0);
	SetStateTracked(D3DRS_STENCILENABLE, 0);
	SetStateTracked(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	SetStateTracked(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
	SetStateTracked(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
	SetStateTracked(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	SetStateTracked(D3DRS_STENCILREF, 0);
	SetStateTracked(D3DRS_STENCILMASK, 0xFFFFFFFF);
	SetStateTracked(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
	SetStateTracked(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
	SetStateTracked(D3DRS_WRAP0, 0);
	SetStateTracked(D3DRS_WRAP1, 0);
	SetStateTracked(D3DRS_WRAP2, 0);
	SetStateTracked(D3DRS_WRAP3, 0);
	SetStateTracked(D3DRS_WRAP4, 0);
	SetStateTracked(D3DRS_WRAP5, 0);
	SetStateTracked(D3DRS_WRAP6, 0);
	SetStateTracked(D3DRS_WRAP7, 0);
	SetStateTracked(D3DRS_WRAP8, 0);
	SetStateTracked(D3DRS_WRAP9, 0);
	SetStateTracked(D3DRS_WRAP10, 0);
	SetStateTracked(D3DRS_WRAP11, 0);
	SetStateTracked(D3DRS_WRAP12, 0);
	SetStateTracked(D3DRS_WRAP13, 0);
	SetStateTracked(D3DRS_WRAP14, 0);
	SetStateTracked(D3DRS_WRAP15, 0);
	SetStateTracked(D3DRS_CLIPPING, 1);
	SetStateTracked(D3DRS_LIGHTING, 1);
	SetStateTracked(D3DRS_AMBIENT, 0);
	SetStateTracked(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
	SetStateTracked(D3DRS_COLORVERTEX, 1);
	SetStateTracked(D3DRS_LOCALVIEWER, 1);
	SetStateTracked(D3DRS_NORMALIZENORMALS, 0);
	SetStateTracked(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
	SetStateTracked(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2);
	SetStateTracked(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
	SetStateTracked(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
	SetStateTracked(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
	SetStateTracked(D3DRS_CLIPPLANEENABLE, 0);
	SetStateTracked(D3DRS_POINTSIZE, 0);
	SetStateTracked(D3DRS_POINTSIZE_MIN, *((DWORD*)(&fValue)));
	SetStateTracked(D3DRS_POINTSPRITEENABLE, 0);
	SetStateTracked(D3DRS_POINTSCALEENABLE, 0);
	SetStateTracked(D3DRS_POINTSCALE_A, *((DWORD*)(&fValue)));
	SetStateTracked(D3DRS_POINTSCALE_B, 0);
	SetStateTracked(D3DRS_POINTSCALE_C, 0);
	SetStateTracked(D3DRS_MULTISAMPLEANTIALIAS, 1);
	SetStateTracked(D3DRS_MULTISAMPLEMASK, 0xFFFFFFFF);
	SetStateTracked(D3DRS_PATCHEDGESTYLE, D3DPATCHEDGE_DISCRETE);
	SetStateTracked(D3DRS_DEBUGMONITORTOKEN, D3DDMT_ENABLE);

	fValue = 64.0f;
	SetStateTracked(D3DRS_POINTSIZE_MAX, *((DWORD*)(&fValue)));
	SetStateTracked(D3DRS_INDEXEDVERTEXBLENDENABLE, 0);
	SetStateTracked(D3DRS_COLORWRITEENABLE, 0xF);
	SetStateTracked(D3DRS_TWEENFACTOR, 0);
	SetStateTracked(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	SetStateTracked(D3DRS_POSITIONDEGREE, D3DDEGREE_CUBIC);
	SetStateTracked(D3DRS_NORMALDEGREE, D3DDEGREE_LINEAR);
	SetStateTracked(D3DRS_SCISSORTESTENABLE, 0);
	SetStateTracked(D3DRS_SLOPESCALEDEPTHBIAS, 0);
	SetStateTracked(D3DRS_ANTIALIASEDLINEENABLE, 0);

	fValue = 1.0f;
	SetStateTracked(D3DRS_MINTESSELLATIONLEVEL, *((DWORD*)(&fValue)));
	SetStateTracked(D3DRS_MAXTESSELLATIONLEVEL, *((DWORD*)(&fValue)));
	SetStateTracked(D3DRS_ADAPTIVETESS_X, 0);
	SetStateTracked(D3DRS_ADAPTIVETESS_Y, 0);
	SetStateTracked(D3DRS_ADAPTIVETESS_Z, *((DWORD*)(&fValue)));
	SetStateTracked(D3DRS_ADAPTIVETESS_W, 0);
	SetStateTracked(D3DRS_ENABLEADAPTIVETESSELLATION, 0);
	SetStateTracked(D3DRS_TWOSIDEDSTENCILMODE, 0);
	SetStateTracked(D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_KEEP);
	SetStateTracked(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP);
	SetStateTracked(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_KEEP);
	SetStateTracked(D3DRS_CCW_STENCILFUNC, D3DCMP_NEVER);
	SetStateTracked(D3DRS_COLORWRITEENABLE1, 0xF);
	SetStateTracked(D3DRS_COLORWRITEENABLE2, 0xF);
	SetStateTracked(D3DRS_COLORWRITEENABLE3, 0xF);
	SetStateTracked(D3DRS_BLENDFACTOR, 0xffffffff);
	SetStateTracked(D3DRS_SRGBWRITEENABLE, 0);
	SetStateTracked(D3DRS_DEPTHBIAS, 0);
	SetStateTracked(D3DRS_SEPARATEALPHABLENDENABLE, 0);
	SetStateTracked(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	SetStateTracked(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);
	SetStateTracked(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);

	dirty = 1;

	psoCompileBuffer = new d912pxy_ringbuffer<d912pxy_pso_cache_item*>(0xFFFF, 0);
}

void d912pxy_pso_cache::State(D3DRENDERSTATETYPE State, DWORD Value)
{		
	LOG_DBG_DTDM("RS[%u]=%u", State, Value);

	switch (State)
	{
	case D3DRS_BLENDFACTOR:
	{
		d912pxy_s.render.replay.OMBlendFac(TransformBlendFactor(Value).val);		
		DX9RSvalues[State] = Value;
	}
	break; //193,   /* D3DCOLOR used for a constant blend factor during alpha blending for devices that support D3DPBLENDCAPS_BLENDFACTOR */
	case D3DRS_STENCILREF:
		d912pxy_s.render.replay.OMStencilRef(Value);
		DX9RSvalues[State] = Value;
		break; //57,   /* Reference value used in stencil test */
	case D3DRS_SCISSORTESTENABLE:
		if (Value)
			d912pxy_s.render.iframe.RestoreScissor();
		else
			d912pxy_s.render.iframe.IgnoreScissor();
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
		DX9RSvalues[State] = Value;
		cDsc.BlendStateRT0.SrcBlend = (D3D12_BLEND)Value;

		if (!DX9RSvalues[D3DRS_SEPARATEALPHABLENDENABLE])
			cDsc.BlendStateRT0.SrcBlendAlpha = (D3D12_BLEND)TransformBlend2AlphaBlend(Value);

		break; //19,   /* D3DBLEND */

	case D3DRS_DESTBLEND:
		DX9RSvalues[State] = Value;
		cDsc.BlendStateRT0.DestBlend = (D3D12_BLEND)Value;
		
		if (!DX9RSvalues[D3DRS_SEPARATEALPHABLENDENABLE])
			cDsc.BlendStateRT0.DestBlendAlpha = (D3D12_BLEND)TransformBlend2AlphaBlend(Value);

		break; //20,   /* D3DBLEND */

	case D3DRS_CULLMODE:
		cDsc.RasterizerState.CullMode = (D3D12_CULL_MODE)Value;
		break; //22,   /* D3DCULL */

	case D3DRS_ZFUNC:
		cDsc.DepthStencilState.DepthFunc = (D3D12_COMPARISON_FUNC)Value;
		break; //23,   /* D3DCMPFUNC */

	case D3DRS_ALPHABLENDENABLE:
		cDsc.BlendStateRT0.BlendEnable = (UINT8)Value;
		//FIXME! must set this to all active RT's somewhere
		break; //27,   /* TRUE to enable alpha blending */

	case D3DRS_COLORWRITEENABLE:
		{
			d912pxy_s.render.iframe.OptimizeZeroWriteRT(Value);
			cDsc.BlendStateRT0.RenderTargetWriteMask = Value & 0xF;
		}		
		break; //168,  // per-channel write enable
	case D3DRS_COLORWRITEENABLE1: 
	case D3DRS_COLORWRITEENABLE2: 
	case D3DRS_COLORWRITEENABLE3: 
		//cDsc.BlendStateRT0.RenderTarget[State - D3DRS_COLORWRITEENABLE1 + 1].RenderTargetWriteMask = Value & 0xF;
		break;

	case D3DRS_BLENDOP:
		DX9RSvalues[State] = Value;
		cDsc.BlendStateRT0.BlendOp = (D3D12_BLEND_OP)Value;

		if (!DX9RSvalues[D3DRS_SEPARATEALPHABLENDENABLE])
			cDsc.BlendStateRT0.BlendOpAlpha = (D3D12_BLEND_OP)Value;
		break; //171,   // D3DBLENDOP setting

	case D3DRS_SLOPESCALEDEPTHBIAS:
		cDsc.RasterizerState.SlopeScaledDepthBias = *(float*)(&Value);
		break; //175,

	case D3DRS_ANTIALIASEDLINEENABLE:
		cDsc.RasterizerState.AntialiasedLineEnable = (UINT8)Value;
		break; //176,

	//stencil
	case D3DRS_STENCILMASK:
		cDsc.DepthStencilState.StencilReadMask = Value & 0xFF;
		break; //58,   /* Mask value used in stencil test */

	case D3DRS_STENCILWRITEMASK:
		cDsc.DepthStencilState.StencilWriteMask = Value & 0xFF;
		break; //59,   /* Write mask applied to values written to stencil buffer */

	case D3DRS_STENCILENABLE:
		cDsc.DepthStencilState.StencilEnable = (UINT8)Value;
		break; //52,   /* BOOL enable/disable stenciling */

	case D3DRS_TWOSIDEDSTENCILMODE:
	{
		LOG_DBG_DTDM("RS twosided stencil %u / %u", cDsc.DepthStencilState.StencilEnable, Value);
		DX9RSvalues[State] = Value;
		if (!Value)
		{
			cDsc.DepthStencilState.BackFace.StencilPassOp = (UINT8)DX9RSvalues[D3DRS_STENCILPASS];
			cDsc.DepthStencilState.BackFace.StencilFailOp = (UINT8)DX9RSvalues[D3DRS_STENCILFAIL];
			cDsc.DepthStencilState.BackFace.StencilDepthFailOp = (UINT8)DX9RSvalues[D3DRS_STENCILZFAIL];
			cDsc.DepthStencilState.BackFace.StencilFunc = (UINT8)DX9RSvalues[D3DRS_STENCILFUNC];
		}
		else {
			cDsc.DepthStencilState.BackFace.StencilFailOp = (UINT8)DX9RSvalues[D3DRS_CCW_STENCILFAIL];
			cDsc.DepthStencilState.BackFace.StencilDepthFailOp = (UINT8)DX9RSvalues[D3DRS_CCW_STENCILZFAIL];
			cDsc.DepthStencilState.BackFace.StencilPassOp = (UINT8)DX9RSvalues[D3DRS_CCW_STENCILPASS];
			cDsc.DepthStencilState.BackFace.StencilFunc = (UINT8)DX9RSvalues[D3DRS_CCW_STENCILFUNC];
			cDsc.DepthStencilState.StencilEnable = 1;
		}
	}
	break; //185,   /* BOOL enable/disable 2 sided stenciling */

	case D3DRS_STENCILFAIL:
		cDsc.DepthStencilState.FrontFace.StencilFailOp = (D3D12_STENCIL_OP)Value;
		if (!DX9RSvalues[D3DRS_TWOSIDEDSTENCILMODE])
			cDsc.DepthStencilState.BackFace.StencilFailOp = (D3D12_STENCIL_OP)Value;
		DX9RSvalues[State] = Value;
		break; //53,   /* D3DSTENCILOP to do if stencil test fails */

	case D3DRS_STENCILZFAIL:
		cDsc.DepthStencilState.FrontFace.StencilDepthFailOp = (D3D12_STENCIL_OP)Value;
		if (!DX9RSvalues[D3DRS_TWOSIDEDSTENCILMODE])
			cDsc.DepthStencilState.BackFace.StencilDepthFailOp = (D3D12_STENCIL_OP)Value;
		DX9RSvalues[State] = Value;
		break; //54,   /* D3DSTENCILOP to do if stencil test passes and Z test fails */

	case D3DRS_STENCILPASS:
		cDsc.DepthStencilState.FrontFace.StencilPassOp = (D3D12_STENCIL_OP)Value;
		if (!DX9RSvalues[D3DRS_TWOSIDEDSTENCILMODE])
			cDsc.DepthStencilState.BackFace.StencilPassOp = (D3D12_STENCIL_OP)Value;
		DX9RSvalues[State] = Value;
		break; //55,   /* D3DSTENCILOP to do if both stencil and Z tests pass */

	case D3DRS_STENCILFUNC:
		cDsc.DepthStencilState.FrontFace.StencilFunc = (D3D12_COMPARISON_FUNC)Value;
		if (!DX9RSvalues[D3DRS_TWOSIDEDSTENCILMODE])
			cDsc.DepthStencilState.BackFace.StencilFunc = (D3D12_COMPARISON_FUNC)Value;
		DX9RSvalues[State] = Value;
		break; //56,   /* D3DCMPFUNC fn.  Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */

	case D3DRS_CCW_STENCILFAIL:
		if (DX9RSvalues[D3DRS_TWOSIDEDSTENCILMODE])
			cDsc.DepthStencilState.BackFace.StencilFailOp = (D3D12_STENCIL_OP)Value;
		DX9RSvalues[State] = Value;
		break; //186,   /* D3DSTENCILOP to do if ccw stencil test fails */

	case D3DRS_CCW_STENCILZFAIL:
		if (DX9RSvalues[D3DRS_TWOSIDEDSTENCILMODE])
			cDsc.DepthStencilState.BackFace.StencilDepthFailOp = (D3D12_STENCIL_OP)Value;
		DX9RSvalues[State] = Value;
		break; //187,   /* D3DSTENCILOP to do if ccw stencil test passes and Z test fails */

	case D3DRS_CCW_STENCILPASS:
		if (DX9RSvalues[D3DRS_TWOSIDEDSTENCILMODE])
			cDsc.DepthStencilState.BackFace.StencilPassOp = (D3D12_STENCIL_OP)Value;
		DX9RSvalues[State] = Value;
		break; //188,   /* D3DSTENCILOP to do if both ccw stencil and Z tests pass */

	case D3DRS_CCW_STENCILFUNC:
		if (DX9RSvalues[D3DRS_TWOSIDEDSTENCILMODE])
			cDsc.DepthStencilState.BackFace.StencilFunc = (D3D12_COMPARISON_FUNC)Value;
		DX9RSvalues[State] = Value;
		break; //189,   /* D3DCMPFUNC fn.  ccw Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */

	case D3DRS_SRGBWRITEENABLE:
		DX9RSvalues[State] = Value;
		d912pxy_s.render.tex.ModStageBit(31, 13, Value);
		//d912pxy_s.render.iframe.TST()->SetTexStage(29, Value);
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
		DX9RSvalues[State] = Value;

		if (Value)
		{
			cDsc.BlendStateRT0.SrcBlendAlpha = (D3D12_BLEND)DX9RSvalues[D3DRS_SRCBLENDALPHA];
			cDsc.BlendStateRT0.DestBlendAlpha = (D3D12_BLEND)DX9RSvalues[D3DRS_DESTBLENDALPHA];
			cDsc.BlendStateRT0.BlendOpAlpha = (D3D12_BLEND)DX9RSvalues[D3DRS_BLENDOPALPHA];
		}
		else {
			cDsc.BlendStateRT0.SrcBlendAlpha = (D3D12_BLEND)TransformBlend2AlphaBlend(DX9RSvalues[D3DRS_SRCBLEND]);
			cDsc.BlendStateRT0.DestBlendAlpha = (D3D12_BLEND)TransformBlend2AlphaBlend(DX9RSvalues[D3DRS_DESTBLEND]);
			cDsc.BlendStateRT0.BlendOpAlpha = (D3D12_BLEND)DX9RSvalues[D3DRS_BLENDOP];
		}

		break; //206,  /* TRUE to enable a separate blending function for the alpha channel */

	case D3DRS_SRCBLENDALPHA:
		DX9RSvalues[State] = Value;

		if (DX9RSvalues[D3DRS_SEPARATEALPHABLENDENABLE])
			cDsc.BlendStateRT0.SrcBlendAlpha = (D3D12_BLEND)Value;
		break; //207,  /* SRC blend factor for the alpha channel when case D3DRS_SEPARATEDESTALPHAENABLE is TRUE */

	case D3DRS_DESTBLENDALPHA:
		DX9RSvalues[State] = Value;

		if (DX9RSvalues[D3DRS_SEPARATEALPHABLENDENABLE])
			cDsc.BlendStateRT0.DestBlendAlpha = (D3D12_BLEND)Value;
		break; //208,  /* DST blend factor for the alpha channel when case D3DRS_SEPARATEDESTALPHAENABLE is TRUE */

	case D3DRS_BLENDOPALPHA:
		DX9RSvalues[State] = Value;
		
		if (DX9RSvalues[D3DRS_SEPARATEALPHABLENDENABLE])
			cDsc.BlendStateRT0.BlendOpAlpha = (D3D12_BLEND_OP)Value;

		break; //209,  /* Blending operation for the alpha channel when case D3DRS_SEPARATEDESTALPHAENABLE is TRUE */

	case D3DRS_ALPHATESTENABLE:
	case D3DRS_ALPHAREF:
	case D3DRS_ALPHAFUNC:
		DX9RSvalues[State] = Value;
		d912pxy_s.render.tex.SetTexture(31, (DX9RSvalues[D3DRS_ALPHATESTENABLE] & 1) | (DX9RSvalues[D3DRS_ALPHAFUNC] << 1) | (DX9RSvalues[D3DRS_ALPHAREF] << 5));
		break;
	case D3DRS_CLIPPLANEENABLE:
	{
		DX9RSvalues[State] = Value;		
		if (!Value)
		{
			const float zvf4[4] = { 0, 0, 0, 0 };
			d912pxy_s.render.batch.SetShaderConstF(1, PXY_INNER_EXTRA_SHADER_CONST_CLIP_P0, 1, (float*)zvf4);
		}
		break;
	}


	default:
		;
	}
}

void d912pxy_pso_cache::SetState(D3DRENDERSTATETYPE State, DWORD Value)
{
	this->State(State, Value);

	dirty |= 1;
}

void d912pxy_pso_cache::SetStateTracked(D3DRENDERSTATETYPE State, DWORD Value)
{
	if (DX9RSvalues[State] != Value)
	{
		SetState(State, Value);
		DX9RSvalues[State] = Value;
	}
}

void d912pxy_pso_cache::VShader(d912pxy_shader * vs)
{
#ifdef _DEBUG
	if (vs)
		LOG_DBG_DTDM("vs = %016llX", vs->GetID());
#endif

	dirty |= 1;

	cDsc.VS = vs;	
}

void d912pxy_pso_cache::PShader(d912pxy_shader * ps)
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
		d912pxy_s.render.replay.PSORaw(&cDsc);

		dirty = 0;		
	} 


	return 1;
}

UINT d912pxy_pso_cache::UseCompiled(d912pxy_pso_cache_item * it)
{	
	if (it)
	{
		d912pxy_s.render.replay.PSOCompiled(it);
		cCPSO = it;

		dirty = 0;
	}
	else
		cCPSO = NULL;

	return 1;
}

UINT d912pxy_pso_cache::UseWithFeedbackPtr(void ** feedback)
{
	d912pxy_s.render.replay.PSORawFeedback(&cDsc, feedback);

	//megai2: force dirty to reset PSO to current state data 
	dirty = 1;	

	return 1;
}

d912pxy_pso_cache_item* d912pxy_pso_cache::UseByDesc(d912pxy_trimmed_dx12_pso* dsc, UINT32 frameStartTime)
{
	dsc->vdeclHash = dsc->InputLayout->GetHash();

	void* dscMem = (void*)((intptr_t)dsc + d912pxy_trimmed_dx12_pso_hash_offset);

	cacheIndexes->PointAt32(dscMem);

	UINT64 id = cacheIndexes->CurrentCID();

	if (id == 0)
	{
		cacheIndexes->SetValue(++cacheIncID);
		id = cacheIncID;

		SaveKeyToCache(id, dsc);
	}

	if (!dsc->VS || !dsc->PS || !dsc->InputLayout)
		return NULL;	

	d912pxy_pso_cache_item* item = d912pxy_s.render.db.shader.GetPair(dsc->VS, dsc->PS)->GetPSOCacheData((UINT32)id, dsc);

	return item;
}

d912pxy_pso_cache_item * d912pxy_pso_cache::GetByDescMT(d912pxy_trimmed_dx12_pso * dsc, UINT32 frameStartTime)
{
	dsc->vdeclHash = dsc->InputLayout->GetHash();

	UINT32 dscHash = cacheIndexes->memHash32((void*)((intptr_t)dsc + d912pxy_trimmed_dx12_pso_hash_offset));
	UINT64 id = cacheIndexes->PointAtMemMTR(&dscHash, 4);

	if (id == 0)
	{
		id = cacheIndexes->PointAtMemMTRW(&dscHash, 4);

		if (!id)
			id = ++cacheIncID;

		cacheIndexes->PointAtMemMTW(id);

		SaveKeyToCache(id, dsc);
	}

	if (!dsc->VS || !dsc->PS || !dsc->InputLayout)
		return NULL;

	return d912pxy_s.render.db.shader.GetPair(dsc->VS, dsc->PS)->GetPSOCacheDataMT((UINT32)id, dsc);
}

ID3D12PipelineState* d912pxy_pso_cache::UseByDescMT(d912pxy_trimmed_dx12_pso * dsc, UINT32 frameStartTime)
{
	dsc->vdeclHash = dsc->InputLayout->GetHash();
	
	UINT32 dscHash = cacheIndexes->memHash32((void*)((intptr_t)dsc + d912pxy_trimmed_dx12_pso_hash_offset));
	UINT64 id = cacheIndexes->PointAtMemMTR(&dscHash, 4);
	
	if (id == 0)
	{	
		id = cacheIndexes->PointAtMemMTRW(&dscHash, 4);

		if (!id)		
			id = ++cacheIncID;		

		cacheIndexes->PointAtMemMTW(id);
		
		SaveKeyToCache(id, dsc);
	}

	if (!dsc->VS || !dsc->PS || !dsc->InputLayout)
	{
		LOG_DBG_DTDM3("fixed pipe draw issued, skipping");
		return NULL;
	}
	
	return d912pxy_s.render.db.shader.GetPair(dsc->VS, dsc->PS)->GetPSOCacheDataMT((UINT32)id, dsc)->GetPtr();
}

void d912pxy_pso_cache::SetRootSignature(ComPtr<ID3D12RootSignature> sig)
{
	dirty |= 1;
	d912pxy_pso_cache::cDscBase.pRootSignature = sig.Get();
}

void d912pxy_pso_cache::MarkDirty(UINT force)
{
	frameCl = d912pxy_s.dx12.cl->GID(CLG_SEQ);

	dirty |= 1 | (force < 1);
}

d912pxy_shader * d912pxy_pso_cache::GetPShader()
{
	return cDsc.PS;
}

d912pxy_shader * d912pxy_pso_cache::GetVShader()
{
	return cDsc.VS;
}

void d912pxy_pso_cache::ThreadJob()
{
	CheckExternalLock();

	while (psoCompileBuffer->HaveElements())
	{
		d912pxy_pso_cache_item* it = psoCompileBuffer->GetElement();

 		it->Compile();
		it->Release();
			
		psoCompileBuffer->Next();

		CheckExternalLock();
	}
}

void d912pxy_pso_cache::CompileItem(d912pxy_pso_cache_item * item)
{
	item->AddRef();
	compileQueLock.Hold();
	psoCompileBuffer->WriteElement(item);
	compileQueLock.Release();
	SignalWork();
}

UINT d912pxy_pso_cache::IsCompileQueueFree()
{
	return (psoCompileBuffer->HaveElements() == 0);
}

void d912pxy_pso_cache::LockCompileQue(UINT lock)
{
	if (lock)
	{
		if (externalLock.GetValue())
			return;

		externalLock.SetValue(1);

		SignalWork();

		externalLock.Wait(2);
	}
	else
		externalLock.SetValue(0);
}

void d912pxy_pso_cache::LoadCachedData()
{
	if (fileCacheFlags & PXY_PSO_CACHE_KEYFILE_READ)
	{
		UINT fsz = 0;
		UINT32* max_ = (UINT32*)d912pxy_s.vfs.LoadFileH(PXY_PSO_CACHE_KEYFILE_NAME, &fsz, PXY_VFS_BID_PSO_CACHE_KEYS); // Alrai: I had to change the name of max as it was being considered a macro in PXY_MALLOC.

		psoKeyCache = NULL;

		if (max_ && *max_)
		{
			cacheIncID = *max_;

			PXY_MALLOC(psoKeyCache, sizeof(d912pxy_serialized_pso_key*) * (*max_ + 2), d912pxy_serialized_pso_key**);

			for (int i = 1; i != (*max_+1); ++i)
			{
				d912pxy_serialized_pso_key* psoTrimmedDsc = (d912pxy_serialized_pso_key*)d912pxy_s.vfs.LoadFileH(i+1, &fsz, PXY_VFS_BID_PSO_CACHE_KEYS);

				if (fsz != sizeof(d912pxy_serialized_pso_key))
				{
					LOG_ERR_THROW2(-1, "pso key cache corrupted. size");
				}

				if (!psoTrimmedDsc)
				{
					LOG_ERR_THROW2(-1, "pso key cache corrupted. data");
				}

				cacheIndexes->PointAt32(&psoTrimmedDsc->staticPsoDesc[0]);
				cacheIndexes->SetValue(i);

				psoKeyCache[i] = psoTrimmedDsc;
			}

			if (d912pxy_s.render.db.shader.GetPrecompileFlag() & PXY_SDB_PSO_PRECOMPILE_LOAD)
			{
				d912pxy_memtree2* mt = d912pxy_s.vfs.GetHeadTree(PXY_VFS_BID_PSO_PRECOMPILE_LIST);

				mt->Begin();

				while (!mt->IterEnd())
				{
					UINT32 entryOffset = (UINT32)mt->CurrentCID();
					if (entryOffset != 0)
					{
						d912pxy_shader_pair_cache_entry* entry = (d912pxy_shader_pair_cache_entry*)d912pxy_s.vfs.GetCachePointer(entryOffset, PXY_VFS_BID_PSO_PRECOMPILE_LIST);

						d912pxy_shader* vs = NULL;
						d912pxy_shader* ps = NULL;

						
						vs = d912pxy_shader::d912pxy_shader_com(1, 0, entry->vs);
						ps = d912pxy_shader::d912pxy_shader_com(0, 0, entry->ps);

						if (!vs || !ps)
						{
							LOG_ERR_DTDM("Shader pair VS: %llX PS: %llX load fail", entry->vs, entry->ps);					

							mt->Next();
							continue;
						}

						for (int i = 1; i != (*max_ + 1); ++i)
						{
							if (entry->compiled[i >> 6] & (1ULL << (i & 0x3F)))
							{
								d912pxy_trimmed_dx12_pso dsc;

								void* dscMem = (void*)((intptr_t)&dsc + d912pxy_trimmed_dx12_pso_hash_offset);

								memcpy(dscMem, psoKeyCache[i]->staticPsoDesc, d912pxy_trimmed_pso_static_data_size);

								d912pxy_vdecl* vdcl;
								d912pxy_s.dev.CreateVertexDeclaration(psoKeyCache[i]->declData, (IDirect3DVertexDeclaration9**)&vdcl);

								dsc.PS = ps;
								dsc.VS = vs;
								dsc.InputLayout = vdcl;

								d912pxy_shader_pair* pair = d912pxy_s.render.db.shader.GetPair(vs, ps);

								pair->PrecompilePSO(i, &dsc);

								//UseByDesc(&dsc, 0);

								vdcl->Release();
							}
						}

						vs->Release();
						ps->Release();
					}

					mt->Next();
				}
			}

			for (int i = 1; i != (*max_ +1); ++i)
			{
				PXY_FREE(psoKeyCache[i]);
			}

			PXY_FREE(psoKeyCache);
			PXY_FREE(max_);
		}
	}
}

d912pxy_trimmed_dx12_pso * d912pxy_pso_cache::GetCurrentDsc()
{
	return &cDsc;
}

void d912pxy_pso_cache::SaveKeyToCache(UINT64 id, d912pxy_trimmed_dx12_pso * dsc)
{
	if (fileCacheFlags & PXY_PSO_CACHE_KEYFILE_WRITE)
	{
		d912pxy_serialized_pso_key cacheEntry;

		UINT unused;

		memcpy(cacheEntry.declData, dsc->InputLayout->GetDeclarationPtr(&unused), sizeof(D3DVERTEXELEMENT9) * PXY_INNER_MAX_VDECL_LEN);
		memcpy(cacheEntry.staticPsoDesc, (void*)((intptr_t)dsc + d912pxy_trimmed_dx12_pso_hash_offset), d912pxy_trimmed_pso_static_data_size);

		d912pxy_s.vfs.ReWriteFileH(id + 1, &cacheEntry, sizeof(d912pxy_serialized_pso_key), PXY_VFS_BID_PSO_CACHE_KEYS);
		d912pxy_s.vfs.ReWriteFileH(PXY_PSO_CACHE_KEYFILE_NAME, &cacheIncID, 4, PXY_VFS_BID_PSO_CACHE_KEYS);
	}
}

UINT32 d912pxy_pso_cache::GetHashedKey(d912pxy_trimmed_dx12_pso * dsc)
{
	return cacheIndexes->memHash32((void*)((intptr_t)dsc + d912pxy_trimmed_dx12_pso_hash_offset));
}

fv4Color d912pxy_pso_cache::TransformBlendFactor(DWORD val)
{
	fv4Color ret;
	
	for (int i = 0; i != 4; ++i)
	{
		ret.val[i] = ((val >> (i << 3)) & 0xFF) / 255.0f;
	}

	return ret;
}

DWORD d912pxy_pso_cache::TransformBlend2AlphaBlend(DWORD val)
{
	DWORD ret = val;

	switch (ret)
	{
	case D3DBLEND_SRCCOLOR:
		ret = D3DBLEND_SRCALPHA;
		break;
	case D3DBLEND_INVSRCCOLOR:
		ret = D3DBLEND_INVSRCALPHA;
		break;
	case D3DBLEND_DESTCOLOR:
		ret = D3DBLEND_DESTALPHA;
		break;
	case D3DBLEND_INVDESTCOLOR:
		ret = D3DBLEND_INVDESTALPHA;
		break;
	default:
		;
	}

	return ret;
}

void d912pxy_pso_cache::CheckExternalLock()
{
	if (externalLock.GetValue() == 1)
	{
		externalLock.SetValue(2);

		while (externalLock.GetValue() == 2)
		{
			Sleep(1000);
		}
	}
}

d912pxy_pso_cache_item::d912pxy_pso_cache_item(d912pxy_trimmed_dx12_pso* sDsc) : d912pxy_comhandler(PXY_COM_OBJ_PSO_ITEM, L"PSO item")
{
	//m_status = 0;
	retPtr = NULL;
	obj = nullptr;

	PXY_MALLOC(desc, sizeof(d912pxy_trimmed_dx12_pso), d912pxy_trimmed_dx12_pso*);

	*desc = *sDsc;

	desc->VS->ThreadRef(1);
	desc->PS->ThreadRef(1);
	desc->InputLayout->ThreadRef(1);
}

d912pxy_pso_cache_item * d912pxy_pso_cache_item::d912pxy_pso_cache_item_com(d912pxy_trimmed_dx12_pso * sDsc)
{
	d912pxy_com_object* ret = d912pxy_s.com.AllocateComObj(PXY_COM_OBJ_PSO_ITEM);
	
	new (&ret->pso_item)d912pxy_pso_cache_item(sDsc);
	ret->vtable = d912pxy_com_route_get_vtable(PXY_COM_ROUTE_QUERY);

	return &ret->pso_item;
}

void d912pxy_pso_cache_item::Compile()
{	
	d912pxy_shader* vsObj = desc->VS;
	d912pxy_shader* psObj = desc->PS;
	d912pxy_vdecl* vdclObj = desc->InputLayout;
		
	try {
		d912pxy_pso_cache::cDscBase.VS = *vsObj->GetCode();
		d912pxy_pso_cache::cDscBase.PS = *psObj->GetCode();
	}
	catch (...) {
		LOG_ERR_DTDM("final error compiling shader pair VS %016llX PS %016llX", vsObj->GetID(), psObj->GetID());

		vsObj->ThreadRef(-1);
		psObj->ThreadRef(-1);
		vdclObj->ThreadRef(-1);

		PXY_FREE(desc);

		//m_status = 2;

		return;
	}
	d912pxy_pso_cache::cDscBase.InputLayout = *vdclObj->GetD12IA_InputElementFmt();

	d912pxy_pso_cache::cDscBase.NumRenderTargets = desc->NumRenderTargets;
	//d912pxy_pso_cache::cDscBase.BlendState.RenderTarget[0].  = desc->BlendStateRT0;
	d912pxy_pso_cache::cDscBase.BlendState.RenderTarget[0].SrcBlend = (D3D12_BLEND)desc->BlendStateRT0.SrcBlend;
	d912pxy_pso_cache::cDscBase.BlendState.RenderTarget[0].SrcBlendAlpha = (D3D12_BLEND)desc->BlendStateRT0.SrcBlendAlpha;
	d912pxy_pso_cache::cDscBase.BlendState.RenderTarget[0].DestBlend = (D3D12_BLEND)desc->BlendStateRT0.DestBlend;
	d912pxy_pso_cache::cDscBase.BlendState.RenderTarget[0].DestBlendAlpha = (D3D12_BLEND)desc->BlendStateRT0.DestBlendAlpha;
	d912pxy_pso_cache::cDscBase.BlendState.RenderTarget[0].BlendEnable = desc->BlendStateRT0.BlendEnable;
	d912pxy_pso_cache::cDscBase.BlendState.RenderTarget[0].RenderTargetWriteMask = desc->BlendStateRT0.RenderTargetWriteMask;
	d912pxy_pso_cache::cDscBase.BlendState.RenderTarget[0].BlendOp = (D3D12_BLEND_OP)desc->BlendStateRT0.BlendOp;
	d912pxy_pso_cache::cDscBase.BlendState.RenderTarget[0].BlendOpAlpha = (D3D12_BLEND_OP)desc->BlendStateRT0.BlendOpAlpha;


//	d912pxy_pso_cache::cDscBase.RasterizerState. = desc->RasterizerState.;

	d912pxy_pso_cache::cDscBase.RasterizerState.FillMode = (D3D12_FILL_MODE)desc->RasterizerState.FillMode;
	d912pxy_pso_cache::cDscBase.RasterizerState.CullMode = (D3D12_CULL_MODE)desc->RasterizerState.CullMode;
	d912pxy_pso_cache::cDscBase.RasterizerState.SlopeScaledDepthBias = desc->RasterizerState.SlopeScaledDepthBias;
	d912pxy_pso_cache::cDscBase.RasterizerState.AntialiasedLineEnable = desc->RasterizerState.AntialiasedLineEnable;
	d912pxy_pso_cache::cDscBase.RasterizerState.DepthBias = desc->RasterizerState.DepthBias;

	//d912pxy_pso_cache::cDscBase.DepthStencilState = desc->DepthStencilState;

	d912pxy_pso_cache::cDscBase.DepthStencilState.DepthEnable = desc->DepthStencilState.DepthEnable;
	d912pxy_pso_cache::cDscBase.DepthStencilState.DepthWriteMask = (D3D12_DEPTH_WRITE_MASK)desc->DepthStencilState.DepthWriteMask;
	d912pxy_pso_cache::cDscBase.DepthStencilState.DepthFunc = (D3D12_COMPARISON_FUNC)desc->DepthStencilState.DepthFunc;
	d912pxy_pso_cache::cDscBase.DepthStencilState.StencilEnable = desc->DepthStencilState.StencilEnable;
	d912pxy_pso_cache::cDscBase.DepthStencilState.FrontFace.StencilFailOp = (D3D12_STENCIL_OP)desc->DepthStencilState.FrontFace.StencilFailOp;
	d912pxy_pso_cache::cDscBase.DepthStencilState.FrontFace.StencilPassOp = (D3D12_STENCIL_OP)desc->DepthStencilState.FrontFace.StencilPassOp;
	d912pxy_pso_cache::cDscBase.DepthStencilState.FrontFace.StencilDepthFailOp = (D3D12_STENCIL_OP)desc->DepthStencilState.FrontFace.StencilDepthFailOp;
	d912pxy_pso_cache::cDscBase.DepthStencilState.FrontFace.StencilFunc = (D3D12_COMPARISON_FUNC)desc->DepthStencilState.FrontFace.StencilFunc;
	d912pxy_pso_cache::cDscBase.DepthStencilState.BackFace.StencilFailOp = (D3D12_STENCIL_OP)desc->DepthStencilState.BackFace.StencilFailOp;
	d912pxy_pso_cache::cDscBase.DepthStencilState.BackFace.StencilPassOp = (D3D12_STENCIL_OP)desc->DepthStencilState.BackFace.StencilPassOp;
	d912pxy_pso_cache::cDscBase.DepthStencilState.BackFace.StencilDepthFailOp = (D3D12_STENCIL_OP)desc->DepthStencilState.BackFace.StencilDepthFailOp;
	d912pxy_pso_cache::cDscBase.DepthStencilState.BackFace.StencilFunc = (D3D12_COMPARISON_FUNC)desc->DepthStencilState.BackFace.StencilFunc;
	d912pxy_pso_cache::cDscBase.DepthStencilState.StencilReadMask = desc->DepthStencilState.StencilReadMask;
	d912pxy_pso_cache::cDscBase.DepthStencilState.StencilWriteMask = desc->DepthStencilState.StencilWriteMask;
	

	d912pxy_pso_cache::cDscBase.RTVFormats[0] = (DXGI_FORMAT)desc->RTVFormat0;
	d912pxy_pso_cache::cDscBase.DSVFormat = (DXGI_FORMAT)desc->DSVFormat;

	if (d912pxy_pso_cache::allowRealtimeChecks)
		RealtimeIntegrityCheck();
	else {
		CreatePSO();
	}

	//m_status = 1;

	vsObj->ThreadRef(-1);
	psObj->ThreadRef(-1);
	vdclObj->ThreadRef(-1);

	PXY_FREE(desc);
}

void d912pxy_pso_cache_item::CreatePSO()
{
	LOG_DBG_DTDM("Compiling PSO with vs = %016llX , ps = %016llX", desc->VS->GetID(), desc->PS->GetID());

	try {
		LOG_ERR_THROW2(d912pxy_s.dx12.dev->CreateGraphicsPipelineState(&d912pxy_pso_cache::cDscBase, IID_PPV_ARGS(&obj)), "PSO item are not created");
	}
	catch (...)
	{
		LOG_ERR_DTDM("CreateGraphicsPipelineState error for VS %016llX PS %016llX", desc->VS->GetID(), desc->PS->GetID());

		char dumpString[sizeof(d912pxy_trimmed_dx12_pso) * 2 + 1];
		dumpString[0] = 0;

		for (int i = 0; i != sizeof(d912pxy_trimmed_dx12_pso); ++i)
		{
			char tmp[3];
			sprintf(tmp, "%02X", ((UINT8*)desc)[i]);
			dumpString[i * 2] = tmp[0];
			dumpString[i * 2 + 1] = tmp[1];
		}

		dumpString[sizeof(d912pxy_trimmed_dx12_pso) * 2] = 0;

		LOG_ERR_DTDM("trimmed pso dump %S", dumpString);
	}
	InterlockedExchange((unsigned long long *)&retPtr, (unsigned long long)obj.Get());
}

void d912pxy_pso_cache_item::CreatePSODerived(UINT64 derivedAlias)
{
	void* shdDerCSO[2];
	UINT shdDerCSOSz[2];

	shdDerCSO[0] = d912pxy_s.vfs.LoadFileH(derivedAlias, &shdDerCSOSz[0], PXY_VFS_BID_DERIVED_CSO_VS);
	shdDerCSO[1] = d912pxy_s.vfs.LoadFileH(derivedAlias, &shdDerCSOSz[1], PXY_VFS_BID_DERIVED_CSO_PS);

	d912pxy_pso_cache::cDscBase.VS.BytecodeLength = shdDerCSOSz[0];
	d912pxy_pso_cache::cDscBase.PS.BytecodeLength = shdDerCSOSz[1];
	d912pxy_pso_cache::cDscBase.VS.pShaderBytecode = shdDerCSO[0];
	d912pxy_pso_cache::cDscBase.PS.pShaderBytecode = shdDerCSO[1];

	CreatePSO();

	PXY_FREE(shdDerCSO[0]);
	PXY_FREE(shdDerCSO[1]);
}

void d912pxy_pso_cache_item::RealtimeIntegrityCheck()
{
	d912pxy_shader_pair_hash_type pairUID = d912pxy_s.render.db.shader.GetPairUID(desc->VS, desc->PS);
	UINT32 psoKey = d912pxy_s.render.db.pso.GetHashedKey(desc);
	UINT64 derivedAlias = pairUID ^ (UINT64)psoKey;

	LOG_DBG_DTDM("DX9 PSO realtime check emulation for pair %llX key %lX alias %llX", pairUID, psoKey, derivedAlias);

	//megai2: both derived cso files are present, just load them to pso and compile on dx12 side
	if (d912pxy_s.vfs.IsPresentH(derivedAlias, PXY_VFS_BID_DERIVED_CSO_PS) && d912pxy_s.vfs.IsPresentH(derivedAlias, PXY_VFS_BID_DERIVED_CSO_VS))
	{
		CreatePSODerived(derivedAlias);
		return;
	}

	char* shdSrc[2];
	UINT shdSrcSz[2];

	shdSrc[0] = (char*)d912pxy_s.vfs.LoadFileH(desc->VS->GetID(), &shdSrcSz[0], PXY_VFS_BID_SHADER_SOURCES);
	shdSrc[1] = (char*)d912pxy_s.vfs.LoadFileH(desc->PS->GetID(), &shdSrcSz[1], PXY_VFS_BID_SHADER_SOURCES);

	if (!shdSrc[0] || !shdSrc[1])
	{
		LOG_ERR_DTDM("No HLSL source available to perfrom PSO RCE for pair %llX key %lX alias %llX", pairUID, psoKey, derivedAlias);
		CreatePSO();
		return;
	}

	/*FILE* tf = fopen("d912pxy/tmp.hlsl", "wb+");
	fwrite(shdSrc[0], 1, shdSrcSz[0], tf);
	fwrite(shdSrc[1], 1, shdSrcSz[1], tf);
	fclose(tf);*/

	//megai2: pass 0 - vdecl to vs input signature typecheck
	LOG_DBG_DTDM("PSO RCE P0");	

	for (int i = 0; i != d912pxy_pso_cache::cDscBase.InputLayout.NumElements; ++i)
	{
		char* semDefPlace = strstr(shdSrc[0], d912pxy_pso_cache::cDscBase.InputLayout.pInputElementDescs[i].SemanticName);

		if (!semDefPlace)
		{
			LOG_DBG_DTDM("semantic %S not used in vs", d912pxy_pso_cache::cDscBase.InputLayout.pInputElementDescs[i].SemanticName);
			continue;
		}

		char* defLine = d912pxy_helper::StrGetCurrentLineStart(semDefPlace);
		char* replPos = strstr(defLine, "4") - 5;

		const char* newType = "float4";

		switch (d912pxy_pso_cache::cDscBase.InputLayout.pInputElementDescs[i].Format)
		{
			case DXGI_FORMAT_R32_FLOAT:
			case DXGI_FORMAT_R32G32_FLOAT:
			case DXGI_FORMAT_R32G32B32_FLOAT:
			case DXGI_FORMAT_R32G32B32A32_FLOAT:
			case DXGI_FORMAT_B8G8R8A8_UNORM:	
			case DXGI_FORMAT_R16G16_SNORM:
			case DXGI_FORMAT_R16G16B16A16_SNORM:
			case DXGI_FORMAT_R16G16_UNORM:
			case DXGI_FORMAT_R16G16B16A16_UNORM:
			case DXGI_FORMAT_R16G16_FLOAT:
			case DXGI_FORMAT_R16G16B16A16_FLOAT:
				break;
			case DXGI_FORMAT_R16G16_SINT:				
			case DXGI_FORMAT_R16G16B16A16_SINT:				
				newType = "  int4";
				break;
			case DXGI_FORMAT_R8G8B8A8_UINT:
				newType = " uint4";
				break;
		}

		memcpy(replPos, newType, 6);

	}

	//megai2: pass 1 - vs output to ps input signature ordering check
	LOG_DBG_DTDM("PSO RCE P1");

	char* vsOut[256] = { NULL };
	char* psIn[256] = { NULL };

	UINT vsOutCnt = 0;
	UINT psInCnt = 0;

	//load vs output
	{
		char* sdeclLine = strstr(shdSrc[0], "VS_OUTPUT");
		char* structDclEmt = d912pxy_helper::StrNextLine(sdeclLine);

		structDclEmt = d912pxy_helper::StrNextLine(structDclEmt);

		while (structDclEmt[0] != '}')
		{
			char* lnStart = structDclEmt;
			structDclEmt = d912pxy_helper::StrNextLine(structDclEmt);

			UINT64 lSz = (intptr_t)structDclEmt - (intptr_t)lnStart;

			PXY_MALLOC(vsOut[vsOutCnt], lSz+1, char*);

			memcpy(vsOut[vsOutCnt], lnStart, lSz);
			vsOut[vsOutCnt][lSz] = 0;
			++vsOutCnt;
		}
	}

	//load ps input
	{
		char* sdeclLine = strstr(shdSrc[1], "PS_INPUT");
		char* structDclEmt = d912pxy_helper::StrNextLine(sdeclLine);

		structDclEmt = d912pxy_helper::StrNextLine(structDclEmt);

		while (structDclEmt[0] != '}')
		{
			char* lnStart = structDclEmt;
			structDclEmt = d912pxy_helper::StrNextLine(structDclEmt);

			UINT64 lSz = (intptr_t)structDclEmt - (intptr_t)lnStart;

			PXY_MALLOC(psIn[psInCnt], lSz+1, char*);

			memcpy(psIn[psInCnt], lnStart, lSz);
			psIn[psInCnt][lSz] = 0;
			++psInCnt;
		}
	}

	//filter ps unused regs
	int filterTgt = psInCnt - 1;

	for (int i = 0; i != psInCnt; ++i)
	{	
		while (strstr(psIn[i], "unused_ireg_"))
		{
			if (filterTgt >= i)
				break;

			char* tSwp = psIn[i];
			psIn[i] = psIn[filterTgt];
			psIn[filterTgt] = tSwp;
			--filterTgt;			
		}
	}

	//find inputs in outputs and reorder last one to input sequence
	for (int i = 0; i != psInCnt; ++i)
	{
		char* inputSemantic = strstr(psIn[i], ": ") + 2;

		for (int j = 0; j != vsOutCnt; ++j)
		{
			if (strstr(vsOut[j], inputSemantic))
			{
				char* strSwp = vsOut[i];			
				vsOut[i] = vsOut[j];
				vsOut[j] = strSwp;
			}
		}
	}

	//write declaration back to VS

	{
		char* sdeclLine = strstr(shdSrc[0], "VS_OUTPUT");
		char* structDclEmt = d912pxy_helper::StrNextLine(sdeclLine);
		structDclEmt = d912pxy_helper::StrNextLine(structDclEmt);

		for (int j = 0; j != vsOutCnt; ++j)
		{
			size_t tStrLen = strlen(vsOut[j]);
			memcpy(structDclEmt, vsOut[j], tStrLen);
			structDclEmt += tStrLen;
			PXY_FREE(vsOut[j]);
		}
	}	

	//write declaration back to PS due to unused reg filtering

	{
		char* sdeclLine = strstr(shdSrc[1], "PS_INPUT");
		char* structDclEmt = d912pxy_helper::StrNextLine(sdeclLine);
		structDclEmt = d912pxy_helper::StrNextLine(structDclEmt);

		for (int j = 0; j != psInCnt; ++j)
		{
			size_t tStrLen = strlen(psIn[j]);
			memcpy(structDclEmt, psIn[j], tStrLen);
			structDclEmt += tStrLen;
			PXY_FREE(psIn[j]);
		}
	}

	/*FILE* tf2 = fopen("d912pxy/tmp2.hlsl", "wb+");
	fwrite(shdSrc[0], 1, shdSrcSz[0], tf);
	fwrite(shdSrc[1], 1, shdSrcSz[1], tf);
	fclose(tf);*/

	d912pxy_shader_replacer* replVS = new d912pxy_shader_replacer(0, 0, desc->VS->GetID(), 1);
	d912pxy_shader_replacer* replPS = new d912pxy_shader_replacer(0, 0, desc->PS->GetID(), 0);

	d912pxy_shader_code bcVS = replVS->CompileFromHLSL_MEM(d912pxy_shader_db_hlsl_dir, shdSrc[0], shdSrcSz[0], 0);
	d912pxy_shader_code bcPS = replPS->CompileFromHLSL_MEM(d912pxy_shader_db_hlsl_dir, shdSrc[1], shdSrcSz[1], 0);

	PXY_FREE(shdSrc[0]);
	PXY_FREE(shdSrc[1]);

	if ((!bcVS.blob) || (!bcPS.blob))
		LOG_ERR_DTDM("PSO RCE fail for pair %llX key %lX alias %llX", pairUID, psoKey, derivedAlias);

	d912pxy_s.vfs.WriteFileH(derivedAlias, bcVS.code, (UINT)bcVS.sz, PXY_VFS_BID_DERIVED_CSO_VS);
	d912pxy_s.vfs.WriteFileH(derivedAlias, bcPS.code, (UINT)bcPS.sz, PXY_VFS_BID_DERIVED_CSO_PS);

	delete replVS;
	delete replPS;

	CreatePSODerived(derivedAlias);
}

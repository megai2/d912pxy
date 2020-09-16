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

d912pxy_dx9_pipeline_state::d912pxy_dx9_pipeline_state()
{
}

d912pxy_dx9_pipeline_state::~d912pxy_dx9_pipeline_state()
{
}

void d912pxy_dx9_pipeline_state::Init()
{
	NonCom_Init(L"dx9_pso");

	cCPSO = NULL;
	cVDecl = NULL;

	psoDesc.val.NumRenderTargets = 1;
	psoDesc.val.rt[0].format = DXGI_FORMAT_B8G8R8A8_UNORM;
	psoDesc.val.ds.format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.val.compareSamplerStage = d912pxy_trimmed_pso_desc::NO_COMPARE_SAMPLERS;
	compareSamplerStage = d912pxy_trimmed_pso_desc::NO_COMPARE_SAMPLERS;

	for (int i = 1 ;i<PXY_INNER_MAX_RENDER_TARGETS;++i)
		psoDesc.val.rt[i].format = DXGI_FORMAT_UNKNOWN;

	for (int i = 0; i != PXY_DX9_MAX_RS_INDEX; ++i)
		DX9RSvalues[i] = PXY_DX9_RS_INITIAL_VALUE;

	//defaut dx9 rs states
	{

		SetDX9RSTracked(D3DRS_ZENABLE, 1);
		SetDX9RSTracked(D3DRS_FILLMODE, D3DFILL_SOLID);
		SetDX9RSTracked(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
		SetDX9RSTracked(D3DRS_ZWRITEENABLE, 1);
		SetDX9RSTracked(D3DRS_ALPHATESTENABLE, 0);
		SetDX9RSTracked(D3DRS_LASTPIXEL, 1);
		SetDX9RSTracked(D3DRS_SRCBLEND, D3DBLEND_ONE);
		SetDX9RSTracked(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		SetDX9RSTracked(D3DRS_CULLMODE, D3DCULL_CCW);
		SetDX9RSTracked(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		SetDX9RSTracked(D3DRS_ALPHAREF, 0);
		SetDX9RSTracked(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);
		SetDX9RSTracked(D3DRS_DITHERENABLE, 0);
		SetDX9RSTracked(D3DRS_ALPHABLENDENABLE, 0);
		SetDX9RSTracked(D3DRS_FOGENABLE, 0);
		SetDX9RSTracked(D3DRS_SPECULARENABLE, 0);
		SetDX9RSTracked(D3DRS_FOGCOLOR, 0);
		SetDX9RSTracked(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
		SetDX9RSTracked(D3DRS_FOGSTART, 0);

		float fValue = 1.0f;
		SetDX9RSTracked(D3DRS_FOGEND, *((DWORD*)(&fValue)));
		SetDX9RSTracked(D3DRS_FOGDENSITY, *((DWORD*)(&fValue)));
		SetDX9RSTracked(D3DRS_RANGEFOGENABLE, 0);
		SetDX9RSTracked(D3DRS_STENCILENABLE, 0);
		SetDX9RSTracked(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
		SetDX9RSTracked(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
		SetDX9RSTracked(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
		SetDX9RSTracked(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
		SetDX9RSTracked(D3DRS_STENCILREF, 0);
		SetDX9RSTracked(D3DRS_STENCILMASK, 0xFFFFFFFF);
		SetDX9RSTracked(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
		SetDX9RSTracked(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
		SetDX9RSTracked(D3DRS_WRAP0, 0);
		SetDX9RSTracked(D3DRS_WRAP1, 0);
		SetDX9RSTracked(D3DRS_WRAP2, 0);
		SetDX9RSTracked(D3DRS_WRAP3, 0);
		SetDX9RSTracked(D3DRS_WRAP4, 0);
		SetDX9RSTracked(D3DRS_WRAP5, 0);
		SetDX9RSTracked(D3DRS_WRAP6, 0);
		SetDX9RSTracked(D3DRS_WRAP7, 0);
		SetDX9RSTracked(D3DRS_WRAP8, 0);
		SetDX9RSTracked(D3DRS_WRAP9, 0);
		SetDX9RSTracked(D3DRS_WRAP10, 0);
		SetDX9RSTracked(D3DRS_WRAP11, 0);
		SetDX9RSTracked(D3DRS_WRAP12, 0);
		SetDX9RSTracked(D3DRS_WRAP13, 0);
		SetDX9RSTracked(D3DRS_WRAP14, 0);
		SetDX9RSTracked(D3DRS_WRAP15, 0);
		SetDX9RSTracked(D3DRS_CLIPPING, 1);
		SetDX9RSTracked(D3DRS_LIGHTING, 1);
		SetDX9RSTracked(D3DRS_AMBIENT, 0);
		SetDX9RSTracked(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
		SetDX9RSTracked(D3DRS_COLORVERTEX, 1);
		SetDX9RSTracked(D3DRS_LOCALVIEWER, 1);
		SetDX9RSTracked(D3DRS_NORMALIZENORMALS, 0);
		SetDX9RSTracked(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
		SetDX9RSTracked(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2);
		SetDX9RSTracked(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
		SetDX9RSTracked(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
		SetDX9RSTracked(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
		SetDX9RSTracked(D3DRS_CLIPPLANEENABLE, 0);
		SetDX9RSTracked(D3DRS_POINTSIZE, 0);
		SetDX9RSTracked(D3DRS_POINTSIZE_MIN, *((DWORD*)(&fValue)));
		SetDX9RSTracked(D3DRS_POINTSPRITEENABLE, 0);
		SetDX9RSTracked(D3DRS_POINTSCALEENABLE, 0);
		SetDX9RSTracked(D3DRS_POINTSCALE_A, *((DWORD*)(&fValue)));
		SetDX9RSTracked(D3DRS_POINTSCALE_B, 0);
		SetDX9RSTracked(D3DRS_POINTSCALE_C, 0);
		SetDX9RSTracked(D3DRS_MULTISAMPLEANTIALIAS, 1);
		SetDX9RSTracked(D3DRS_MULTISAMPLEMASK, 0xFFFFFFFF);
		SetDX9RSTracked(D3DRS_PATCHEDGESTYLE, D3DPATCHEDGE_DISCRETE);
		SetDX9RSTracked(D3DRS_DEBUGMONITORTOKEN, D3DDMT_ENABLE);

		fValue = 64.0f;
		SetDX9RSTracked(D3DRS_POINTSIZE_MAX, *((DWORD*)(&fValue)));
		SetDX9RSTracked(D3DRS_INDEXEDVERTEXBLENDENABLE, 0);
		SetDX9RSTracked(D3DRS_COLORWRITEENABLE, 0xF);
		SetDX9RSTracked(D3DRS_TWEENFACTOR, 0);
		SetDX9RSTracked(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		SetDX9RSTracked(D3DRS_POSITIONDEGREE, D3DDEGREE_CUBIC);
		SetDX9RSTracked(D3DRS_NORMALDEGREE, D3DDEGREE_LINEAR);
		SetDX9RSTracked(D3DRS_SCISSORTESTENABLE, 0);
		SetDX9RSTracked(D3DRS_SLOPESCALEDEPTHBIAS, 0);
		SetDX9RSTracked(D3DRS_ANTIALIASEDLINEENABLE, 0);

		fValue = 1.0f;
		SetDX9RSTracked(D3DRS_MINTESSELLATIONLEVEL, *((DWORD*)(&fValue)));
		SetDX9RSTracked(D3DRS_MAXTESSELLATIONLEVEL, *((DWORD*)(&fValue)));
		SetDX9RSTracked(D3DRS_ADAPTIVETESS_X, 0);
		SetDX9RSTracked(D3DRS_ADAPTIVETESS_Y, 0);
		SetDX9RSTracked(D3DRS_ADAPTIVETESS_Z, *((DWORD*)(&fValue)));
		SetDX9RSTracked(D3DRS_ADAPTIVETESS_W, 0);
		SetDX9RSTracked(D3DRS_ENABLEADAPTIVETESSELLATION, 0);
		SetDX9RSTracked(D3DRS_TWOSIDEDSTENCILMODE, 0);
		SetDX9RSTracked(D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_KEEP);
		SetDX9RSTracked(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP);
		SetDX9RSTracked(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_KEEP);
		SetDX9RSTracked(D3DRS_CCW_STENCILFUNC, D3DCMP_NEVER);
		SetDX9RSTracked(D3DRS_COLORWRITEENABLE1, 0xF);
		SetDX9RSTracked(D3DRS_COLORWRITEENABLE2, 0xF);
		SetDX9RSTracked(D3DRS_COLORWRITEENABLE3, 0xF);
		SetDX9RSTracked(D3DRS_BLENDFACTOR, 0xffffffff);
		SetDX9RSTracked(D3DRS_SRGBWRITEENABLE, 0);
		SetDX9RSTracked(D3DRS_DEPTHBIAS, 0);
		SetDX9RSTracked(D3DRS_SEPARATEALPHABLENDENABLE, 0);
		SetDX9RSTracked(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
		SetDX9RSTracked(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);
		SetDX9RSTracked(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);

	}

	MarkDirty();
}

void d912pxy_dx9_pipeline_state::UnInit()
{
	d912pxy_noncom::UnInit();
}

void d912pxy_dx9_pipeline_state::SetDX9RS(D3DRENDERSTATETYPE State, DWORD Value)
{
	ProcessDX9RSChange(State, Value);

	MarkDirty();
}

void d912pxy_dx9_pipeline_state::SetDX9RSTracked(D3DRENDERSTATETYPE State, DWORD Value)
{
	if (DX9RSvalues[State] != Value)
	{
		SetDX9RS(State, Value);
		DX9RSvalues[State] = Value;
	}
}

void d912pxy_dx9_pipeline_state::VShader(d912pxy_shader* vs)
{
#ifdef _DEBUG
	if (vs)
	{
		LOG_DBG_DTDM("vs = %016llX", vs->GetID());
	}
#endif

	psoDesc.ref.VS = vs;

	MarkDirty();
}

void d912pxy_dx9_pipeline_state::PShader(d912pxy_shader* ps)
{
#ifdef _DEBUG
	if (ps)
	{
		LOG_DBG_DTDM("ps = %016llX", ps->GetID());
	}
#endif

	psoDesc.ref.PS = ps;

	MarkDirty();
}

void d912pxy_dx9_pipeline_state::IAFormat(d912pxy_vdecl* vertexDcl)
{
	cVDecl = vertexDcl;
	psoDesc.ref.InputLayout = vertexDcl;

	MarkDirty();
}

void d912pxy_dx9_pipeline_state::IAFormatInstanced(d912pxy_vdecl* vertexDcl)
{
	psoDesc.ref.InputLayout = vertexDcl;

	MarkDirty();
}

void d912pxy_dx9_pipeline_state::RTVFormat(DXGI_FORMAT fmt, UINT idx)
{
	if (fmt != psoDesc.val.rt[idx].format)
	{		
		psoDesc.val.rt[idx].format = fmt;
		MarkDirty();
	}
}

void d912pxy_dx9_pipeline_state::DSVFormat(DXGI_FORMAT fmt)
{
	if (fmt != psoDesc.val.ds.format)
	{
		psoDesc.val.ds.format = fmt;
		MarkDirty();
	}
}

void d912pxy_dx9_pipeline_state::OMReflect(UINT RTcnt, D3D12_CPU_DESCRIPTOR_HANDLE* dsv)
{
	if (psoDesc.val.NumRenderTargets != RTcnt)
	{
		psoDesc.val.NumRenderTargets = RTcnt;
		MarkDirty();
	}

	if (!dsv)
		DSVFormat(DXGI_FORMAT_UNKNOWN);
}

void d912pxy_dx9_pipeline_state::UpdateCompareSampler(UINT stage, bool use)
{
	if (!use)
	{		
		if (stage == compareSamplerStage)
			compareSamplerStage = d912pxy_trimmed_pso_desc::NO_COMPARE_SAMPLERS;
	}
	else
		compareSamplerStage = stage;
}

d912pxy_vdecl* d912pxy_dx9_pipeline_state::GetIAFormat()
{
	return cVDecl;
}

d912pxy_shader* d912pxy_dx9_pipeline_state::GetPShader()
{
	return psoDesc.ref.PS;
}

d912pxy_shader* d912pxy_dx9_pipeline_state::GetVShader()
{
	return psoDesc.ref.VS;
}

d912pxy_pso_item* d912pxy_dx9_pipeline_state::GetCurrentCPSO()
{
	return cCPSO;
}

d912pxy_trimmed_pso_desc d912pxy_dx9_pipeline_state::GetCurrentDesc()
{
	return psoDesc;
}

void d912pxy_dx9_pipeline_state::SetCurrentDesc(d912pxy_trimmed_pso_desc& dsc)
{
	psoDesc = dsc;
}

DWORD d912pxy_dx9_pipeline_state::GetDX9RsValue(D3DRENDERSTATETYPE State)
{
	switch (State)
	{
	case D3DRS_ALPHABLENDENABLE:
		return psoDesc.val.rt[0].blend.enable;
		break;
	case D3DRS_DESTBLEND:
		return psoDesc.val.rt[0].blend.dest;
		break;
	default:
		return DX9RSvalues[State];
	}

	return 0;
}

void d912pxy_dx9_pipeline_state::Use()
{
	if (compareSamplerStage != psoDesc.val.compareSamplerStage)
	{
		psoDesc.val.compareSamplerStage = compareSamplerStage;
		isDirty = true;
	}

	if (isDirty)
	{
		d912pxy_s.render.replay.DoPSORaw(&psoDesc);

		isDirty = false;
	}
}

void d912pxy_dx9_pipeline_state::UseCompiled(d912pxy_pso_item* it)
{
	cCPSO = it;
	if (cCPSO)
	{
		d912pxy_s.render.replay.DoPSOCompiled(it);

		isDirty = false;
	}
}

void d912pxy_dx9_pipeline_state::UseWithFeedbackPtr(void** feedback)
{
	//megai2: if crashes due to 3rd party mods releasing resources in wrong time will be encountered again
	psoDesc.HoldRefs(true);

	d912pxy_s.render.replay.DoPSORawFeedback(&psoDesc, feedback);

	//megai2: force dirty to reset PSO to current state data 
	MarkDirty();
}

fv4Color d912pxy_dx9_pipeline_state::TransformBlendFactor(DWORD val)
{
	fv4Color ret;

	for (int i = 0; i != 4; ++i)
	{
		ret.val[i] = ((val >> (i << 3)) & 0xFF) / 255.0f;
	}

	return ret;
}

DWORD d912pxy_dx9_pipeline_state::TransformBlend2AlphaBlend(DWORD val)
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

void d912pxy_dx9_pipeline_state::MarkDirty()
{
	isDirty = true;
}

void d912pxy_dx9_pipeline_state::ProcessDX9RSChange(D3DRENDERSTATETYPE State, DWORD Value)
{
	LOG_DBG_DTDM("RS[%u]=%u", State, Value);

	switch (State)
	{
	case D3DRS_BLENDFACTOR:
	{
		DX9RSvalues[State] = Value;
		d912pxy_s.render.replay.DoOMBlendFac(TransformBlendFactor(Value).val);
	}
	break; //193,   /* D3DCOLOR used for a constant blend factor during alpha blending for devices that support D3DPBLENDCAPS_BLENDFACTOR */
	case D3DRS_STENCILREF:
		DX9RSvalues[State] = Value;
		d912pxy_s.render.replay.DoOMStencilRef(Value);
		break; //57,   /* Reference value used in stencil test */
	case D3DRS_SCISSORTESTENABLE:
		DX9RSvalues[State] = Value;
		if (Value)
			d912pxy_s.render.iframe.RestoreScissor();
		else
			d912pxy_s.render.iframe.IgnoreScissor();
		break;
	case D3DRS_ZENABLE:
		psoDesc.val.ds.enable = (Value == D3DZB_TRUE);
		break; //7,    /* D3DZBUFFERTYPE (or TRUE/FALSE for legacy) */

	case D3DRS_FILLMODE:
		if (D3DFILL_SOLID == Value)
			psoDesc.val.rast.fillMode = D3D12_FILL_MODE_SOLID;
		else if (D3DFILL_WIREFRAME == Value)
			psoDesc.val.rast.fillMode = D3D12_FILL_MODE_WIREFRAME;
		else {
			psoDesc.val.rast.fillMode = D3D12_FILL_MODE_WIREFRAME;
			LOG_DBG_DTDM("RS fillmode point unimpl");
		}
		break; //8,    /* D3DFILLMODE */

	case D3DRS_ZWRITEENABLE:
		psoDesc.val.ds.writeMask = Value ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;	
		break; //14,   /* TRUE to enable z writes */

	case D3DRS_SRCBLEND:
		DX9RSvalues[State] = Value;
		psoDesc.val.rt[0].blend.src = (D3D12_BLEND)Value;

		if (!DX9RSvalues[D3DRS_SEPARATEALPHABLENDENABLE])
			psoDesc.val.rt[0].blend.srcAlpha = (D3D12_BLEND)TransformBlend2AlphaBlend(Value);

		break; //19,   /* D3DBLEND */

	case D3DRS_DESTBLEND:
		DX9RSvalues[State] = Value;
		psoDesc.val.rt[0].blend.dest = (D3D12_BLEND)Value;

		if (!DX9RSvalues[D3DRS_SEPARATEALPHABLENDENABLE])
			psoDesc.val.rt[0].blend.destAlpha = (D3D12_BLEND)TransformBlend2AlphaBlend(Value);

		break; //20,   /* D3DBLEND */

	case D3DRS_CULLMODE:
		psoDesc.val.rast.cullMode = (D3D12_CULL_MODE)Value;
		break; //22,   /* D3DCULL */

	case D3DRS_ZFUNC:
		psoDesc.val.ds.func = (D3D12_COMPARISON_FUNC)Value;
		break; //23,   /* D3DCMPFUNC */

	case D3DRS_ALPHABLENDENABLE:
		psoDesc.val.rt[0].blend.enable = (UINT8)Value;
		//FIXME! must set this to all active RT's somewhere
		break; //27,   /* TRUE to enable alpha blending */

	case D3DRS_COLORWRITEENABLE:
	{
		//megai2: zero write RT optimization can break PS depth write, so disable this for now
		//d912pxy_s.render.iframe.OptimizeZeroWriteRT(Value);
		psoDesc.val.rt[State - D3DRS_COLORWRITEENABLE].blend.writeMask = Value & 0xF;
	}
	break; //168,  // per-channel write enable
	case D3DRS_COLORWRITEENABLE1:
	case D3DRS_COLORWRITEENABLE2:
	case D3DRS_COLORWRITEENABLE3:
	{
		psoDesc.val.rt[1 + (State - D3DRS_COLORWRITEENABLE1)].blend.writeMask = Value & 0xF;
	}
	break;

	case D3DRS_BLENDOP:
		DX9RSvalues[State] = Value;
		psoDesc.val.rt[0].blend.op = (D3D12_BLEND_OP)Value;

		if (!DX9RSvalues[D3DRS_SEPARATEALPHABLENDENABLE])
			psoDesc.val.rt[0].blend.opAlpha = (D3D12_BLEND_OP)Value;
		break; //171,   // D3DBLENDOP setting

	case D3DRS_SLOPESCALEDEPTHBIAS:
		psoDesc.val.rast.slopeScaledDepthBias = *(float*)(&Value);
		break; //175,

	case D3DRS_ANTIALIASEDLINEENABLE:
		psoDesc.val.rast.antialiasedLineEnable = (UINT8)Value;
		break; //176,

	//stencil
	case D3DRS_STENCILMASK:
		psoDesc.val.ds.stencilReadMask = Value & 0xFF;
		break; //58,   /* Mask value used in stencil test */

	case D3DRS_STENCILWRITEMASK:
		psoDesc.val.ds.stencilWriteMask = Value & 0xFF;
		break; //59,   /* Write mask applied to values written to stencil buffer */

	case D3DRS_STENCILENABLE:
		psoDesc.val.ds.stencilEnable = (UINT8)Value;
		break; //52,   /* BOOL enable/disable stenciling */

	case D3DRS_TWOSIDEDSTENCILMODE:
	{
		LOG_DBG_DTDM("RS twosided stencil %u / %u", psoDesc.val.ds.stencilEnable, Value);
		DX9RSvalues[State] = Value;
		if (!Value)
		{
			psoDesc.val.ds.backFace.passOp = (UINT8)DX9RSvalues[D3DRS_STENCILPASS];
			psoDesc.val.ds.backFace.failOp = (UINT8)DX9RSvalues[D3DRS_STENCILFAIL];
			psoDesc.val.ds.backFace.depthFailOp = (UINT8)DX9RSvalues[D3DRS_STENCILZFAIL];
			psoDesc.val.ds.backFace.func = (UINT8)DX9RSvalues[D3DRS_STENCILFUNC];
		}
		else {
			psoDesc.val.ds.backFace.failOp = (UINT8)DX9RSvalues[D3DRS_CCW_STENCILFAIL];
			psoDesc.val.ds.backFace.depthFailOp = (UINT8)DX9RSvalues[D3DRS_CCW_STENCILZFAIL];
			psoDesc.val.ds.backFace.passOp = (UINT8)DX9RSvalues[D3DRS_CCW_STENCILPASS];
			psoDesc.val.ds.backFace.func = (UINT8)DX9RSvalues[D3DRS_CCW_STENCILFUNC];
			psoDesc.val.ds.stencilEnable = 1;
		}
	}
	break; //185,   /* BOOL enable/disable 2 sided stenciling */

	case D3DRS_STENCILFAIL:
		psoDesc.val.ds.frontFace.failOp = (D3D12_STENCIL_OP)Value;
		if (!DX9RSvalues[D3DRS_TWOSIDEDSTENCILMODE])
			psoDesc.val.ds.backFace.failOp = (D3D12_STENCIL_OP)Value;
		DX9RSvalues[State] = Value;
		break; //53,   /* D3DSTENCILOP to do if stencil test fails */

	case D3DRS_STENCILZFAIL:
		psoDesc.val.ds.frontFace.depthFailOp = (D3D12_STENCIL_OP)Value;
		if (!DX9RSvalues[D3DRS_TWOSIDEDSTENCILMODE])
			psoDesc.val.ds.backFace.depthFailOp = (D3D12_STENCIL_OP)Value;
		DX9RSvalues[State] = Value;
		break; //54,   /* D3DSTENCILOP to do if stencil test passes and Z test fails */

	case D3DRS_STENCILPASS:
		psoDesc.val.ds.frontFace.passOp = (D3D12_STENCIL_OP)Value;
		if (!DX9RSvalues[D3DRS_TWOSIDEDSTENCILMODE])
			psoDesc.val.ds.backFace.passOp = (D3D12_STENCIL_OP)Value;
		DX9RSvalues[State] = Value;
		break; //55,   /* D3DSTENCILOP to do if both stencil and Z tests pass */

	case D3DRS_STENCILFUNC:
		psoDesc.val.ds.frontFace.func = (D3D12_COMPARISON_FUNC)Value;
		if (!DX9RSvalues[D3DRS_TWOSIDEDSTENCILMODE])
			psoDesc.val.ds.backFace.func = (D3D12_COMPARISON_FUNC)Value;
		DX9RSvalues[State] = Value;
		break; //56,   /* D3DCMPFUNC fn.  Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */

	case D3DRS_CCW_STENCILFAIL:
		if (DX9RSvalues[D3DRS_TWOSIDEDSTENCILMODE])
			psoDesc.val.ds.backFace.failOp = (D3D12_STENCIL_OP)Value;
		DX9RSvalues[State] = Value;
		break; //186,   /* D3DSTENCILOP to do if ccw stencil test fails */

	case D3DRS_CCW_STENCILZFAIL:
		if (DX9RSvalues[D3DRS_TWOSIDEDSTENCILMODE])
			psoDesc.val.ds.backFace.depthFailOp = (D3D12_STENCIL_OP)Value;
		DX9RSvalues[State] = Value;
		break; //187,   /* D3DSTENCILOP to do if ccw stencil test passes and Z test fails */

	case D3DRS_CCW_STENCILPASS:
		if (DX9RSvalues[D3DRS_TWOSIDEDSTENCILMODE])
			psoDesc.val.ds.backFace.passOp = (D3D12_STENCIL_OP)Value;
		DX9RSvalues[State] = Value;
		break; //188,   /* D3DSTENCILOP to do if both ccw stencil and Z tests pass */

	case D3DRS_CCW_STENCILFUNC:
		if (DX9RSvalues[D3DRS_TWOSIDEDSTENCILMODE])
			psoDesc.val.ds.backFace.func = (D3D12_COMPARISON_FUNC)Value;
		DX9RSvalues[State] = Value;
		break; //189,   /* D3DCMPFUNC fn.  ccw Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */

	case D3DRS_SRGBWRITEENABLE:
		DX9RSvalues[State] = Value;
		d912pxy_s.render.state.tex.ModStageBit(31, 13, Value);
		//d912pxy_s.render.iframe.TST()->SetTexStage(29, Value);
		break;

	case D3DRS_DEPTHBIAS:
	{
		int depthMul;
		depthMul = (1 << 23) - 1;

		INT fixVal = (INT)(*(float*)&Value * depthMul);
		psoDesc.val.rast.depthBias = fixVal;
		break; //195,
	}

	case D3DRS_SEPARATEALPHABLENDENABLE:
		//LOG_DBG_DTDM("RS sep ablend %u / %u", cDsc.BlendState.RenderTarget[0].BlendEnable, Value); //megai2: same as with stencil. only one bool for blends, not twho. tricky!
		DX9RSvalues[State] = Value;

		if (Value)
		{
			psoDesc.val.rt[0].blend.srcAlpha = (D3D12_BLEND)DX9RSvalues[D3DRS_SRCBLENDALPHA];
			psoDesc.val.rt[0].blend.destAlpha = (D3D12_BLEND)DX9RSvalues[D3DRS_DESTBLENDALPHA];
			psoDesc.val.rt[0].blend.opAlpha = (D3D12_BLEND)DX9RSvalues[D3DRS_BLENDOPALPHA];
		}
		else {
			psoDesc.val.rt[0].blend.srcAlpha = (D3D12_BLEND)TransformBlend2AlphaBlend(DX9RSvalues[D3DRS_SRCBLEND]);
			psoDesc.val.rt[0].blend.destAlpha = (D3D12_BLEND)TransformBlend2AlphaBlend(DX9RSvalues[D3DRS_DESTBLEND]);
			psoDesc.val.rt[0].blend.opAlpha = (D3D12_BLEND)DX9RSvalues[D3DRS_BLENDOP];
		}

		break; //206,  /* TRUE to enable a separate blending function for the alpha channel */

	case D3DRS_SRCBLENDALPHA:
		DX9RSvalues[State] = Value;

		if (DX9RSvalues[D3DRS_SEPARATEALPHABLENDENABLE])
			psoDesc.val.rt[0].blend.srcAlpha = (D3D12_BLEND)Value;
		break; //207,  /* SRC blend factor for the alpha channel when case D3DRS_SEPARATEDESTALPHAENABLE is TRUE */

	case D3DRS_DESTBLENDALPHA:
		DX9RSvalues[State] = Value;

		if (DX9RSvalues[D3DRS_SEPARATEALPHABLENDENABLE])
			psoDesc.val.rt[0].blend.destAlpha = (D3D12_BLEND)Value;
		break; //208,  /* DST blend factor for the alpha channel when case D3DRS_SEPARATEDESTALPHAENABLE is TRUE */

	case D3DRS_BLENDOPALPHA:
		DX9RSvalues[State] = Value;

		if (DX9RSvalues[D3DRS_SEPARATEALPHABLENDENABLE])
			psoDesc.val.rt[0].blend.opAlpha = (D3D12_BLEND_OP)Value;

		break; //209,  /* Blending operation for the alpha channel when case D3DRS_SEPARATEDESTALPHAENABLE is TRUE */

	case D3DRS_ALPHATESTENABLE:
	case D3DRS_ALPHAREF:
	case D3DRS_ALPHAFUNC:
		DX9RSvalues[State] = Value;
		//TODO: remove hardcoded 31, everywhere, ! 
		d912pxy_s.render.state.tex.ModStageByMask(31, (DX9RSvalues[D3DRS_ALPHATESTENABLE] & 1) | (DX9RSvalues[D3DRS_ALPHAFUNC] << 1) | (DX9RSvalues[D3DRS_ALPHAREF] << 5), 0xFFFFE000);
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

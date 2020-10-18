#include "stdafx.h"

#if _WIN64
	#define SRV_GET_MODE (intptr_t)newTex & PXY_COM_OBJ_SIGNATURE_TEXTURE_RTDS
#else
	#define SRV_GET_MODE 
#endif

d912pxy_texture_state::d912pxy_texture_state() 
{
	memset(DX9SSTValues, 7, sizeof(DWORD)*(D3DSAMP_DMAPOFFSET+1));
}

d912pxy_texture_state::~d912pxy_texture_state()
{

}

void d912pxy_texture_state::Init()
{
	NonCom_Init(L"texture state");

	samplerHeap = d912pxy_s.dev.GetDHeap(PXY_INNER_HEAP_SPL);

	UINT16 defaultMinLOD = (UINT16)d912pxy_s.config.GetValueUI64(PXY_CFG_SAMPLERS_MIN_LOD);

	ZeroMemory(&splDsc, sizeof(D3D12_SAMPLER_DESC));
	splDsc.ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	splDsc.MaxLOD = 1e9;

	ZeroMemory(&current, sizeof(d912pxy_device_texture_state));

	for (int i = 0; i != PXY_INNER_MAX_TEXTURE_STAGES; ++i)
	{
		trimmedSpl[i].Dsc0 = 0x49;
		trimmedSpl[i].Dsc1 = 0x49;
		trimmedSpl[i].MinLOD = (UINT16)defaultMinLOD;
	}

	current.dirty = 0xFFFFFFFFFF;
}

void d912pxy_texture_state::UnInit()
{
	for (auto i = splLookup.begin(); i < splLookup.end(); ++i)
	{
		uint32_t cid = i.value();
		if (cid)
			samplerHeap->FreeSlot(cid - 1);
	}

	d912pxy_noncom::UnInit();
}

void d912pxy_texture_state::SetTextureSrvId(UINT stage, UINT srv)
{
	current.dirty |= 1ULL << (stage >> 2);
	current.texHeapID[stage] = srv;

	LOG_DBG_DTDM("tex[%u] = %u", stage, srv);
}

void d912pxy_texture_state::SetTexture(UINT stage, d912pxy_basetexture* texRef)
{
	current.dirtyTexRefs |= (1ULL << stage);
	current.texRefs[stage] = texRef;

	LOG_DBG_DTDM("texRef[%u] = %p", stage, texRef);
}

void d912pxy_texture_state::ModStageByMask(UINT stage, UINT srv, UINT mask)
{
	current.dirty |= 1ULL << (stage >> 2);
			
	current.texHeapID[stage] = (current.texHeapID[stage] & mask) | srv;

	LOG_DBG_DTDM("tex[%u] = %u", stage, current.texHeapID[stage]);
}

void d912pxy_texture_state::ModStageBit(UINT stage, UINT bit, UINT set)
{
	UINT ov = current.texHeapID[stage];
	UINT val;

	if (set)
		val = (1 << bit) | ov;
	else
		val = (~(1 << bit)) & ov;

	current.dirty |= 1ULL << (stage >> 2);

	current.texHeapID[stage] = val;

	LOG_DBG_DTDM("tex[%u] = %u", stage, val);
}

void d912pxy_texture_state::ModSamplerTracked(UINT stage, D3DSAMPLERSTATETYPE state, DWORD value)
{
	if (DX9SSTValues[state] != value)
	{
		ModSampler(stage, state, value);
		DX9SSTValues[state] = value;
	}
}

void d912pxy_texture_state::ModSampler(UINT stage, D3DSAMPLERSTATETYPE state, DWORD value)
{
	LOG_DBG_DTDM("Sampler[%u][%u] = %u", stage, state, value);

	d912pxy_trimmed_sampler_dsc* cDesc = &trimmedSpl[stage];
	current.dirty |= 1ULL << (stage + 8);		

	switch (state)
	{
	case D3DSAMP_ADDRESSU:		
	case D3DSAMP_ADDRESSV:				
	case D3DSAMP_ADDRESSW:		
		state = (D3DSAMPLERSTATETYPE)((state - D3DSAMP_ADDRESSU)*3);
		cDesc->Dsc1 = (UINT16)((value << state) | (cDesc->Dsc1 & (~(0x7 << state))));
		break;
	case D3DSAMP_MAXMIPLEVEL:		
		cDesc->MinLOD = (UINT16)value;
		break;
	case D3DSAMP_MIPMAPLODBIAS:		
		cDesc->MipLODBias = (INT16)(max(D3D12_MIP_LOD_BIAS_MIN, min(D3D12_MIP_LOD_BIAS_MAX, *((float*)&value))));
		break;
	case D3DSAMP_SRGBTEXTURE:
	    ModStageBit(30, stage, value);
		break;	
	case D3DSAMP_MAGFILTER: //5
	case D3DSAMP_MINFILTER: //6
	case D3DSAMP_MIPFILTER: //7
	{
		state = (D3DSAMPLERSTATETYPE)((state - D3DSAMP_MAGFILTER) * 3);
		UINT16 nFilter = (UINT16)((cDesc->Dsc0 & (~(0x7 << state))) | (value << state));//megai2: ignore dx12 filter type for now and deal with it later		
		cDesc->Dsc0 = nFilter;
		break;
	}
	case D3DSAMP_MAXANISOTROPY:		
		cDesc->Dsc1 = (UINT16)((value << 9) | (cDesc->Dsc1 & 0x1FF));
		break;
	/*case D3DSAMP_BORDERCOLOR:
		cDesc->borderColor = value;
		break;*/
	default:
		;
	}
}

UINT d912pxy_texture_state::Use()
{	
	if (current.dirtyTexRefs)
	{
		UINT64 dMask = current.dirtyTexRefs;
		current.dirtyTexRefs = 0;
		for (int i = 0; i < 32 && dMask; ++i,dMask >>= 1)
		{
			if ((dMask & 1) == 0)
				continue;			

			d912pxy_basetexture* newTex = current.texRefs[i];
			UINT64 srvId = 0;//megai2: make this to avoid memory reading. but we must be assured that mNullTextureSRV is equal to this constant!
			if (newTex)
			{
				srvId = newTex->GetSRVHeapId(SRV_GET_MODE);
				d912pxy_s.render.state.pso.UpdateCompareSampler(i, newTex->UsesCompareFormat());
			}
			else
				d912pxy_s.render.state.pso.UpdateCompareSampler(i, false);

			d912pxy_s.render.state.tex.SetTextureSrvId(i, (UINT32)srvId);
		}
	}

	if (!current.dirty)
		return 0;
	
	UINT64 df = 0;
	UINT64 splDf = current.dirty >> 8ULL;	
	
	int i = 0;

	while (splDf)
	{
		if (splDf & 1)
		{			
			current.splHeapID[i] = LookupSamplerId(i);
			df |= 1ULL << (i >> 2);					
		}		
		++i;
		splDf = splDf >> 1;		
	}

	df = (df << 8) | (current.dirty & 0xFF);

	i = 0;

	while (df)
	{
		if (df & 1)		
			d912pxy_s.render.batch.GPUWrite((void*)((intptr_t)&current.texHeapID[0] + i * 16), 1, i);
		
		++i;
		df = df >> 1;
	}

	current.dirty = df;

	return 0;
}

void d912pxy_texture_state::AddDirtyFlag(DWORD val)
{
	current.dirty |= val;
}

void d912pxy_texture_state::ClearActiveTextures()
{
	//stage 31 is hardcoded atest value, not actual texture stage
	for (int i = 0; i != PXY_INNER_MAX_TEXTURE_STAGES - 1; ++i)
	{
		SetTexture(i, 0);
	}
}

UINT d912pxy_texture_state::LookupSamplerId(const d912pxy_trimmed_sampler_dsc& trimmedDsc)
{
	uint32_t& ret = splLookup[trimmedDsc];

	if (ret != 0)
		return ret - 1;

	UpdateFullSplDsc(trimmedDsc);

	ret = CreateNewSampler();

	return ret;
}

UINT d912pxy_texture_state::LookupSamplerId(UINT stage)
{
	return LookupSamplerId(trimmedSpl[stage]);
}

void d912pxy_texture_state::UpdateFullSplDsc(const d912pxy_trimmed_sampler_dsc& trimmedSpl)
{
	//megai2: handle filter type for real

	UINT16 dx9FilterName = trimmedSpl.Dsc0;

	D3D12_FILTER dx12Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

	if (
		((dx9FilterName & 0x3) == 0x3) |
		((dx9FilterName & 0x18) == 0x18) |
		((dx9FilterName & 0xC0) == 0xC0)   //3 for aniso
		)
	{
		dx12Filter = D3D12_ENCODE_ANISOTROPIC_FILTER(D3D12_FILTER_REDUCTION_TYPE_STANDARD);
	}
	else {
		//0 1 is POINT and LINEAR
		//but in dx9 1 is point and 2 is linear

		UINT minF = ((0x2 & dx9FilterName) != 0);
		UINT magF = ((0x10 & dx9FilterName) != 0);
		UINT mipF = (((0x80 & dx9FilterName) != 0));
		
		//special hack for PCF filter
		if ((0x7 & dx9FilterName) == 0)
		{
			dx12Filter = D3D12_ENCODE_BASIC_FILTER(
				D3D12_FILTER_TYPE_LINEAR,
				D3D12_FILTER_TYPE_LINEAR,
				D3D12_FILTER_TYPE_LINEAR,
				D3D12_FILTER_REDUCTION_TYPE_COMPARISON
			);
		}
		else {

			dx12Filter = D3D12_ENCODE_BASIC_FILTER(
				minF,
				magF,
				mipF,
				D3D12_FILTER_REDUCTION_TYPE_STANDARD
			);
		}
	}

	splDsc.Filter = dx12Filter;

	UINT16 dx9FilterAWA = trimmedSpl.Dsc1;

	splDsc.AddressU = (D3D12_TEXTURE_ADDRESS_MODE)((dx9FilterAWA) & 0x7);
	splDsc.AddressV = (D3D12_TEXTURE_ADDRESS_MODE)((dx9FilterAWA >> 3) & 0x7);
	splDsc.AddressW = (D3D12_TEXTURE_ADDRESS_MODE)((dx9FilterAWA >> 6) & 0x7);
	splDsc.MaxAnisotropy = dx9FilterAWA >> 9;

	/*DWORD Color = trimmedSpl[from].borderColor;

	float bTmp = 0;

	for (int i = 0; i != 4; ++i)
	{
		bTmp = ((Color >> (i << 3)) & 0xFF) / 255.0f;		
		splDsc.BorderColor[i] = bTmp;
	}*/

	splDsc.MipLODBias = trimmedSpl.MipLODBias;
	splDsc.MinLOD = trimmedSpl.MinLOD;
}

UINT d912pxy_texture_state::CreateNewSampler()
{
	LOG_DBG_DTDM("new sampler f %08lX lmi %.2f lma %.2f lbi %.2f U %u V %u W %u A %u B0 %.3f B1 %.3f B2 %.3f B3 %.3f",
		splDsc.Filter, splDsc.MinLOD, splDsc.MaxLOD, splDsc.MipLODBias, splDsc.AddressU, splDsc.AddressV, splDsc.AddressW, splDsc.MaxAnisotropy,
		splDsc.BorderColor[0], splDsc.BorderColor[1], splDsc.BorderColor[2], splDsc.BorderColor[3]);

	return samplerHeap->CreateSampler(&splDsc)+1;
}

#include "stdafx.h"

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_DEVICE_TEXSTATE

HRESULT WINAPI d912pxy_device::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture)
{
	API_OVERHEAD_TRACK_START(0)

	Stage = (Stage & 0xF) + 16 * ((Stage >> 4) != 0);

	UINT64 srvId = 0;//megai2: make this to avoid memory reading. but we must be assured that mNullTextureSRV is equal to this constant!

	if (pTexture)
	{
		srvId = *(UINT64*)((intptr_t)pTexture - 0x8);
		if (srvId & 0x100000000)
		{
			srvId = pTexture->GetPriority();
		}
	}
		
	mTextureState.dirty |= (1 << (Stage >> 2));
	mTextureState.texHeapID[Stage] = (UINT32)srvId;

#ifdef TRACK_SHADER_BUGS_PROFILE
	if (pTexture)
	{
		d912pxy_basetexture* btex = dynamic_cast<d912pxy_basetexture*>(pTexture);

		stageFormatsTrack[Stage] = btex->GetBaseSurface()->GetDX9DescAtLevel(0).Format;
	}
	else
		stageFormatsTrack[Stage] = D3DFMT_UNKNOWN;
#endif

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{ 
	API_OVERHEAD_TRACK_START(0)

	LOG_DBG_DTDM("Sampler[%u][%u] = %u", Sampler, Type, Value);

	d912pxy_s(samplerState)->ModSampler(Sampler, Type, Value);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
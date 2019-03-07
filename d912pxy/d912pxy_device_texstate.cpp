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

	d912pxy_s(textureState)->SetTexture(Stage, (UINT32)srvId);

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

	Sampler = (Sampler & 0xF) + 16 * (Sampler >= D3DDMAPSAMPLER);

	d912pxy_s(textureState)->ModSampler(Sampler, Type, Value);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
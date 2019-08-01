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

HRESULT d912pxy_device::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture)
{
	Stage = (Stage & 0xF) + 16 * ((Stage >> 4) != 0);

	UINT64 srvId = 0;//megai2: make this to avoid memory reading. but we must be assured that mNullTextureSRV is equal to this constant!

	if (pTexture)
	{
		srvId = PXY_COM_LOOKUP_(pTexture, basetex).GetSRVHeapId((intptr_t)pTexture & PXY_COM_OBJ_SIGNATURE_TEXTURE_RTDS);
	}

	d912pxy_s.render.tex.SetTexture(Stage, (UINT32)srvId);

	return D3D_OK; 
}

HRESULT d912pxy_device::SetTexture_PS(DWORD Stage, IDirect3DBaseTexture9* pTexture)
{
	Stage = (Stage & 0xF) + 16 * ((Stage >> 4) != 0);

	UINT64 srvId = 0;//megai2: make this to avoid memory reading. but we must be assured that mNullTextureSRV is equal to this constant!

	if (pTexture)
	{
		srvId = (PXY_COM_LOOKUP(pTexture, basetex))->GetSRVHeapId((intptr_t)pTexture & PXY_COM_OBJ_SIGNATURE_TEXTURE_RTDS);
	}

	d912pxy_s.render.tex.SetTexture(Stage, (UINT32)srvId);

	if (pTexture)
	{
		d912pxy_basetexture* btex = PXY_COM_LOOKUP(pTexture, basetex);

		stageFormatsTrack[Stage] = btex->GetBaseSurface()->GetDX9DescAtLevel(0).Format;
	}
	else
		stageFormatsTrack[Stage] = D3DFMT_UNKNOWN;

	return D3D_OK;
}

HRESULT d912pxy_device::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{ 	
	Sampler = (Sampler & 0xF) + 16 * (Sampler >= D3DDMAPSAMPLER);

	d912pxy_s.render.tex.ModSampler(Sampler, Type, Value);
	
	return D3D_OK;
}

HRESULT d912pxy_device::SetSamplerState_Tracked(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	Sampler = (Sampler & 0xF) + 16 * (Sampler >= D3DDMAPSAMPLER);

	d912pxy_s.render.tex.ModSampler(Sampler, Type, Value);

	return D3D_OK;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
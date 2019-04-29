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

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_DEVICE_SURFACE

HRESULT WINAPI d912pxy_device::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)
	
	if (RenderTargetIndex >= PXY_INNER_MAX_RENDER_TARGETS)
		return D3DERR_INVALIDCALL;

	d912pxy_surface* rtSurf = (d912pxy_surface*)pRenderTarget;

	d912pxy_s(iframe)->BindSurface(1 + RenderTargetIndex, rtSurf);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppRenderTarget = d912pxy_s(iframe)->GetBindedSurface(RenderTargetIndex + 1);
	(*ppRenderTarget)->AddRef();

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{ 
	LOG_DBG_DTDM("depth surface set to %016llX", pNewZStencil);
	
	API_OVERHEAD_TRACK_START(0)

	d912pxy_s(iframe)->BindSurface(0, (d912pxy_surface*)pNewZStencil);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppZStencilSurface = d912pxy_s(iframe)->GetBindedSurface(0);
	(*ppZStencilSurface)->AddRef();	

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	d912pxy_surface* src = (d912pxy_surface*)pRenderTarget;
	d912pxy_surface* dst = (d912pxy_surface*)pDestSurface;
	src->BCopyTo(dst, 3, d912pxy_s(GPUcl)->GID(CLG_SEQ));

	dst->CopySurfaceDataToCPU();

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
	API_OVERHEAD_TRACK_START(0)

	d912pxy_s(CMDReplay)->StretchRect((d912pxy_surface*)pSourceSurface, (d912pxy_surface*)pDestSurface);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	LOG_DBG_DTDM("Clear Rects: %u", Count);

	API_OVERHEAD_TRACK_START(0)

	if (Flags & D3DCLEAR_TARGET)
	{
		float fvColor[4] =
		{
			((Color >> 24) & 0xFF) / 255.0f,
			((Color >> 0) & 0xFF) / 255.0f,
			((Color >> 8) & 0xFF) / 255.0f,
			((Color >> 16) & 0xFF) / 255.0f
		};

		d912pxy_surface* surf = d912pxy_s(iframe)->GetBindedSurface(1);

		if (surf)
			d912pxy_s(CMDReplay)->RTClear(surf, fvColor, d912pxy_s(iframe)->GetViewport());
		//iframe->GetBindedSurface(1)->d912_rtv_clear(fvColor, Count, (D3D12_RECT*)pRects);//megai2: rect is 4 uint structure, may comply
	}

	if (Flags & (D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER))
	{
		DWORD cvtCf = ((D3D12_CLEAR_FLAG_DEPTH * ((Flags & D3DCLEAR_ZBUFFER) != 0)) | (D3D12_CLEAR_FLAG_STENCIL * ((Flags & D3DCLEAR_STENCIL) != 0)));

		d912pxy_surface* surf = d912pxy_s(iframe)->GetBindedSurface(0);

		if (surf)
			d912pxy_s(CMDReplay)->DSClear(surf, Z, Stencil & 0xFF, (D3D12_CLEAR_FLAGS)cvtCf, d912pxy_s(iframe)->GetViewport());

		//	surf->d912_dsv_clear(Z, Stencil & 0xFF, Count, (D3D12_RECT*)pRects, (D3D12_CLEAR_FLAGS)cvtCf);
	}

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
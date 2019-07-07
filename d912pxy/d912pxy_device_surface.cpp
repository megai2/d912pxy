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

HRESULT d912pxy_device::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)
	
	if (RenderTargetIndex >= PXY_INNER_MAX_RENDER_TARGETS)
		return D3DERR_INVALIDCALL;

	d912pxy_surface* rtSurf = PXY_COM_LOOKUP(pRenderTarget, surface);

	d912pxy_s(iframe)->BindSurface(1 + RenderTargetIndex, rtSurf);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT d912pxy_device::SetRenderTarget_Compat(DWORD RenderTargetIndex, IDirect3DSurface9 * pRenderTarget)
{
	API_OVERHEAD_TRACK_START(0)

	if (RenderTargetIndex >= PXY_INNER_MAX_RENDER_TARGETS)
		return D3DERR_INVALIDCALL;

	d912pxy_surface* rtSurf = PXY_COM_LOOKUP(pRenderTarget, surface);

	d912pxy_s(iframe)->BindSurface(1 + RenderTargetIndex, rtSurf);

	D3DVIEWPORT9 wp;
	wp.X = 0;
	wp.Y = 0;

	D3DSURFACE_DESC sdsc = rtSurf->GetDX9DescAtLevel(0);

	wp.Width = sdsc.Width;
	wp.Height = sdsc.Height;
	wp.MaxZ = 1;
	wp.MinZ = 0;

	SetViewport(&wp);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT d912pxy_device::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppRenderTarget = PXY_COM_CAST_(IDirect3DSurface9, d912pxy_s(iframe)->GetBindedSurface(RenderTargetIndex + 1));
	(*ppRenderTarget)->AddRef();

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT d912pxy_device::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{ 
	LOG_DBG_DTDM("depth surface set to %016llX", pNewZStencil);
	
	API_OVERHEAD_TRACK_START(0)

	if (pNewZStencil)
		d912pxy_s(iframe)->BindSurface(0, PXY_COM_LOOKUP(pNewZStencil, surface));
	else 
		d912pxy_s(iframe)->BindSurface(0, 0);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT d912pxy_device::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppZStencilSurface = PXY_COM_CAST_(IDirect3DSurface9, d912pxy_s(iframe)->GetBindedSurface(0));
	(*ppZStencilSurface)->AddRef();	

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT d912pxy_device::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	d912pxy_surface* src = PXY_COM_LOOKUP(pRenderTarget, surface);
	d912pxy_surface* dst = PXY_COM_LOOKUP(pDestSurface, surface);
	src->BCopyTo(dst, 3, d912pxy_s(GPUcl)->GID(CLG_SEQ));

	dst->CopySurfaceDataToCPU();

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT d912pxy_device::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
	API_OVERHEAD_TRACK_START(0)

	d912pxy_s(CMDReplay)->StretchRect(PXY_COM_LOOKUP(pSourceSurface, surface), PXY_COM_LOOKUP(pDestSurface, surface));

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT d912pxy_device::Clear_Emulated(DWORD Count, const D3DRECT * pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	API_OVERHEAD_TRACK_START(0)

	m_clearEmul->Clear(Count, pRects, Flags, Color, Z, Stencil);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT d912pxy_device::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
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
	}

	if (Flags & (D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER))
	{
		DWORD cvtCf = ((D3D12_CLEAR_FLAG_DEPTH * ((Flags & D3DCLEAR_ZBUFFER) != 0)) | (D3D12_CLEAR_FLAG_STENCIL * ((Flags & D3DCLEAR_STENCIL) != 0)));

		d912pxy_surface* surf = d912pxy_s(iframe)->GetBindedSurface(0);

		if (surf)
			d912pxy_s(CMDReplay)->DSClear(surf, Z, Stencil & 0xFF, (D3D12_CLEAR_FLAGS)cvtCf, d912pxy_s(iframe)->GetViewport());
	}

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
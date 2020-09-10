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
	if (RenderTargetIndex >= PXY_INNER_MAX_RENDER_TARGETS)
		return D3DERR_INVALIDCALL;

	d912pxy_surface* rtSurf = PXY_COM_LOOKUP(pRenderTarget, surface);

	d912pxy_s.render.iframe.BindSurface(1 + RenderTargetIndex, rtSurf);

	return D3D_OK; 
}

HRESULT d912pxy_device::SetRenderTarget_Compat(DWORD RenderTargetIndex, IDirect3DSurface9 * pRenderTarget)
{
	if (RenderTargetIndex >= PXY_INNER_MAX_RENDER_TARGETS)
		return D3DERR_INVALIDCALL;

	d912pxy_surface* rtSurf = PXY_COM_LOOKUP(pRenderTarget, surface);

	d912pxy_s.render.iframe.BindSurface(1 + RenderTargetIndex, rtSurf);

	//don't update viewport when setting extra RTs
	if (RenderTargetIndex == 0)
	{
		D3DVIEWPORT9 wp;
		wp.X = 0;
		wp.Y = 0;

		D3DSURFACE_DESC sdsc = rtSurf->GetDX9DescAtLevel(0);

		wp.Width = sdsc.Width;
		wp.Height = sdsc.Height;
		wp.MaxZ = 1;
		wp.MinZ = 0;

		SetViewport(&wp);
	}

	return D3D_OK;
}

HRESULT d912pxy_device::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{ 
	*ppRenderTarget = PXY_COM_CAST_(IDirect3DSurface9, d912pxy_s.render.iframe.GetBindedSurface(RenderTargetIndex + 1));
	(*ppRenderTarget)->AddRef();

	return D3D_OK;
}

HRESULT d912pxy_device::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{ 
	d912pxy_surface* newDS = PXY_COM_LOOKUP(pNewZStencil, surface);

	//megai2: remove this filter when GW2 is fixed
	if (newDS)
	{
		if (newDS->GetDX9DescAtLevel(0).Usage != D3DUSAGE_DEPTHSTENCIL)
			return D3DERR_INVALIDCALL;
	}

	d912pxy_s.render.iframe.BindSurface(0, newDS);

	return D3D_OK; 
}

HRESULT d912pxy_device::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{ 
	*ppZStencilSurface = PXY_COM_CAST_(IDirect3DSurface9, d912pxy_s.render.iframe.GetBindedSurface(0));
	(*ppZStencilSurface)->AddRef();	

	return D3D_OK; 
}

HRESULT d912pxy_device::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{	
	d912pxy_surface* src = PXY_COM_LOOKUP(pRenderTarget, surface);
	d912pxy_surface* dst = d912pxy_surface::CorrectLayerRepresent(PXY_COM_CAST(d912pxy_com_object, pDestSurface));
	   
	d912pxy_s.render.replay.DoStretchRect(src, dst);
	
	dst->CopySurfaceDataToCPU();

	return D3D_OK;
}

HRESULT d912pxy_device::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{	
	d912pxy_surface* sSrc = d912pxy_surface::CorrectLayerRepresent(PXY_COM_CAST(d912pxy_com_object, pSourceSurface));
	d912pxy_surface* sDst = d912pxy_surface::CorrectLayerRepresent(PXY_COM_CAST(d912pxy_com_object, pDestSurface));

	auto dSrc = sSrc->GetDX9DescAtLevel(0);
	auto dDst = sDst->GetDX9DescAtLevel(0);

	if ((dSrc.Height == dDst.Height) &&
		(dSrc.Width == dDst.Width) &&
		(dSrc.Format == dDst.Format))		
		d912pxy_s.render.replay.DoStretchRect(sSrc, sDst);
	else 
		m_emulatedSurfaceOps->StretchRect(sSrc, sDst);		

	return D3D_OK;
}

HRESULT d912pxy_device::Clear_Emulated(DWORD Count, const D3DRECT * pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	m_emulatedSurfaceOps->Clear(Count, pRects, Flags, Color, Z, Stencil);
	
	return D3D_OK;
}

HRESULT d912pxy_device::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{	
	if (Flags & D3DCLEAR_TARGET)
	{
		float fvColor[4] =
		{
			((Color >> 24) & 0xFF) / 255.0f,
			((Color >> 0) & 0xFF) / 255.0f,
			((Color >> 8) & 0xFF) / 255.0f,
			((Color >> 16) & 0xFF) / 255.0f
		};

		d912pxy_surface* surf = d912pxy_s.render.iframe.GetBindedSurface(1);

		if (surf)
			d912pxy_s.render.replay.DoRTClear(surf, fvColor, d912pxy_s.render.iframe.GetViewport());
	}

	if (Flags & (D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER))
	{
		DWORD cvtCf = ((D3D12_CLEAR_FLAG_DEPTH * ((Flags & D3DCLEAR_ZBUFFER) != 0)) | (D3D12_CLEAR_FLAG_STENCIL * ((Flags & D3DCLEAR_STENCIL) != 0)));

		d912pxy_surface* surf = d912pxy_s.render.iframe.GetBindedSurface(0);
		if (surf)
			d912pxy_s.render.replay.DoDSClear(surf, Z, Stencil & 0xFF, (D3D12_CLEAR_FLAGS)cvtCf, d912pxy_s.render.iframe.GetViewport());
	}

	return D3D_OK;
}

HRESULT d912pxy_device::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint)
{ 
	if (pSourceRect || pDestPoint)
		return D3D_OK;

	d912pxy_surface* src = d912pxy_surface::CorrectLayerRepresent(PXY_COM_CAST(d912pxy_com_object, pSourceSurface));
	d912pxy_surface* dst = d912pxy_surface::CorrectLayerRepresent(PXY_COM_CAST(d912pxy_com_object, pDestinationSurface));

	d912pxy_s.render.replay.DoStretchRect(src, dst);

	return D3D_OK;
}

HRESULT d912pxy_device::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) 
{ 
	d912pxy_surface* src = PXY_COM_CAST(d912pxy_com_object, pSourceTexture)->basetex.GetBaseSurface();
	d912pxy_surface* dst = PXY_COM_CAST(d912pxy_com_object, pDestinationTexture)->basetex.GetBaseSurface();

	d912pxy_s.render.replay.DoStretchRect(src, dst);

	return D3D_OK;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
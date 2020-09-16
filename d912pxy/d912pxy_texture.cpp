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

#define D912PXY_METHOD_IMPL_CN d912pxy_texture
#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_TEXTURE

d912pxy_texture::d912pxy_texture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format) : d912pxy_basetexture()
{
	m_levels = Levels;
	
	LOG_DBG_DTDM("tex usage is %u", Usage);

	attachedCache.shouldBarrier = (Usage == D3DUSAGE_RENDERTARGET) | (Usage == D3DUSAGE_DEPTHSTENCIL);

	if (m_levels != 0)
		baseSurface = d912pxy_s.pool.surface.GetSurface(Width, Height, Format, m_levels, 1, Usage, &attachedCache.srvId);
	else 
		baseSurface = d912pxy_surface::d912pxy_surface_com(Width, Height, Format, Usage, D3DMULTISAMPLE_NONE,0,	0, &m_levels, 1, &attachedCache.srvId);

	if (!attachedCache.shouldBarrier)
		attachedCache.srvId = baseSurface->GetSRVHeapId();

	attachedCache.compareFormat = (Format == D3DFMT_D24X8) || (Format == D3DFMT_D24S8);
}


d912pxy_texture * d912pxy_texture::d912pxy_texture_com(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format)
{
	d912pxy_com_object* ret = d912pxy_s.com.AllocateComObj(((Usage == D3DUSAGE_RENDERTARGET) | (Usage == D3DUSAGE_DEPTHSTENCIL)) ? PXY_COM_OBJ_TEXTURE_RTDS : PXY_COM_OBJ_TEXTURE);
	
	new (&ret->tex_2d)d912pxy_texture(Width, Height, Levels, Usage, Format);
	ret->vtable = d912pxy_com_route_get_vtable(PXY_COM_ROUTE_TEXTURE_2D);

	return &ret->tex_2d;
}

d912pxy_texture::~d912pxy_texture()
{	
}

#undef D912PXY_METHOD_IMPL_CN

HRESULT d912pxy_texture::GetLevelDesc(UINT Level, D3DSURFACE_DESC * pDesc)
{
	*pDesc = baseSurface->GetDX9DescAtLevel(Level);
	
	return D3D_OK;
}

HRESULT d912pxy_texture::GetSurfaceLevel(UINT Level, IDirect3DSurface9 ** ppSurfaceLevel)
{
	if (attachedCache.shouldBarrier)
		*ppSurfaceLevel = PXY_COM_CAST_(IDirect3DSurface9, baseSurface);
	else
		*ppSurfaceLevel = PXY_COM_CAST_(IDirect3DSurface9, baseSurface->GetLayer(Level, 0));

	(*ppSurfaceLevel)->AddRef();

	return D3D_OK;
}

HRESULT d912pxy_texture::LockRect(UINT Level, D3DLOCKED_RECT * pLockedRect, const RECT * pRect, DWORD Flags)
{
	LOG_DBG_DTDM("LockRect lv %u", Level);

	HRESULT ret = baseSurface->GetLayer(Level, 0)->LockRect(pLockedRect, pRect, Flags);

	return ret;
}

HRESULT d912pxy_texture::UnlockRect(UINT Level)
{
	HRESULT ret = baseSurface->GetLayer(Level, 0)->UnlockRect();

	return ret;
}

HRESULT d912pxy_texture::AddDirtyRect(const RECT * pDirtyRect)
{
	//if (m_levels == 1)
		//baseSurface->GetLayer(0, 0)->SetDirtyRect(pDirtyRect->left, pDirtyRect->right, pDirtyRect->top, pDirtyRect->bottom);


	return D3D_OK;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
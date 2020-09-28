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

d912pxy_vtexture* d912pxy_vtexture::d912pxy_vtexture_com(UINT Width, UINT Height, UINT depth, UINT Levels, DWORD Usage, D3DFORMAT Format)
{
	d912pxy_com_object* ret = d912pxy_s.com.AllocateComObj(((Usage == D3DUSAGE_RENDERTARGET) | (Usage == D3DUSAGE_DEPTHSTENCIL)) ? PXY_COM_OBJ_TEXTURE_RTDS : PXY_COM_OBJ_TEXTURE);

	new (&ret->tex_3d)d912pxy_vtexture(Width, Height, depth, Levels, Usage, Format);
	ret->vtable = d912pxy_com_route_get_vtable(PXY_COM_ROUTE_TEXTURE_3D);

	return &ret->tex_3d;
}

d912pxy_vtexture::~d912pxy_vtexture()
{

}

d912pxy_vtexture::d912pxy_vtexture(UINT Width, UINT Height, UINT depth, UINT Levels, DWORD Usage, D3DFORMAT Format) : d912pxy_basetexture()
{
	m_levels = Levels;

	LOG_DBG_DTDM("tex3d usage is %u", Usage);

	attachedCache.shouldBarrier = (Usage == D3DUSAGE_RENDERTARGET) | (Usage == D3DUSAGE_DEPTHSTENCIL);

	if (m_levels != 0)
		baseSurface = d912pxy_s.pool.surface.GetSurface(Width, Height, Format, m_levels, depth, Usage, &attachedCache.srvId);
	else
		baseSurface = d912pxy_surface::d912pxy_surface_com(Width, Height, Format, Usage, D3DMULTISAMPLE_NONE, 0, 0, &m_levels, depth, &attachedCache.srvId);

	if (!attachedCache.shouldBarrier)
		attachedCache.srvId = baseSurface->GetSRVHeapId();

	attachedCache.compareFormat = (Format == D3DFMT_D24X8) || (Format == D3DFMT_D24S8);
}

HRESULT d912pxy_vtexture::GetLevelDesc(UINT Level, D3DVOLUME_DESC* pDesc)
{	
	return D3DERR_INVALIDCALL;
}

HRESULT d912pxy_vtexture::GetVolumeLevel(UINT Level, IDirect3DVolume9** ppVolumeLevel)
{
	return D3DERR_INVALIDCALL;
}

HRESULT d912pxy_vtexture::LockBox(UINT Level, D3DLOCKED_BOX* pLockedVolume, CONST D3DBOX* pBox, DWORD Flags)
{
	return D3DERR_INVALIDCALL;
}

HRESULT d912pxy_vtexture::UnlockBox(UINT Level)
{
	return D3DERR_INVALIDCALL;
}

HRESULT d912pxy_vtexture::AddDirtyBox(CONST D3DBOX* pDirtyBox)
{
	return D3DERR_INVALIDCALL;
}
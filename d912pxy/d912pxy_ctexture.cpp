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

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_TEXTURE

d912pxy_ctexture::d912pxy_ctexture(UINT edgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format) : d912pxy_basetexture()
{
	m_levels = Levels;

	LOG_DBG_DTDM("ctexture w:%u", edgeLength);

	if (Usage & D3DUSAGE_RENDERTARGET)
		LOG_ERR_THROW2(-1, "cubemap rt");// baseSurface = new d912pxy_surface(dev, edgeLength, edgeLength, Format, D3DMULTISAMPLE_NONE, 0, 0, 0);
	else if (Usage & D3DUSAGE_DEPTHSTENCIL)
		LOG_ERR_THROW2(-1, "cubemap ds");// baseSurface = new d912pxy_surface(dev, edgeLength, edgeLength, Format, D3DMULTISAMPLE_NONE, 0, 0, 1);
	else if (m_levels != 0)
	{
		baseSurface = d912pxy_s.pool.surface.GetSurface(edgeLength, edgeLength, Format, m_levels, 6, 0, &attachedCache.srvId);
	} else 
		baseSurface = d912pxy_surface::d912pxy_surface_com(edgeLength, edgeLength, Format, Usage, D3DMULTISAMPLE_NONE, 0, 0, &m_levels, 6, &attachedCache.srvId);

	for (int i = 0; i != 6; ++i)
	{		
		faceSurfaces[i] = baseSurface;
	}

	attachedCache.srvId = baseSurface->GetSRVHeapId();
	attachedCache.extraData = 0;
}


d912pxy_ctexture * d912pxy_ctexture::d912pxy_ctexture_com(UINT edgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format)
{
	d912pxy_com_object* ret = d912pxy_s.com.AllocateComObj(PXY_COM_OBJ_TEXTURE);
	
	new (&ret->tex_cube)d912pxy_ctexture(edgeLength, Levels, Usage, Format);
	ret->vtable = d912pxy_com_route_get_vtable(PXY_COM_ROUTE_TEXTURE_CUBE);

	return &ret->tex_cube;
}

d912pxy_ctexture::~d912pxy_ctexture()
{
}

#define D912PXY_METHOD_IMPL_CN d912pxy_ctexture

D912PXY_METHOD_IMPL_NC(GetLevelDesc)(THIS_ UINT Level, D3DSURFACE_DESC *pDesc)
{ 
	*pDesc = baseSurface->GetDX9DescAtLevel(Level);
	
	return D3D_OK;
}

D912PXY_METHOD_IMPL_NC(GetCubeMapSurface)(THIS_ D3DCUBEMAP_FACES FaceType, UINT Level, IDirect3DSurface9** ppCubeMapSurface)
{ 
	*ppCubeMapSurface = PXY_COM_CAST_(IDirect3DSurface9, baseSurface->GetLayer(Level, FaceType));

	(*ppCubeMapSurface)->AddRef();

	return D3D_OK; 
}

D912PXY_METHOD_IMPL_NC(LockRect)(THIS_ D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{ 
	LOG_DBG_DTDM("LockRect lv %u fc %u", Level, FaceType);

	baseSurface->GetLayer(Level, FaceType)->LockRect(pLockedRect, pRect, Flags);

	return D3D_OK; 
}

D912PXY_METHOD_IMPL_NC(UnlockRect)(THIS_ D3DCUBEMAP_FACES FaceType, UINT Level)
{ 
	baseSurface->GetLayer(Level, FaceType)->UnlockRect();

	return D3D_OK; 
}

D912PXY_METHOD_IMPL_NC(AddDirtyRect)(THIS_ D3DCUBEMAP_FACES FaceType, CONST RECT* pDirtyRect)
{ 
//	faceSurfaces[FaceType]->SetDirtyRect(pDirtyRect->left, pDirtyRect->right, pDirtyRect->top, pDirtyRect->bottom);
	return D3D_OK; 
}

#undef D912PXY_METHOD_IMPL_CN

UINT d912pxy_ctexture::GetSRVHeapId()
{
	return faceSurfaces[0]->GetSRVHeapId();
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
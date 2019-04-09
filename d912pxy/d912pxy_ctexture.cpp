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

d912pxy_ctexture::d912pxy_ctexture(d912pxy_device* dev, UINT edgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format) : d912pxy_basetexture(dev)
{
	srvIDc = (UINT32*)((intptr_t)this - 8);

	m_levels = Levels;

	LOG_DBG_DTDM("ctexture w:%u", edgeLength);

	if (Usage & D3DUSAGE_RENDERTARGET)
		LOG_ERR_THROW2(-1, "cubemap rt");// baseSurface = new d912pxy_surface(dev, edgeLength, edgeLength, Format, D3DMULTISAMPLE_NONE, 0, 0, 0);
	else if (Usage & D3DUSAGE_DEPTHSTENCIL)
		LOG_ERR_THROW2(-1, "cubemap ds");// baseSurface = new d912pxy_surface(dev, edgeLength, edgeLength, Format, D3DMULTISAMPLE_NONE, 0, 0, 1);
	else if (m_levels != 0)
	{
		baseSurface = d912pxy_s(pool_surface)->GetSurface(edgeLength, edgeLength, Format, m_levels, 6);
	} else 
		baseSurface = new d912pxy_surface(dev, edgeLength, edgeLength, Format, Usage, &m_levels, 6);

	for (int i = 0; i != 6; ++i)
	{		
		faceSurfaces[i] = baseSurface;
	}

	srvIDc[1] = 0;
	srvIDc[0] = baseSurface->GetSRVHeapId();
}


d912pxy_ctexture::~d912pxy_ctexture()
{
	baseSurface->Release();
}

#define D912PXY_METHOD_IMPL_CN d912pxy_ctexture

D912PXY_IUNK_IMPL

/*** IDirect3DBaseTexture9 METHOD_IMPLs ***/
/*** IDirect3DResource9 methods ***/
D912PXY_METHOD_IMPL(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) { return d912pxy_resource::GetDevice(ppDevice); }
D912PXY_METHOD_IMPL(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags) { return d912pxy_resource::SetPrivateData(refguid, pData, SizeOfData, Flags); }
D912PXY_METHOD_IMPL(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData) { return d912pxy_resource::GetPrivateData(refguid, pData, pSizeOfData); }
D912PXY_METHOD_IMPL(FreePrivateData)(THIS_ REFGUID refguid) { return d912pxy_resource::FreePrivateData(refguid); }
D912PXY_METHOD_IMPL_(DWORD, SetPriority)(THIS_ DWORD PriorityNew) { return d912pxy_resource::SetPriority(PriorityNew); }
D912PXY_METHOD_IMPL_(DWORD, GetPriority)(THIS) { return d912pxy_basetexture::GetPriority(); }
D912PXY_METHOD_IMPL_(void, PreLoad)(THIS) { d912pxy_resource::PreLoad(); }
D912PXY_METHOD_IMPL_(D3DRESOURCETYPE, GetType)(THIS) { return d912pxy_resource::GetType(); }

D912PXY_METHOD_IMPL_(DWORD, SetLOD)(THIS_ DWORD LODNew) { return d912pxy_basetexture::SetLOD(LODNew); }
D912PXY_METHOD_IMPL_(DWORD, GetLOD)(THIS) { return d912pxy_basetexture::GetLOD(); }
D912PXY_METHOD_IMPL_(DWORD, GetLevelCount)(THIS) { return d912pxy_basetexture::GetLevelCount(); }
D912PXY_METHOD_IMPL(SetAutoGenFilterType)(THIS_ D3DTEXTUREFILTERTYPE FilterType) { return d912pxy_basetexture::SetAutoGenFilterType(FilterType); }
D912PXY_METHOD_IMPL_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)(THIS) { return d912pxy_basetexture::GetAutoGenFilterType(); }
D912PXY_METHOD_IMPL_(void, GenerateMipSubLevels)(THIS) { d912pxy_basetexture::GenerateMipSubLevels(); }

D912PXY_METHOD_IMPL(GetLevelDesc)(THIS_ UINT Level, D3DSURFACE_DESC *pDesc)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(1)

	*pDesc = baseSurface->GetDX9DescAtLevel(Level);

	API_OVERHEAD_TRACK_END(1)

	return D3D_OK;
}

D912PXY_METHOD_IMPL(GetCubeMapSurface)(THIS_ D3DCUBEMAP_FACES FaceType, UINT Level, IDirect3DSurface9** ppCubeMapSurface)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(1)

	*ppCubeMapSurface = (IDirect3DSurface9*)baseSurface->GetLayer(Level, FaceType);
	(*ppCubeMapSurface)->AddRef();


	API_OVERHEAD_TRACK_END(1)

	return D3D_OK; 
}

D912PXY_METHOD_IMPL(LockRect)(THIS_ D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{ 
	LOG_DBG_DTDM("LockRect lv %u fc %u", Level, FaceType);

	baseSurface->GetLayer(Level, FaceType)->LockRect(pLockedRect, pRect, Flags);

	return D3D_OK; 
}

D912PXY_METHOD_IMPL(UnlockRect)(THIS_ D3DCUBEMAP_FACES FaceType, UINT Level)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	baseSurface->GetLayer(Level, FaceType)->UnlockRect();

	return D3D_OK; 
}

D912PXY_METHOD_IMPL(AddDirtyRect)(THIS_ D3DCUBEMAP_FACES FaceType, CONST RECT* pDirtyRect)
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
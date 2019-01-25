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

d912pxy_texture::d912pxy_texture(d912pxy_device* dev, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format) : d912pxy_basetexture(dev)
{
	srvIDc = (UINT32*)((intptr_t)this - 8);

	m_levels = Levels;

	UINT32 rtTrans;

	LOG_DBG_DTDM("tex usage is %u", Usage);

	if (Usage & D3DUSAGE_RENDERTARGET)
	{		
		LOG_DBG_DTDM("texture as rendertarget %u", Format);
		baseSurface = new d912pxy_surface(dev, Width, Height, Format, D3DMULTISAMPLE_NONE, 0, 0, 0);		
		rtTrans = 1;
	}
	else if (Usage & D3DUSAGE_DEPTHSTENCIL)
	{
		LOG_DBG_DTDM("texture as depthstencil");
		baseSurface = new d912pxy_surface(dev, Width, Height, Format, D3DMULTISAMPLE_NONE, 0, 0, 1);
		rtTrans = 1;
	}
	else {
		LOG_DBG_DTDM("texture as SRV");
		if (m_levels != 0)
		{
			baseSurface = d912pxy_s(pool_surface)->GetSurface(Width, Height, Format, m_levels, 1);
		} else 
			baseSurface = new d912pxy_surface(dev, Width, Height, Format, Usage, &m_levels, 1);
		rtTrans = 0;
	}

	srvIDc[1] = rtTrans;
	srvIDc[0] = baseSurface->GetSRVHeapId();
}


d912pxy_texture::~d912pxy_texture()
{
	baseSurface->Release();
}

D912PXY_IUNK_IMPL

/*** IDirect3DResource9 methods ***/
D912PXY_METHOD_IMPL(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) { return d912pxy_resource::GetDevice(ppDevice); }
D912PXY_METHOD_IMPL(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags) { return d912pxy_resource::SetPrivateData(refguid, pData, SizeOfData, Flags); }
D912PXY_METHOD_IMPL(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData) { return d912pxy_resource::GetPrivateData(refguid, pData, pSizeOfData); }
D912PXY_METHOD_IMPL(FreePrivateData)(THIS_ REFGUID refguid) { return d912pxy_resource::FreePrivateData(refguid); }
D912PXY_METHOD_IMPL_(DWORD, SetPriority)(THIS_ DWORD PriorityNew) { return d912pxy_resource::SetPriority(PriorityNew); }
D912PXY_METHOD_IMPL_(DWORD, GetPriority)(THIS) { return d912pxy_basetexture::GetPriority(); }
D912PXY_METHOD_IMPL_(void, PreLoad)(THIS) { d912pxy_resource::PreLoad(); }
D912PXY_METHOD_IMPL_(D3DRESOURCETYPE, GetType)(THIS) { return d912pxy_resource::GetType(); }

D912PXY_METHOD_IMPL_(DWORD, SetLOD)(THIS_ DWORD LODNew){ return d912pxy_basetexture::SetLOD(LODNew); }
D912PXY_METHOD_IMPL_(DWORD, GetLOD)(THIS){ return d912pxy_basetexture::GetLOD(); }
D912PXY_METHOD_IMPL_(DWORD, GetLevelCount)(THIS){ return d912pxy_basetexture::GetLevelCount(); }
D912PXY_METHOD_IMPL(SetAutoGenFilterType)(THIS_ D3DTEXTUREFILTERTYPE FilterType){ return d912pxy_basetexture::SetAutoGenFilterType(FilterType); }
D912PXY_METHOD_IMPL_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)(THIS){ return d912pxy_basetexture::GetAutoGenFilterType(); }
D912PXY_METHOD_IMPL_(void, GenerateMipSubLevels)(THIS)
{ 
	d912pxy_basetexture::GenerateMipSubLevels(); 
}

D912PXY_METHOD_IMPL(GetLevelDesc)(THIS_ UINT Level, D3DSURFACE_DESC *pDesc)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(1)

	*pDesc = baseSurface->GetDX9DescAtLevel(Level);

	API_OVERHEAD_TRACK_END(1)
	return D3D_OK; 
}

D912PXY_METHOD_IMPL(GetSurfaceLevel)(THIS_ UINT Level, IDirect3DSurface9** ppSurfaceLevel)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(1)
	
	if (srvIDc[1])
	{
		*ppSurfaceLevel = (IDirect3DSurface9*)baseSurface;
	} else 
		*ppSurfaceLevel = (IDirect3DSurface9*)baseSurface->GetLayer(Level, 0);

	(*ppSurfaceLevel)->AddRef();

	API_OVERHEAD_TRACK_END(1)

	return D3D_OK; 
}

D912PXY_METHOD_IMPL(LockRect)(THIS_ UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{ 
	LOG_DBG_DTDM("LockRect lv %u", Level);

	API_OVERHEAD_TRACK_START(1)

	HRESULT ret = baseSurface->GetLayer(Level, 0)->LockRect(pLockedRect, pRect, Flags);

	API_OVERHEAD_TRACK_END(1)

	return ret;
}

D912PXY_METHOD_IMPL(UnlockRect)(THIS_ UINT Level)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(1)

	HRESULT ret = baseSurface->GetLayer(Level, 0)->UnlockRect();

	API_OVERHEAD_TRACK_END(1)

	return ret;
}

D912PXY_METHOD_IMPL(AddDirtyRect)(THIS_ CONST RECT* pDirtyRect)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(1)

	if (m_levels == 1)
		baseSurface->GetLayer(0,0)->SetDirtyRect(pDirtyRect->left, pDirtyRect->right, pDirtyRect->top, pDirtyRect->bottom);
	
	API_OVERHEAD_TRACK_END(1)

	return D3D_OK; 
}

#undef D912PXY_METHOD_IMPL_CN
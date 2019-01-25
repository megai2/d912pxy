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
#pragma once
#include "stdafx.h"

class d912pxy_ctexture : public IDirect3DCubeTexture9, public d912pxy_basetexture
{
public:
	d912pxy_ctexture(d912pxy_device* dev, UINT edgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format);
	~d912pxy_ctexture();

	/*** IUnknown methods ***/
	D912PXY_METHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	D912PXY_METHOD_(ULONG, AddRef)(THIS);
	D912PXY_METHOD_(ULONG, Release)(THIS);

	/*** IDirect3DBaseTexture9 methods ***/
	D912PXY_METHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice);
	D912PXY_METHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	D912PXY_METHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData);
	D912PXY_METHOD(FreePrivateData)(THIS_ REFGUID refguid);
	D912PXY_METHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew);
	D912PXY_METHOD_(DWORD, GetPriority)(THIS);
	D912PXY_METHOD_(void, PreLoad)(THIS);
	D912PXY_METHOD_(D3DRESOURCETYPE, GetType)(THIS);
	D912PXY_METHOD_(DWORD, SetLOD)(THIS_ DWORD LODNew);
	D912PXY_METHOD_(DWORD, GetLOD)(THIS);
	D912PXY_METHOD_(DWORD, GetLevelCount)(THIS);
	D912PXY_METHOD(SetAutoGenFilterType)(THIS_ D3DTEXTUREFILTERTYPE FilterType);
	D912PXY_METHOD_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)(THIS);
	D912PXY_METHOD_(void, GenerateMipSubLevels)(THIS);
	D912PXY_METHOD(GetLevelDesc)(THIS_ UINT Level, D3DSURFACE_DESC *pDesc);
	D912PXY_METHOD(GetCubeMapSurface)(THIS_ D3DCUBEMAP_FACES FaceType, UINT Level, IDirect3DSurface9** ppCubeMapSurface);
	D912PXY_METHOD(LockRect)(THIS_ D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);
	D912PXY_METHOD(UnlockRect)(THIS_ D3DCUBEMAP_FACES FaceType, UINT Level);
	D912PXY_METHOD(AddDirtyRect)(THIS_ D3DCUBEMAP_FACES FaceType, CONST RECT* pDirtyRect);

	UINT GetSRVHeapId();

private:
	d912pxy_surface * faceSurfaces[6];
};


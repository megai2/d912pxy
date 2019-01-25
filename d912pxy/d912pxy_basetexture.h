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

class d912pxy_basetexture : public IDirect3DBaseTexture9, public d912pxy_resource
{
public:
	d912pxy_basetexture(d912pxy_device * dev);
	~d912pxy_basetexture();

	D912PXY_METHOD(QueryInterface)( REFIID riid, void** ppvObj);
	D912PXY_METHOD_(ULONG, AddRef)();
	D912PXY_METHOD_(ULONG, Release)();

	D912PXY_METHOD(GetDevice)( IDirect3DDevice9** ppDevice);
	D912PXY_METHOD(SetPrivateData)( REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	D912PXY_METHOD(GetPrivateData)( REFGUID refguid, void* pData, DWORD* pSizeOfData);
	D912PXY_METHOD(FreePrivateData)( REFGUID refguid);
	D912PXY_METHOD_(DWORD, SetPriority)( DWORD PriorityNew);
	D912PXY_METHOD_(DWORD, GetPriority)();
	D912PXY_METHOD_(void, PreLoad)();
	D912PXY_METHOD_(D3DRESOURCETYPE, GetType)();
	D912PXY_METHOD_(DWORD, SetLOD)( DWORD LODNew);
	D912PXY_METHOD_(DWORD, GetLOD)();
	D912PXY_METHOD_(DWORD, GetLevelCount)();
	D912PXY_METHOD(SetAutoGenFilterType)( D3DTEXTUREFILTERTYPE FilterType);
	D912PXY_METHOD_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)();
	D912PXY_METHOD_(void, GenerateMipSubLevels)();

	UINT GetSRVHeapId();

	d912pxy_surface* GetBaseSurface() { return baseSurface; };

	UINT FinalRelease();

protected:
	UINT32 * srvIDc;

	d912pxy_surface * baseSurface;

	UINT m_levels;	
};


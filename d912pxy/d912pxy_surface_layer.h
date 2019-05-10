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

class d912pxy_surface_layer: public IDirect3DSurface9
{
public:
	d912pxy_surface_layer(d912pxy_surface* iBase, UINT32 iSubres, UINT32 iBSize, UINT32 iWPitch, UINT32 iWidth, UINT32 imemPerPix);
	~d912pxy_surface_layer();

	/*** IUnknown methods ***/
	D912PXY_METHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	D912PXY_METHOD_(ULONG, AddRef)(THIS);
	D912PXY_METHOD_(ULONG, Release)(THIS);

	/*** IDirect3DResource9 methods ***/
	D912PXY_METHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice);
	D912PXY_METHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	D912PXY_METHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData);
	D912PXY_METHOD(FreePrivateData)(THIS_ REFGUID refguid);
	D912PXY_METHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew);
	D912PXY_METHOD_(DWORD, GetPriority)(THIS);
	D912PXY_METHOD_(void, PreLoad)(THIS);
	D912PXY_METHOD_(D3DRESOURCETYPE, GetType)(THIS);
	D912PXY_METHOD(GetContainer)(THIS_ REFIID riid, void** ppContainer);
	D912PXY_METHOD(GetDesc)(THIS_ D3DSURFACE_DESC *pDesc);
	D912PXY_METHOD(LockRect)(THIS_ D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);
	D912PXY_METHOD(UnlockRect)(THIS);
	D912PXY_METHOD(GetDC)(THIS_ HDC *phdc);
	D912PXY_METHOD(ReleaseDC)(THIS_ HDC hdc);

	void SetDirtyRect(UINT32 left, UINT32 right, UINT32 top, UINT32 bottom);

	HRESULT UnlockRectEx(UINT32 transform);

	void* SurfacePixel(UINT32 x, UINT32 y);

private:
	UINT isDrect;	
	D3DRECT drect;

	d912pxy_surface * base;
	UINT32 subres;
	
	UINT32 lockDepth;

	void* surfMem;
	
	UINT wPitch;
	UINT width;
	UINT memPerPix;

	UINT intRefc;

};


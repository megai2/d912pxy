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

class d912pxy_surface_layer : public d912pxy_vtable
{
public:	
	static d912pxy_surface_layer* d912pxy_surface_layer_com(d912pxy_com_object* iBase, UINT32 iSubres, UINT32 iBSize, UINT32 iWPitch, UINT32 iWidth, UINT32 imemPerPix);
	static void DeAllocate(d912pxy_surface_layer* obj);
	
	D912PXY_METHOD(QueryInterface)(PXY_THIS_ REFIID riid, void** ppvObj);
	D912PXY_METHOD_(ULONG, AddRef)(PXY_THIS);
	D912PXY_METHOD_(ULONG, Release)(PXY_THIS);
	D912PXY_METHOD(GetDevice)(PXY_THIS_ IDirect3DDevice9** ppDevice);
	D912PXY_METHOD(SetPrivateData)(PXY_THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	D912PXY_METHOD(GetPrivateData)(PXY_THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData);
	D912PXY_METHOD(FreePrivateData)(PXY_THIS_ REFGUID refguid);
	D912PXY_METHOD_(DWORD, SetPriority)(PXY_THIS_ DWORD PriorityNew);
	D912PXY_METHOD_(DWORD, GetPriority)(PXY_THIS);
	D912PXY_METHOD_(void, PreLoad)(PXY_THIS);
	D912PXY_METHOD_(D3DRESOURCETYPE, GetType)(PXY_THIS);
	D912PXY_METHOD(GetContainer)(PXY_THIS_ REFIID riid, void** ppContainer);
	D912PXY_METHOD(GetDesc)(PXY_THIS_ D3DSURFACE_DESC *pDesc);
	D912PXY_METHOD(LockRect)(PXY_THIS_ D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);
	D912PXY_METHOD(UnlockRect)(PXY_THIS);
	D912PXY_METHOD(GetDC)(PXY_THIS_ HDC *phdc);
	D912PXY_METHOD(ReleaseDC)(PXY_THIS_ HDC hdc);

	D912PXY_METHOD_NC(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	D912PXY_METHOD_NC_(ULONG, AddRef)(THIS);
	D912PXY_METHOD_NC_(ULONG, Release)(THIS);

	D912PXY_METHOD_NC(GetDesc)(D3DSURFACE_DESC *pDesc);
	D912PXY_METHOD_NC(LockRect)(D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);
	D912PXY_METHOD_NC(UnlockRect)();

	void SetDirtyRect(UINT32 left, UINT32 right, UINT32 top, UINT32 bottom);
	void* SurfacePixel(UINT32 x, UINT32 y);

	d912pxy_com_object* GetBaseSurface() { return base; };

private:
	d912pxy_surface_layer(d912pxy_com_object* iBase, UINT32 iSubres, UINT32 iBSize, UINT32 iWPitch, UINT32 iWidth, UINT32 imemPerPix);
	~d912pxy_surface_layer();

	UINT isDrect;	
	D3DRECT drect;

	d912pxy_com_object * base;
	UINT32 subres;
	
	UINT32 lockDepth;
	UINT32 lockWrite;

	void* surfMem;
	
	UINT wPitch;
	UINT width;
	UINT memPerPix;

};


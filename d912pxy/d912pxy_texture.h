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

class d912pxy_device;

class d912pxy_texture : public d912pxy_basetexture
{
public:
	static d912pxy_texture* d912pxy_texture_com(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format);

	~d912pxy_texture();

	D912PXY_METHOD(GetLevelDesc)(PXY_THIS_ UINT Level, D3DSURFACE_DESC *pDesc);
	D912PXY_METHOD(GetSurfaceLevel)(PXY_THIS_ UINT Level, IDirect3DSurface9** ppSurfaceLevel);
	D912PXY_METHOD(LockRect)(PXY_THIS_ UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);
	D912PXY_METHOD(UnlockRect)(PXY_THIS_ UINT Level);
	D912PXY_METHOD(AddDirtyRect)(PXY_THIS_ CONST RECT* pDirtyRect);

	D912PXY_METHOD_NC(GetLevelDesc)(THIS_ UINT Level, D3DSURFACE_DESC *pDesc);
	D912PXY_METHOD_NC(GetSurfaceLevel)(THIS_ UINT Level, IDirect3DSurface9** ppSurfaceLevel);
	D912PXY_METHOD_NC(LockRect)(THIS_ UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);
	D912PXY_METHOD_NC(UnlockRect)(THIS_ UINT Level);
	D912PXY_METHOD_NC(AddDirtyRect)(THIS_ CONST RECT* pDirtyRect);

private:
	d912pxy_texture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format);
};


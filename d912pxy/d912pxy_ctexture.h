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

class d912pxy_ctexture : public d912pxy_basetexture
{
public:	
	static d912pxy_ctexture* d912pxy_ctexture_com(UINT edgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format);
	~d912pxy_ctexture();

	D912PXY_METHOD(GetLevelDesc)(PXY_THIS_ UINT Level, D3DSURFACE_DESC *pDesc);
	D912PXY_METHOD(GetCubeMapSurface)(PXY_THIS_ D3DCUBEMAP_FACES FaceType, UINT Level, IDirect3DSurface9** ppCubeMapSurface);
	D912PXY_METHOD(LockRect)(PXY_THIS_ D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);
	D912PXY_METHOD(UnlockRect)(PXY_THIS_ D3DCUBEMAP_FACES FaceType, UINT Level);
	D912PXY_METHOD(AddDirtyRect)(PXY_THIS_ D3DCUBEMAP_FACES FaceType, CONST RECT* pDirtyRect);

	D912PXY_METHOD_NC(GetLevelDesc)(THIS_ UINT Level, D3DSURFACE_DESC *pDesc);
	D912PXY_METHOD_NC(GetCubeMapSurface)(THIS_ D3DCUBEMAP_FACES FaceType, UINT Level, IDirect3DSurface9** ppCubeMapSurface);
	D912PXY_METHOD_NC(LockRect)(THIS_ D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);
	D912PXY_METHOD_NC(UnlockRect)(THIS_ D3DCUBEMAP_FACES FaceType, UINT Level);
	D912PXY_METHOD_NC(AddDirtyRect)(THIS_ D3DCUBEMAP_FACES FaceType, CONST RECT* pDirtyRect);

	UINT GetSRVHeapId();

private:
	d912pxy_ctexture(UINT edgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format);

	d912pxy_surface * faceSurfaces[6];
};


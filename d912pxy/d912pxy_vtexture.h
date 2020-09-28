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

class d912pxy_vtexture : public d912pxy_basetexture
{
public:
	static d912pxy_vtexture* d912pxy_vtexture_com(UINT Width, UINT Height, UINT depth, UINT Levels, DWORD Usage, D3DFORMAT Format);
	~d912pxy_vtexture();
		
	D912PXY_METHOD(GetLevelDesc)(PXY_THIS_ UINT Level, D3DVOLUME_DESC *pDesc);
	D912PXY_METHOD(GetVolumeLevel)(PXY_THIS_ UINT Level, IDirect3DVolume9** ppVolumeLevel);
	D912PXY_METHOD(LockBox)(PXY_THIS_ UINT Level, D3DLOCKED_BOX* pLockedVolume, CONST D3DBOX* pBox, DWORD Flags);
	D912PXY_METHOD(UnlockBox)(PXY_THIS_ UINT Level);
	D912PXY_METHOD(AddDirtyBox)(PXY_THIS_ CONST D3DBOX* pDirtyBox);

	D912PXY_METHOD_NC(GetLevelDesc)(THIS_ UINT Level, D3DVOLUME_DESC* pDesc);
	D912PXY_METHOD_NC(GetVolumeLevel)(THIS_ UINT Level, IDirect3DVolume9** ppVolumeLevel);
	D912PXY_METHOD_NC(LockBox)(THIS_ UINT Level, D3DLOCKED_BOX* pLockedVolume, CONST D3DBOX* pBox, DWORD Flags);
	D912PXY_METHOD_NC(UnlockBox)(THIS_ UINT Level);
	D912PXY_METHOD_NC(AddDirtyBox)(THIS_ CONST D3DBOX* pDirtyBox);

private:
	d912pxy_vtexture(UINT Width, UINT Height, UINT depth, UINT Levels, DWORD Usage, D3DFORMAT Format);
};


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

d912pxy_vtexture::d912pxy_vtexture(d912pxy_device* dev, UINT Width, UINT Height, UINT depth, UINT Levels, DWORD Usage, D3DFORMAT Format) : d912pxy_basetexture(dev)
{

}


d912pxy_vtexture::~d912pxy_vtexture()
{

}

#define D912PXY_METHOD_IMPL_CN d912pxy_vtexture

D912PXY_IUNK_IMPL

/*** IDirect3DBaseTexture9 METHOD_IMPLs ***/
D912PXY_METHOD_IMPL(GetDevice)(THIS_ IDirect3DDevice9** ppDevice){ return 0; }
D912PXY_METHOD_IMPL(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags){ return 0; }
D912PXY_METHOD_IMPL(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData){ return 0; }
D912PXY_METHOD_IMPL(FreePrivateData)(THIS_ REFGUID refguid){ return 0; }
D912PXY_METHOD_IMPL_(DWORD, SetPriority)(THIS_ DWORD PriorityNew){ return 0; }
D912PXY_METHOD_IMPL_(DWORD, GetPriority)(THIS){ return 0; }
D912PXY_METHOD_IMPL_(void, PreLoad)(THIS){ return; }
D912PXY_METHOD_IMPL_(D3DRESOURCETYPE, GetType)(THIS) { return d912pxy_resource::GetType(); }
D912PXY_METHOD_IMPL_(DWORD, SetLOD)(THIS_ DWORD LODNew){ return 0; }
D912PXY_METHOD_IMPL_(DWORD, GetLOD)(THIS){ return 0; }
D912PXY_METHOD_IMPL_(DWORD, GetLevelCount)(THIS){ return 0; }
D912PXY_METHOD_IMPL(SetAutoGenFilterType)(THIS_ D3DTEXTUREFILTERTYPE FilterType){ return 0; }
D912PXY_METHOD_IMPL_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)(THIS){ return D3DTEXF_NONE; }
D912PXY_METHOD_IMPL_(void, GenerateMipSubLevels)(THIS){ return; }
D912PXY_METHOD_IMPL(GetLevelDesc)(THIS_ UINT Level, D3DVOLUME_DESC *pDesc){ return 0; }
D912PXY_METHOD_IMPL(GetVolumeLevel)(THIS_ UINT Level, IDirect3DVolume9** ppVolumeLevel){ return 0; }
D912PXY_METHOD_IMPL(LockBox)(THIS_ UINT Level, D3DLOCKED_BOX* pLockedVolume, CONST D3DBOX* pBox, DWORD Flags){ return 0; }
D912PXY_METHOD_IMPL(UnlockBox)(THIS_ UINT Level){ return 0; }
D912PXY_METHOD_IMPL(AddDirtyBox)(THIS_ CONST D3DBOX* pDirtyBox){ return 0; }

#undef D912PXY_METHOD_IMPL_CN
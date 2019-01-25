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

#define D912PXY_METHOD_IMPL_CN d912pxy_basetexture

d912pxy_basetexture::d912pxy_basetexture(d912pxy_device * dev) : d912pxy_resource(dev, RTID_TEXTURE, L"texture")
{

}

d912pxy_basetexture::~d912pxy_basetexture()
{
}

D912PXY_IUNK_IMPL

/*** IDirect3DResource9 methods ***/
D912PXY_METHOD_IMPL(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) { return d912pxy_resource::GetDevice(ppDevice); }
D912PXY_METHOD_IMPL(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags) { return d912pxy_resource::SetPrivateData(refguid, pData, SizeOfData, Flags); }
D912PXY_METHOD_IMPL(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData) { return d912pxy_resource::GetPrivateData(refguid, pData, pSizeOfData); }
D912PXY_METHOD_IMPL(FreePrivateData)(THIS_ REFGUID refguid) { return d912pxy_resource::FreePrivateData(refguid); }
D912PXY_METHOD_IMPL_(DWORD, SetPriority)(THIS_ DWORD PriorityNew) { return d912pxy_resource::SetPriority(PriorityNew); }

//do lil hack for speed
D912PXY_METHOD_IMPL_(DWORD, GetPriority)(THIS) 
{ 
	return baseSurface->GetSRVHeapId();
}

D912PXY_METHOD_IMPL_(void, PreLoad)(THIS) { d912pxy_resource::PreLoad(); }
D912PXY_METHOD_IMPL_(D3DRESOURCETYPE, GetType)(THIS) { return d912pxy_resource::GetType(); }

D912PXY_METHOD_IMPL_(DWORD, SetLOD)(DWORD LODNew)
{
	LOG_DBG_DTDM(__FUNCTION__);
	return D3D_OK;
}
D912PXY_METHOD_IMPL_(DWORD, GetLOD)()
{
	LOG_DBG_DTDM(__FUNCTION__);
	return 0;
}

D912PXY_METHOD_IMPL_(DWORD, GetLevelCount)()
{
	LOG_DBG_DTDM(__FUNCTION__);
	return 1;
}

D912PXY_METHOD_IMPL(SetAutoGenFilterType)(D3DTEXTUREFILTERTYPE FilterType)
{
	LOG_DBG_DTDM(__FUNCTION__);
	return D3D_OK;
}

D912PXY_METHOD_IMPL_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)()
{
	LOG_DBG_DTDM(__FUNCTION__);
	return D3DTEXF_POINT;
}

D912PXY_METHOD_IMPL_(void, GenerateMipSubLevels)()
{
	LOG_DBG_DTDM(__FUNCTION__);
	return;
}

#undef D912PXY_METHOD_IMPL_CN

UINT d912pxy_basetexture::GetSRVHeapId()
{
	return baseSurface->GetSRVHeapId();
}

UINT d912pxy_basetexture::FinalRelease()
{
	void* bptr = (void*)srvIDc;
	if (d912pxy_comhandler::FinalReleaseTest() == 3)
	{
		this->~d912pxy_basetexture();
		free(bptr);
		return 1;
	}
	else
		return 2;	
}
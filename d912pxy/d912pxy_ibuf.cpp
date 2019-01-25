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

d912pxy_ibuf::d912pxy_ibuf(d912pxy_vstream* iBase) 
{
	base = iBase;	
}

d912pxy_ibuf::~d912pxy_ibuf()
{

}

#define D912PXY_METHOD_IMPL_CN d912pxy_ibuf

D912PXY_METHOD_IMPL(QueryInterface)(THIS_ REFIID riid, void** ppvObj)
{
	return base->QueryInterface(riid, ppvObj);
}

D912PXY_METHOD_IMPL_(ULONG, AddRef)(THIS)
{
	return base->AddRef();
}

D912PXY_METHOD_IMPL_(ULONG, Release)(THIS)
{
	return base->Release();
}

/*** IDirect3DResource9 methods ***/
D912PXY_METHOD_IMPL(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) { return base->GetDevice(ppDevice); }
D912PXY_METHOD_IMPL(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags) { return base->SetPrivateData(refguid, pData, SizeOfData, Flags); }
D912PXY_METHOD_IMPL(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData) { return base->GetPrivateData(refguid, pData, pSizeOfData); }
D912PXY_METHOD_IMPL(FreePrivateData)(THIS_ REFGUID refguid) { return base->FreePrivateData(refguid); }
D912PXY_METHOD_IMPL_(DWORD, SetPriority)(THIS_ DWORD PriorityNew) { return base->SetPriority(PriorityNew); }
D912PXY_METHOD_IMPL_(DWORD, GetPriority)(THIS) { return base->GetPriority(); }
D912PXY_METHOD_IMPL_(void, PreLoad)(THIS) { return base->PreLoad(); }
D912PXY_METHOD_IMPL_(D3DRESOURCETYPE, GetType)(THIS) { return base->GetType(); }

D912PXY_METHOD_IMPL(Lock)(THIS_ UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags)
{
//	LOG_DBG_DTDM("Lock %u %u %u", OffsetToLock, SizeToLock, Flags);

	base->Lock(OffsetToLock, SizeToLock, ppbData, Flags);

	return D3D_OK;
}

D912PXY_METHOD_IMPL(Unlock)(THIS)
{
//	LOG_DBG_DTDM(__FUNCTION__);

	base->Unlock();

	return D3D_OK;
}

D912PXY_METHOD_IMPL(GetDesc)(THIS_ D3DINDEXBUFFER_DESC *pDesc)
{
//	LOG_DBG_DTDM(__FUNCTION__);

	return D3DERR_INVALIDCALL;
}

#undef D912PXY_METHOD_IMPL_CN

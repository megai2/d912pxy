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

d912pxy_vbuf::d912pxy_vbuf(d912pxy_vstream * iBase) 
{
	base = iBase;
}

d912pxy_vbuf::~d912pxy_vbuf()
{

}

#define D912PXY_METHOD_IMPL_CN d912pxy_vbuf

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

	API_OVERHEAD_TRACK_START(2)

	base->Lock(OffsetToLock, SizeToLock, ppbData, Flags);

	API_OVERHEAD_TRACK_END(2)

	return D3D_OK;
}

D912PXY_METHOD_IMPL(Unlock)(THIS)
{ 
//	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(2)
	
	base->Unlock();

	API_OVERHEAD_TRACK_END(2)

	return D3D_OK; 
}

D912PXY_METHOD_IMPL(GetDesc)(THIS_ D3DVERTEXBUFFER_DESC *pDesc)
{
//	LOG_DBG_DTDM(__FUNCTION__);

	return D3DERR_INVALIDCALL;
}

#undef D912PXY_METHOD_IMPL_CN

/*

void d912pxy_vbuf::ReformatDataForVDecl(UINT stride, UINT slot, d912pxy_vdecl * decl)
{
	if (opFlags & PXY_INNER_BUFFER_FLAG_REFMTD)
		return;

	D3DVERTEXELEMENT9* declEl;
	UINT cnt;

	declEl = decl->GetDeclarationPtr(&cnt);
	--cnt;

	for (int i = 0; i != cnt; ++i)
	{
		if (declEl[i].Stream != slot)
			continue;

		switch (declEl[i].Type)
		{
		case D3DDECLTYPE_D3DCOLOR:
		{
			UINT bIdx = 0;
			UINT8* aaByte = (UINT8*)bufferMem;
			while (bIdx < (dx9desc.Size - declEl[i].Offset - 3))
			{				
				//FMT for dxgi is ABGR
				//DX9 really uses ARGB

				UINT8 swap = 0;

				UINT32 colorV = *(UINT32*)&aaByte[bIdx + declEl[i].Offset];

				//  WW ZZ YY XX
				//0xFF FF FF FF
				// d3d color is 
				//  AA RR GG BB				
				//expands to 
				//  AA BB GG RR

				// so
				// W = A
				// Z = B
				// Y = G
				// X = R

				//means we need to swap RR and BB

				colorV = colorV & 0xFF00FF00 |
					((colorV & 0xFF) << 16) | //move B to Z
					((colorV & 0xFF0000) >> 16) //move R to X
					;

				*(UINT32*)&aaByte[bIdx + declEl[i].Offset] = colorV;

				bIdx += stride;				
			} 
		}
		break;
		default:
			;
		}
	}

	opFlags |= PXY_INNER_BUFFER_FLAG_REFMTD;
}
*/
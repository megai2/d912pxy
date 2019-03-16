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

#define PXY_INNER_BUFFER_FLAG_DIRTY 1

//megai2: probably better way exists
#define d912pxy_vstream_from_index(val) ((d912pxy_vstream*)((IDirect3DVertexBuffer9*)val))
#define d912pxy_vstream_to_index(val) ((IDirect3DIndexBuffer9*)((IDirect3DVertexBuffer9*)val))

typedef struct d912pxy_vstream_lock_data {
	d912pxy_vstream* dst;
	UINT32 size;
	UINT32 offset;
} d912pxy_vstream_lock_data;

class d912pxy_vstream : public d912pxy_resource, public IDirect3DVertexBuffer9
{
public:
	d912pxy_vstream(d912pxy_device* dev, UINT Length, DWORD Usage, DWORD fmt, DWORD isIB);
	~d912pxy_vstream();

	//DX9 com methods

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

	D912PXY_METHOD(Lock)(THIS_ UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags);
	D912PXY_METHOD(Unlock)(THIS);
	D912PXY_METHOD(GetDesc)(THIS_ D3DVERTEXBUFFER_DESC *pDesc);
		
	//internal methods

	void IFrameBindVB(UINT stride, UINT slot, UINT offset, ID3D12GraphicsCommandList* cl);
	void IFrameBindIB(ID3D12GraphicsCommandList* cl);

	void NoteFormatChange(DWORD fmt, DWORD isIB);

	UINT FinalReleaseCB();

	IDirect3DVertexBuffer9* AsDX9VB();
	IDirect3DIndexBuffer9* AsDX9IB();

	UINT32 PooledAction(UINT32 use);
	void UploadDataCopy(intptr_t ulMem, UINT32 offset, UINT32 size);

	void ProcessUpload(d912pxy_vstream_lock_data* linfo, ID3D12GraphicsCommandList * cl);

private:	
	union bindData {
		D3D12_VERTEX_BUFFER_VIEW v;
		D3D12_INDEX_BUFFER_VIEW i;
	} bindData;

	D3DVERTEXBUFFER_DESC dx9desc;
	
	void* data;

	d912pxy_upload_item* ulObj;

	d912pxy_vstream_lock_data lockInfo[PXY_INNER_MAX_LOCK_DEPTH];
	LONG lockDepth;
};


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

typedef struct d912pxy_vstream_lock_data {
	d912pxy_vstream* dst;
	UINT32 size;
	UINT32 offset;
} d912pxy_vstream_lock_data;

class d912pxy_vstream : public d912pxy_vtable, public d912pxy_resource
{
public:	
	static d912pxy_vstream* d912pxy_vstream_com(UINT Length, DWORD Usage, DWORD fmt, DWORD isIB);
	~d912pxy_vstream();

	//DX9 com methods

	D912PXY_METHOD(Lock)(PXY_THIS_ UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags);
	D912PXY_METHOD(Unlock)(PXY_THIS);	
	D912PXY_METHOD(GetDesc)(PXY_THIS_ D3DVERTEXBUFFER_DESC *pDesc);
			
	//internal methods

	D912PXY_METHOD_NC(Lock)(THIS_ UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags);
	D912PXY_METHOD_NC(Unlock)(THIS);

	void UnlockRanged(UINT newOffset, UINT newSize);

	void IFrameBindVB(UINT stride, UINT slot, UINT offset, ID3D12GraphicsCommandList* cl);
	void IFrameBindIB(ID3D12GraphicsCommandList* cl);

	void NoteFormatChange(DWORD fmt, DWORD isIB);

	UINT FinalReleaseCB();

	UINT32 PooledAction(UINT32 use);
	void UploadDataCopy(intptr_t ulMem, UINT32 offset, UINT32 size);

	void ProcessUpload(d912pxy_vstream_lock_data* linfo, ID3D12GraphicsCommandList * cl);

	void FinishUpload(ID3D12GraphicsCommandList * cl);

	void ConstructResource();

	UINT GetLength();

	static UINT32 threadedCtor;

	void LoadFromBlock(const d912pxy::MemoryArea& mem);

private:	
	d912pxy_vstream(UINT Length, DWORD Usage, DWORD fmt, DWORD isIB);

	d912pxy_upload_item* ul;
	UINT64 ul_offset;

	union bindData {
		D3D12_VERTEX_BUFFER_VIEW v;
		D3D12_INDEX_BUFFER_VIEW i;
	} bindData;

	D3DVERTEXBUFFER_DESC dx9desc;
	
	void* data;

	d912pxy_vstream_lock_data lockInfo[PXY_INNER_MAX_LOCK_DEPTH];
	LONG lockDepth;		
};


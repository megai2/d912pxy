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

class d912pxy_vstream : public d912pxy_resource
{
public:
	d912pxy_vstream(d912pxy_device* dev, UINT Length, DWORD Usage, DWORD fmt, DWORD isIB);
	~d912pxy_vstream();

	void Lock(UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags);
	void Unlock();

	void IFrameBindVB(UINT stride, UINT slot, UINT offset, ID3D12GraphicsCommandList* cl);
	void IFrameBindIB(ID3D12GraphicsCommandList* cl);

	void NoteFormatChange(DWORD fmt, DWORD isIB);

	UINT FinalReleaseCB();

	IDirect3DVertexBuffer9* AsDX9VB();
	IDirect3DIndexBuffer9* AsDX9IB();

	UINT32 PooledAction(UINT32 use);

	void AsyncUploadDataCopy(UINT32 offset, UINT32 size, ID3D12GraphicsCommandList * cl);

private:	
	union bindData {
		D3D12_VERTEX_BUFFER_VIEW v;
		D3D12_INDEX_BUFFER_VIEW i;
	} bindData;

	D3DVERTEXBUFFER_DESC dx9desc;

	D3D12_RANGE mappingRange;

	d912pxy_vbuf* vbR;
	d912pxy_ibuf* ibR;

	void* data;

	UINT lockDepth;
	UINT lockOffsets[PXY_INNER_MAX_LOCK_DEPTH];
	UINT lockSizes[PXY_INNER_MAX_LOCK_DEPTH];
};


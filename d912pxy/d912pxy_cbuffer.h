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

class d912pxy_cbuffer : public d912pxy_resource
{
public:
	d912pxy_cbuffer(UINT length, UINT uploadOnly=0);	
	d912pxy_cbuffer(UINT length, UINT uploadOnly, UINT32 allowUploadBuffer);
	d912pxy_cbuffer(d912pxy_cbuffer* oBuf, UINT offset, UINT iSz);
	~d912pxy_cbuffer();

	void WriteUINT32(UINT index, UINT32* val, UINT count);
	void WriteFloat(UINT index, float* val, UINT count);

	D3D12_CPU_DESCRIPTOR_HANDLE GetDHeapHandle();
	D3D12_GPU_DESCRIPTOR_HANDLE GetDHeapGPUHandle();

	UINT GetHeapID() { return heapId; };

	void Upload();
	void UploadTarget(d912pxy_cbuffer* target, UINT offset, UINT size);
	void UploadOffset(UINT offset, UINT size);
	void UploadOffsetNB(ID3D12GraphicsCommandList* cq, UINT offset, UINT size);

	void* OffsetWritePoint(UINT offset);

	d912pxy_resource* GetUploadRes();

	intptr_t DevPtr() { return pointers.dev; };
	intptr_t HostPtr() { return pointers.host; };

private:
	d912pxy_resource_ptr pointers;

	d912pxy_resource * uploadRes;
	UINT heapId;
};


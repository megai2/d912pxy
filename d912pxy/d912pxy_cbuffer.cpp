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

d912pxy_cbuffer::d912pxy_cbuffer(d912pxy_device* dev, UINT length, UINT uploadOnly) : d912pxy_resource(dev, RTID_CBUFFER, L"const buffer")
{
	if ((length & 0xFF) != 0)
	{
		length = (length & ~0xFF) + 0x100;
	}

	if (!uploadOnly)
	{
		d12res_buffer(length, D3D12_HEAP_TYPE_DEFAULT);

		if (length < 0xFFFF)
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC viDsc;
			viDsc.BufferLocation = m_res->GetGPUVirtualAddress();
			viDsc.SizeInBytes = length;
			dHeap = dev->GetDHeap(PXY_INNER_HEAP_CBV);
			heapId = dHeap->CreateCBV(&viDsc);
		}
	}
	else
		dHeap = 0;

	uploadRes = new d912pxy_resource(dev, RTID_UL_BUF, L"constant upload buffer");
	uploadRes->d12res_buffer(length, D3D12_HEAP_TYPE_UPLOAD);

//	pointers.host = NULL;
	LOG_ERR_THROW(uploadRes->GetD12Obj()->Map(0, 0, (void**)&pointers.host));
}

d912pxy_cbuffer::d912pxy_cbuffer(d912pxy_device * dev, UINT length, UINT uploadOnly, void* n2) : d912pxy_resource(dev, RTID_CBUFFER, L"uav const buffer")
{
	if ((length & 0xFF) != 0)
	{
		length = (length & ~0xFF) + 0x100;
	}

	if (!uploadOnly)
	{
		d12res_uav_buffer(length, D3D12_HEAP_TYPE_DEFAULT);
	}
	else
		dHeap = 0;

	uploadRes = new d912pxy_resource(dev, RTID_UL_BUF, L"constant upload buffer");
	uploadRes->d12res_buffer(length, D3D12_HEAP_TYPE_UPLOAD);

	LOG_ERR_THROW(uploadRes->GetD12Obj()->Map(0, 0, (void**)&pointers.host));
	pointers.dev = uploadRes->GetVA_GPU();
}

d912pxy_cbuffer::d912pxy_cbuffer(d912pxy_device * dev, d912pxy_cbuffer * oBuf, UINT offset, UINT iSz) : d912pxy_resource(dev, RTID_CBUFFER, L"const buffer offset")
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC viDsc;

	if ((iSz & 0xFF) != 0)
	{
		iSz = (iSz & ~0xFF) + 0x100;
	}

	viDsc.BufferLocation = oBuf->GetD12Obj()->GetGPUVirtualAddress() + offset;
	viDsc.SizeInBytes = iSz;
	dHeap = dev->GetDHeap(PXY_INNER_HEAP_CBV);
	heapId = dHeap->CreateCBV(&viDsc);
	
	pointers.host = (intptr_t)oBuf->OffsetWritePoint(offset);
}

d912pxy_cbuffer::~d912pxy_cbuffer()
{	
	if (uploadRes)
		uploadRes->Release();

	if (dHeap)
		dHeap->FreeSlot(heapId);
}

void d912pxy_cbuffer::WriteUINT32(UINT index, UINT32* val, UINT count)
{
	if (!pointers.host)
		LOG_ERR_THROW(uploadRes->GetD12Obj()->Map(0, 0, (void**)&pointers.host));

	void* startMem = (void*)((intptr_t)pointers.host + index * 4);

	memcpy(startMem, val, count * 4);
}

void d912pxy_cbuffer::WriteFloat(UINT index, float* val, UINT count)
{
	if (!pointers.host)
		LOG_ERR_THROW(uploadRes->GetD12Obj()->Map(0, 0, (void**)&pointers.host));

	void* startMem = (void*)((intptr_t)pointers.host + index * 4);

	memcpy(startMem, val, count * 4);
}

D3D12_CPU_DESCRIPTOR_HANDLE d912pxy_cbuffer::GetDHeapHandle()
{
	return dHeap->GetDHeapHandle(heapId);
}

D3D12_GPU_DESCRIPTOR_HANDLE d912pxy_cbuffer::GetDHeapGPUHandle()
{
	return dHeap->GetGPUDHeapHandle(heapId);
}

void d912pxy_cbuffer::Upload()
{
	//pointers.host = NULL;
	//uploadRes->GetD12Obj()->Unmap(0, 0);
	uploadRes->CopyTo(this, 1, d912pxy_s(GPUcl)->GID(CLG_TOP));	
}

void d912pxy_cbuffer::UploadTarget(d912pxy_cbuffer * target, UINT offset, UINT size)
{
	ComPtr<ID3D12GraphicsCommandList> cq = d912pxy_s(GPUcl)->GID(CLG_SEQ);

	target->IFrameBarrierTrans(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_COPY_DEST, CLG_SEQ);

	cq->CopyBufferRegion(target->GetD12Obj(), offset, uploadRes->GetD12Obj(), offset, size);
}

void d912pxy_cbuffer::UploadOffset(UINT offset, UINT size)
{
	ComPtr<ID3D12GraphicsCommandList> cq = d912pxy_s(GPUcl)->GID(CLG_TOP);

	IFrameBarrierTrans(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_COPY_DEST, CLG_TOP);

	cq->CopyBufferRegion(m_res.Get(), offset, uploadRes->GetD12Obj(), offset, size);

	IFrameBarrierTrans(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_GENERIC_READ, CLG_TOP);
}

void d912pxy_cbuffer::UploadOffsetNB(ID3D12GraphicsCommandList* cq, UINT offset, UINT size)
{
	cq->CopyBufferRegion(m_res.Get(), offset, uploadRes->GetD12Obj(), offset, size);
}

void * d912pxy_cbuffer::OffsetWritePoint(UINT offset)
{
	return (void*)((intptr_t)pointers.host + offset);
}

d912pxy_resource * d912pxy_cbuffer::GetUploadRes()
{
	return uploadRes;
}

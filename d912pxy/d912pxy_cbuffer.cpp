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

d912pxy_cbuffer::d912pxy_cbuffer(UINT length, UINT32 allowUploadBuffer) : d912pxy_resource(RTID_CBUFFER, PXY_COM_OBJ_NOVTABLE, L"uav const buffer")
{
	if ((length & 0xFF) != 0)
	{
		length = (length & ~0xFF) + 0x100;
	}

	d12res_uav_buffer(length, D3D12_HEAP_TYPE_DEFAULT);

	dHeap = 0;

	if (allowUploadBuffer)
	{
		uploadRes = new d912pxy_resource(RTID_UL_BUF, PXY_COM_OBJ_NOVTABLE, L"constant upload buffer");
		uploadRes->d12res_buffer(length, D3D12_HEAP_TYPE_UPLOAD);
		LOG_ERR_THROW(uploadRes->GetD12Obj()->Map(0, 0, (void**)&pointers.host));
	}
	else
		uploadRes = NULL;

	pointers.dev = GetVA_GPU();
}

d912pxy_cbuffer::~d912pxy_cbuffer()
{	
	if (uploadRes)
		delete uploadRes;
}

void d912pxy_cbuffer::Upload()
{
	//pointers.host = NULL;
	//uploadRes->GetD12Obj()->Unmap(0, 0);
	uploadRes->BCopyTo(this, 2, d912pxy_s.dx12.cl->GID(CLG_TOP));	
}

void d912pxy_cbuffer::UploadTarget(d912pxy_cbuffer * target, UINT offset, UINT size)
{
	ComPtr<ID3D12GraphicsCommandList> cq = d912pxy_s.dx12.cl->GID(CLG_SEQ);

	target->BTransitGID(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_COPY_DEST, CLG_SEQ);

	cq->CopyBufferRegion(target->GetD12Obj(), offset, uploadRes->GetD12Obj(), offset, size);
}

void d912pxy_cbuffer::UploadOffset(UINT offset, UINT size)
{
	ComPtr<ID3D12GraphicsCommandList> cq = d912pxy_s.dx12.cl->GID(CLG_TOP);

	BTransitGID(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_COPY_DEST, CLG_TOP);

	cq->CopyBufferRegion(m_res, offset, uploadRes->GetD12Obj(), offset, size);

	BTransitGID(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_GENERIC_READ, CLG_TOP);
}

void d912pxy_cbuffer::UploadOffsetNB(ID3D12GraphicsCommandList* cq, UINT offset, UINT size)
{
	cq->CopyBufferRegion(m_res, offset, uploadRes->GetD12Obj(), offset, size);
}
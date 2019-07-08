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
#include "d912pxy_vstream.h"

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_VSTREAM
#define D912PXY_METHOD_IMPL_CN d912pxy_vstream

UINT32 d912pxy_vstream::threadedCtor = 0;

d912pxy_vstream::d912pxy_vstream(UINT Length, DWORD Usage, DWORD fmt, DWORD isIB) : d912pxy_resource(isIB ? RTID_IBUF : RTID_VBUF, PXY_COM_OBJ_VSTREAM, isIB ? L"vstream i" : L"vstream v")
{		
	lockDepth = 0;

	ulObj = NULL;

	PXY_MALLOC(data, Length, void*);

	dx9desc.FVF = 0;
	dx9desc.Pool = D3DPOOL_DEFAULT;
	dx9desc.Format = D3DFMT_UNKNOWN;
	if (isIB)
		dx9desc.Type = D3DRTYPE_INDEXBUFFER;
	else 
		dx9desc.Type = D3DRTYPE_VERTEXBUFFER;
	dx9desc.Size = Length;	
	dx9desc.Usage = Usage;

	NoteFormatChange(fmt, isIB);

	if (!threadedCtor)
		ConstructResource();		
}

d912pxy_vstream * d912pxy_vstream::d912pxy_vstream_com(UINT Length, DWORD Usage, DWORD fmt, DWORD isIB)
{
	d912pxy_com_object* ret = d912pxy_s.com.AllocateComObj(PXY_COM_OBJ_VSTREAM);
	ret->vtable = d912pxy_com_route_get_vtable(PXY_COM_ROUTE_VBUF);

	new (&ret->vstream)d912pxy_vstream(Length, Usage, fmt, isIB);

	return &ret->vstream;
}

d912pxy_vstream::~d912pxy_vstream()
{
	if (GetCurrentPoolSyncValue()) {
		PXY_FREE(data);
	}
}

D912PXY_METHOD_IMPL_NC(Lock)(THIS_ UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags)
{
	API_OVERHEAD_TRACK_START(2)

	d912pxy_vstream_lock_data linfo;
	linfo.dst = this;

	if (!SizeToLock)
	{
		linfo.offset = 0;
		linfo.size = dx9desc.Size - OffsetToLock;
	}
	else {
		linfo.offset = OffsetToLock;
		linfo.size = SizeToLock;
	}

	//megai2: TODO probably thread safety guard
	lockInfo[lockDepth] = linfo;
	++lockDepth;
	
	*ppbData = (void*)((intptr_t)(data) + OffsetToLock);

	API_OVERHEAD_TRACK_END(2)

	return D3D_OK;
}

D912PXY_METHOD_IMPL_NC(Unlock)(THIS)
{
	API_OVERHEAD_TRACK_START(2)
			
	--lockDepth;
	d912pxy_s.thread.bufld.IssueUpload(lockInfo[lockDepth]);	

	API_OVERHEAD_TRACK_END(2)

	return D3D_OK;
}

void d912pxy_vstream::UnlockRanged(UINT newOffset, UINT newSize)
{
	API_OVERHEAD_TRACK_START(2)

	--lockDepth;

	lockInfo[lockDepth].offset = newOffset;
	lockInfo[lockDepth].size = newSize;

	d912pxy_s.thread.bufld.IssueUpload(lockInfo[lockDepth]);

	API_OVERHEAD_TRACK_END(2)
	
}

void d912pxy_vstream::IFrameBindVB(UINT stride, UINT slot, UINT offset, ID3D12GraphicsCommandList * cl)
{	
	if (!m_res)	
		ConstructResource();				

	D3D12_VERTEX_BUFFER_VIEW bindDataLocal;

	bindDataLocal = bindData.v;
	bindDataLocal.StrideInBytes = stride;

	if (offset != 0)
	{						
		bindDataLocal.SizeInBytes -= offset;
		bindDataLocal.BufferLocation += offset;		
	}	

	cl->IASetVertexBuffers(slot, 1, &bindDataLocal);
}

void d912pxy_vstream::IFrameBindIB(ID3D12GraphicsCommandList * cl)
{
	if (!m_res)	
		ConstructResource();
	 
	cl->IASetIndexBuffer(&bindData.i);
}

void d912pxy_vstream::NoteFormatChange(DWORD fmt, DWORD isIB)
{	
	if (isIB)
	{		
		if (fmt == D3DFMT_INDEX16)
			bindData.i.Format = DXGI_FORMAT_R16_UINT;
		else
			bindData.i.Format = DXGI_FORMAT_R32_UINT;

		bindData.i.SizeInBytes = dx9desc.Size;
	}
	else {		
		bindData.v.SizeInBytes = dx9desc.Size;
		bindData.v.StrideInBytes = 0;
	}
}

UINT d912pxy_vstream::FinalReleaseCB()
{
	if (d912pxy_s.pool.vstream.IsRunning())
	{
		EvictFromGPU();

		d912pxy_vstream* tv = this;
		d912pxy_s.pool.vstream.PoolRW(d912pxy_s.pool.vstream.MemCatFromSize(dx9desc.Size), &tv, 1);
		return 0;
	}
	else {
		return 1;
	}
}

UINT32 d912pxy_vstream::PooledAction(UINT32 use)
{
	if (!d912pxy_comhandler::PooledAction(use))
	{		
		if (use)
			MakeGPUResident();

		return 0;
	}

	if (use)
	{		
		if (!threadedCtor)
			ConstructResource();

		PXY_MALLOC(data, dx9desc.Size, void*);
	}
	else {
		if (m_res)
		{
			m_res->Release();
			m_res = NULL;
		}

		PXY_FREE(data);
	}

	PooledActionExit();

	return 1;
}

void d912pxy_vstream::ProcessUpload(d912pxy_vstream_lock_data* linfo, ID3D12GraphicsCommandList * cl)
{			
	if (!m_res)
		ConstructResource();

	if (!ulObj)
	{
		BTransitTo(0, D3D12_RESOURCE_STATE_COPY_DEST, cl);
		ulObj = d912pxy_s.pool.upload.GetUploadObject(dx9desc.Size);
		d912pxy_s.thread.bufld.AddToFinishList(this);
	}
	
	UploadDataCopy(ulObj->DPtr() + linfo->offset, linfo->offset, linfo->size);
	
	ulObj->UploadTargetWithOffset(this, linfo->offset, linfo->offset, linfo->size, cl);	
}

void d912pxy_vstream::FinishUpload(ID3D12GraphicsCommandList * cl)
{
	BTransitTo(0, D3D12_RESOURCE_STATE_GENERIC_READ, cl);
	ulObj->Release();
	ulObj = NULL;
}

void d912pxy_vstream::ConstructResource()
{
	ctorSync.Hold();

	if (m_res)
	{
		ctorSync.Release();
		return;
	}

	ID3D12Resource* tmpLocation = d12res_buffer_target(dx9desc.Size, D3D12_HEAP_TYPE_DEFAULT);
	bindData.i.BufferLocation = tmpLocation->GetGPUVirtualAddress();

	m_res = tmpLocation;

	ctorSync.Release();
}

UINT d912pxy_vstream::GetLength()
{
	return dx9desc.Size;
}

void d912pxy_vstream::UploadDataCopy(intptr_t ulMem, UINT32 offset, UINT32 size)
{
	memcpy((void*)ulMem, (void*)((intptr_t)data + offset), size);	
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE
#undef D912PXY_METHOD_IMPL_CN
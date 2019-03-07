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

d912pxy_resource::d912pxy_resource(d912pxy_device * dev, d912pxy_resource_typeid type, const wchar_t * cat) : d912pxy_comhandler(dev, cat)
{
	m_tid = type;
	swapRef = 0;
	evicted = 0;

	uploadRes[0] = 0;
	uploadRes[1] = 0;
}

d912pxy_resource::~d912pxy_resource()
{
	if (uploadRes[0])
		uploadRes[0]->Release();

	if (uploadRes[1])
		uploadRes[1]->Release();
}

HRESULT d912pxy_resource::QueryInterface(REFIID riid, void ** ppvObj)
{
	return d912pxy_comhandler::QueryInterface(riid, ppvObj);
}

ULONG d912pxy_resource::AddRef(void)
{
	return d912pxy_comhandler::AddRef();
}

ULONG d912pxy_resource::Release(void)
{
	return d912pxy_comhandler::Release();
}

HRESULT d912pxy_resource::GetDevice(IDirect3DDevice9 ** ppDevice)
{
	return d912pxy_noncom::GetDevice(ppDevice);
}

HRESULT d912pxy_resource::SetPrivateData(REFGUID refguid, CONST void * pData, DWORD SizeOfData, DWORD Flags)
{
	LOG_DBG_DTDM(__FUNCTION__);
	return E_NOTIMPL;
}

HRESULT d912pxy_resource::GetPrivateData(REFGUID refguid, void * pData, DWORD * pSizeOfData)
{
	LOG_DBG_DTDM(__FUNCTION__);
	return E_NOTIMPL;
}

HRESULT d912pxy_resource::FreePrivateData(REFGUID refguid)
{
	LOG_DBG_DTDM(__FUNCTION__);
	return E_NOTIMPL;
}

DWORD d912pxy_resource::SetPriority(DWORD PriorityNew)
{
	LOG_DBG_DTDM(__FUNCTION__);
	return D3D_OK;
}

DWORD d912pxy_resource::GetPriority()
{
	LOG_DBG_DTDM(__FUNCTION__);
	return 0;
}

void d912pxy_resource::PreLoad()
{
	LOG_DBG_DTDM(__FUNCTION__);
}

D3DRESOURCETYPE d912pxy_resource::GetType()
{
	LOG_DBG_DTDM(__FUNCTION__);
	return D3DRTYPE_SURFACE;
}

HRESULT d912pxy_resource::d12res_zbuf(DXGI_FORMAT fmt, float clearV, UINT width, UINT height, DXGI_FORMAT clearVFmt)
{
	D3D12_CLEAR_VALUE optimizedClearValue = {};
	optimizedClearValue.Format = clearVFmt;

	optimizedClearValue.DepthStencil = { clearV, 0 };

	D3D12_HEAP_PROPERTIES rhCfg = m_dev->GetResourceHeap(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC rsDesc;

	rsDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rsDesc.Alignment = 0;
	rsDesc.Width = width;
	rsDesc.Height = height;
	rsDesc.DepthOrArraySize = 1;
	rsDesc.MipLevels = 1;
	rsDesc.SampleDesc.Count = 1;
	rsDesc.SampleDesc.Quality = 0;
	rsDesc.Format = fmt;
	rsDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rsDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	LOG_ERR_THROW(d912pxy_s(DXDev)->CreateCommittedResource(
		&rhCfg,
		D3D12_HEAP_FLAG_NONE,
		&rsDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&optimizedClearValue,
		IID_PPV_ARGS(&m_res)
	));

	stateCache = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	//m_dev->GetReplayObj()->DSClear((d912pxy_surface*)this, clearV, 0, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL);

	return D3D_OK;
}

HRESULT d912pxy_resource::d12res_tex2d(UINT width, UINT height, DXGI_FORMAT fmt, UINT16* levels, UINT arrSz)
{
	D3D12_HEAP_PROPERTIES rhCfg = m_dev->GetResourceHeap(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC rsDesc;

	rsDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rsDesc.Alignment = 0;
	rsDesc.Width = width;
	rsDesc.Height = height;
	rsDesc.DepthOrArraySize = arrSz;
	rsDesc.MipLevels = *levels;
	rsDesc.SampleDesc.Count = 1;
	rsDesc.SampleDesc.Quality = 0;
	rsDesc.Format = fmt;
	rsDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rsDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	HRESULT hr = d912pxy_s(DXDev)->CreateCommittedResource(
		&rhCfg,
		D3D12_HEAP_FLAG_NONE,
		&rsDesc,
		D3D12_RESOURCE_STATE_COMMON,
		NULL,
		IID_PPV_ARGS(&m_res)
	);

	if (hr != S_OK)
	{
		m_log->P7_ERROR(LGC_DEFAULT, TM("d12res_tex2d w %u h %u arsz %u lvls %u fmt %u"), width, height, arrSz, *levels, fmt);
		LOG_ERR_THROW2(hr, "texture object create failed");
	}

	m_res->SetName(L"texture obj");

	stateCache = D3D12_RESOURCE_STATE_COMMON;

	LOG_DBG_DTDM("mip levels after miplevels = 0 => %u", m_res->GetDesc().MipLevels);

	*levels = m_res->GetDesc().MipLevels;

	return D3D_OK;
}

HRESULT d912pxy_resource::d12_buffer_mapnwrite(UINT64 offset, UINT64 rowPitch, UINT64 height, UINT64 size, void * memsrc2, const D3D12_RANGE* wofs)
{
	intptr_t bufferRef;
	intptr_t memsrc = (intptr_t)memsrc2;

	LOG_ERR_THROW(m_res->Map(0, NULL, (void**)&bufferRef));

	bufferRef += offset;

	//megai2: what a fucking MESS
	for (int i = 0; i != height; ++i)
	{
		memcpy((void*)bufferRef, (void*)memsrc, size);

		bufferRef += rowPitch;
		memsrc += size;
	}

	m_res->Unmap(0, wofs);

	return E_NOTIMPL;
}

void d912pxy_resource::UpdateDescCache()
{
	descCache = m_res->GetDesc();
	
	switch (descCache.Dimension)
	{
	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		subresCountCache = descCache.DepthOrArraySize * descCache.MipLevels;
		break;

	case D3D12_RESOURCE_DIMENSION_BUFFER:
		subresCountCache = descCache.DepthOrArraySize * descCache.MipLevels * descCache.Height;
		break;

	default:
		subresCountCache = 1;
	}
	
	subresFootprints = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)malloc(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT)*subresCountCache);
	subresSizes = (size_t*)malloc(sizeof(size_t)*subresCountCache);

	d912pxy_s(DXDev)->GetCopyableFootprints(
		&m_res->GetDesc(),
		0,
		subresCountCache,
		0,
		subresFootprints,
		NULL,
		NULL,
		subresSizes
	);
}

void d912pxy_resource::EvictFromGPU()
{
	if (!evicted)
	{
		ID3D12Pageable* mr = (ID3D12Pageable*)m_res.Get();
		d912pxy_helper::ThrowIfFailed(d912pxy_s(DXDev)->Evict(1, &mr), "Evict");
		evicted = 1;
	}
}

void d912pxy_resource::MakeGPUResident()
{
	if (!evicted)
		return;

	ID3D12Pageable* mr = (ID3D12Pageable*)m_res.Get();
	d912pxy_s(DXDev)->MakeResident(1, &mr);

	evicted = 0;
}

void d912pxy_resource::AsyncBufferCopy(d912pxy_upload_item* src, UINT offset, UINT size, ID3D12GraphicsCommandList* cl)
{
	MakeGPUResident();

	IFrameBarrierTrans(0, D3D12_RESOURCE_STATE_COPY_DEST, CLG_BUF);
	//memcpy((void*)((intptr_t)mappedMem[uploadResSel] + offset), (void*)((intptr_t)bufferContent + offset), size);
	//cl->CopyBufferRegion(m_res.Get(), offset, src->GetD12Obj().Get(), offset, size);
	src->UploadTarget(m_res.Get(), offset, size, cl);

	IFrameBarrierTrans(0, D3D12_RESOURCE_STATE_GENERIC_READ, CLG_BUF);	
	++swapRef;
	ThreadRef(-1);
}

void d912pxy_resource::AsyncBufferCopyPrepared(d912pxy_upload_item * src, UINT offset, UINT size, ID3D12GraphicsCommandList * cl)
{
	//IFrameBarrierTrans(0, D3D12_RESOURCE_STATE_COPY_DEST, CLG_BUF);

	src->UploadTarget(m_res.Get(), offset, size, cl);

	//IFrameBarrierTrans(0, D3D12_RESOURCE_STATE_GENERIC_READ, CLG_BUF);
	
}

void d912pxy_resource::IFrameBarrierTrans(UINT subres, D3D12_RESOURCE_STATES to, d912pxy_gpu_cmd_list_group id)
{
	if (to == stateCache)
		return;

	ComPtr<ID3D12GraphicsCommandList> cq = d912pxy_s(GPUcl)->GID(id);

	D3D12_RESOURCE_BARRIER bar;

	bar.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	bar.Transition.pResource = m_res.Get();
	bar.Transition.StateBefore = stateCache;
	bar.Transition.StateAfter = to;
	bar.Transition.Subresource = subres;
	bar.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	LOG_DBG_DTDM("dbarrier %016llX to %u from %u", m_res.Get(), to, stateCache);

	stateCache = to;	

	cq->ResourceBarrier(1, &bar);
}

void d912pxy_resource::IFrameBarrierTrans2(UINT subres, D3D12_RESOURCE_STATES to, D3D12_RESOURCE_STATES from, ID3D12GraphicsCommandList * cl)
{
	D3D12_RESOURCE_BARRIER bar;

	bar.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	bar.Transition.pResource = m_res.Get();
	bar.Transition.StateBefore = from;
	bar.Transition.StateAfter = to;
	bar.Transition.Subresource = subres;
	bar.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	LOG_DBG_DTDM("rbarrier %016llX to %u from %u", m_res.Get(), to, from);

	cl->ResourceBarrier(1, &bar);	
}

void d912pxy_resource::IFrameBarrierTrans4(UINT subres, D3D12_RESOURCE_STATES to, ID3D12GraphicsCommandList * cl)
{
	if (to == stateCache)
		return;

	D3D12_RESOURCE_BARRIER bar;

	bar.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	bar.Transition.pResource = m_res.Get();
	bar.Transition.StateBefore = stateCache;
	bar.Transition.StateAfter = to;
	bar.Transition.Subresource = subres;
	bar.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	LOG_DBG_DTDM("dbarrier %016llX to %u from %u", m_res.Get(), to, stateCache);

	stateCache = to;

	cl->ResourceBarrier(1, &bar);
}

void d912pxy_resource::IFrameBarrierTrans3(UINT subres, D3D12_RESOURCE_STATES to, D3D12_RESOURCE_STATES from, ID3D12GraphicsCommandList * cl, ComPtr<ID3D12Resource> res)
{
	D3D12_RESOURCE_BARRIER bar;

	bar.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	bar.Transition.pResource = res.Get();
	bar.Transition.StateBefore = from;
	bar.Transition.StateAfter = to;
	bar.Transition.Subresource = subres;
	bar.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	cl->ResourceBarrier(1, &bar);
}

void d912pxy_resource::IFrameTrans(D3D12_RESOURCE_STATES to)
{
	LOG_DBG_DTDM("abarrier %016llX to %u from %u", m_res.Get(), to, stateCache);
	stateCache = to;
}

void d912pxy_resource::CopyTo2(d912pxy_resource * dst, ID3D12GraphicsCommandList * cq)
{
	cq->CopyResource(dst->GetD12Obj(), m_res.Get());
}

void d912pxy_resource::CopyTo3(ComPtr<ID3D12Resource> dst, ID3D12GraphicsCommandList * cq)
{
	cq->CopyResource(dst.Get(), m_res.Get());
}

void d912pxy_resource::CopyTo(d912pxy_resource * dst, UINT srcIsReady, ID3D12GraphicsCommandList* cq)
{
	D3D12_RESOURCE_BARRIER bar[2];

	bar[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	bar[0].Transition.pResource = m_res.Get();
	bar[0].Transition.StateBefore = stateCache;
	bar[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	bar[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	bar[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	bar[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	bar[1].Transition.pResource = dst->GetD12Obj();
	bar[1].Transition.StateBefore = dst->GetCurrentState();
	bar[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	bar[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	bar[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	if (srcIsReady)
		cq->ResourceBarrier(1, &bar[1]);
	else
		cq->ResourceBarrier(2, bar);

	cq->CopyResource(dst->GetD12Obj(), m_res.Get());

	bar[0].Transition.StateAfter = stateCache;
	bar[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	bar[1].Transition.StateAfter = dst->GetCurrentState();
	bar[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;

	if (srcIsReady)
		cq->ResourceBarrier(1, &bar[1]);
	else
		cq->ResourceBarrier(2, bar);

}

void d912pxy_resource::CopyBuffer(d912pxy_resource * dst, UINT srcIsReady, UINT offset, UINT size)
{
	ComPtr<ID3D12GraphicsCommandList> cq = d912pxy_s(GPUcl)->GID(CLG_SEQ);

	D3D12_RESOURCE_BARRIER bar[2];

	bar[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	bar[0].Transition.pResource = m_res.Get();
	bar[0].Transition.StateBefore = stateCache;
	bar[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	bar[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	bar[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	bar[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	bar[1].Transition.pResource = dst->GetD12Obj();
	bar[1].Transition.StateBefore = dst->GetCurrentState();
	bar[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	bar[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	bar[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	if (srcIsReady)
		cq->ResourceBarrier(1, &bar[1]);
	else
		cq->ResourceBarrier(2, bar);

	cq->CopyBufferRegion(dst->GetD12Obj(), offset, m_res.Get(), offset, size);

	bar[0].Transition.StateAfter = stateCache;
	bar[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	bar[1].Transition.StateAfter = dst->GetCurrentState();
	bar[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;

	if (srcIsReady)
		cq->ResourceBarrier(1, &bar[1]);
	else
		cq->ResourceBarrier(2, bar);
}

HRESULT d912pxy_resource::d12res_rtgt(DXGI_FORMAT fmt, float * clearV, UINT width, UINT height)
{
	D3D12_CLEAR_VALUE optimizedClearValue = {};
	optimizedClearValue.Format = fmt;
	optimizedClearValue.Color[0] = clearV[0];
	optimizedClearValue.Color[1] = clearV[1];
	optimizedClearValue.Color[2] = clearV[2];
	optimizedClearValue.Color[3] = clearV[3];
	//optimizedClearValue.DepthStencil = { clearV, 0 };

	D3D12_HEAP_PROPERTIES rhCfg = m_dev->GetResourceHeap(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC rsDesc;

	rsDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rsDesc.Alignment = 0;
	rsDesc.Width = width;
	rsDesc.Height = height;
	rsDesc.DepthOrArraySize = 1;
	rsDesc.MipLevels = 1;
	rsDesc.SampleDesc.Count = 1;
	rsDesc.SampleDesc.Quality = 0;
	rsDesc.Format = fmt;
	rsDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rsDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	LOG_ERR_THROW(d912pxy_s(DXDev)->CreateCommittedResource(
		&rhCfg,
		D3D12_HEAP_FLAG_NONE,
		&rsDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&optimizedClearValue,
		IID_PPV_ARGS(&m_res)
	));

	stateCache = D3D12_RESOURCE_STATE_RENDER_TARGET;

	return D3D_OK;
}

HRESULT d912pxy_resource::d12res_buffer(size_t size, D3D12_HEAP_TYPE heap)
{
	m_res = nullptr;

	D3D12_HEAP_PROPERTIES rhCfg = m_dev->GetResourceHeap(heap);
	D3D12_RESOURCE_DESC rsDesc;

	rsDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rsDesc.Alignment = 0;
	rsDesc.Width = size;
	rsDesc.Height = 1;
	rsDesc.DepthOrArraySize = 1;
	rsDesc.MipLevels = 1;
	rsDesc.SampleDesc.Count = 1;
	rsDesc.SampleDesc.Quality = 0;
	rsDesc.Format = DXGI_FORMAT_UNKNOWN;
	rsDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	rsDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	
	LOG_ERR_THROW(d912pxy_s(DXDev)->CreateCommittedResource(
		&rhCfg,
		D3D12_HEAP_FLAG_NONE,
		&rsDesc, 
		D3D12_RESOURCE_STATE_GENERIC_READ,
		0,
		IID_PPV_ARGS(&m_res)
	));

	if (heap == D3D12_HEAP_TYPE_UPLOAD)	
		m_res->SetName(L"upload buffer");
	else 
		m_res->SetName(L"vmem buffer");

	stateCache = D3D12_RESOURCE_STATE_GENERIC_READ;

	return D3D_OK;
}

HRESULT d912pxy_resource::d12res_readback_buffer(size_t size)
{
	D3D12_HEAP_PROPERTIES rhCfg = m_dev->GetResourceHeap(D3D12_HEAP_TYPE_READBACK);
	D3D12_RESOURCE_DESC rsDesc;

	rsDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rsDesc.Alignment = 0;
	rsDesc.Width = size;
	rsDesc.Height = 1;
	rsDesc.DepthOrArraySize = 1;
	rsDesc.MipLevels = 1;	
	rsDesc.SampleDesc.Count = 1;
	rsDesc.SampleDesc.Quality = 0;
	rsDesc.Format = DXGI_FORMAT_UNKNOWN;
	rsDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	rsDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	LOG_ERR_THROW(d912pxy_s(DXDev)->CreateCommittedResource(
		&rhCfg,
		D3D12_HEAP_FLAG_NONE,
		&rsDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		0,
		IID_PPV_ARGS(&m_res)
	));

	stateCache = D3D12_RESOURCE_STATE_COPY_DEST;

	return D3D_OK;
}

HRESULT d912pxy_resource::d12res_uav_buffer(size_t size, D3D12_HEAP_TYPE heap)
{
	D3D12_HEAP_PROPERTIES rhCfg = m_dev->GetResourceHeap(heap);
	D3D12_RESOURCE_DESC rsDesc;

	rsDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rsDesc.Alignment = 0;
	rsDesc.Width = size;
	rsDesc.Height = 1;
	rsDesc.DepthOrArraySize = 1;
	rsDesc.MipLevels = 1;
	rsDesc.SampleDesc.Count = 1;
	rsDesc.SampleDesc.Quality = 0;
	rsDesc.Format = DXGI_FORMAT_UNKNOWN;
	rsDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	rsDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	LOG_ERR_THROW(d912pxy_s(DXDev)->CreateCommittedResource(
		&rhCfg,
		D3D12_HEAP_FLAG_NONE,
		&rsDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		0,
		IID_PPV_ARGS(&m_res)
	));

	stateCache = D3D12_RESOURCE_STATE_GENERIC_READ;

	return D3D_OK;
}

void d912pxy_resource::IFrameEndRefSwap()
{	
#ifdef UPLOAD_POOL_USE_AND_DISCARD
	//megai2: this code is not thread safe on its own, but looks like dx9 app's multithreading model involves lock operations sync at the end of the frame
	//so if this is true, this code will be safe to call based on app logic 
	if (uploadRes[uploadResSel])
	{
		uploadRes[uploadResSel]->Release();
		uploadRes[uploadResSel] = 0;
	}
#endif

	uploadResSel = !uploadResSel;
	swapRef = 0;
	ClearDirtyFlag();	
}

intptr_t d912pxy_resource::GetVA_GPU()
{
	return m_res->GetGPUVirtualAddress();
}

void d912pxy_resource::CreateUploadBuffer(UINT id, UINT size)
{
	uploadRes[id] = d912pxy_s(pool_upload)->GetUploadObject(size);

	mappedMem[id] = uploadRes[id]->MapDPtr();
}

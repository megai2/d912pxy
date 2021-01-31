/*
MIT License

Copyright(c) 2018-2020 megai2

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

UINT d912pxy_resource::residencyOverride = 0;

d912pxy_resource::d912pxy_resource(d912pxy_resource_typeid type, d912pxy_com_obj_typeid tid, const wchar_t * cat) : d912pxy_comhandler(tid, cat)
{
	m_tid = type;
	evicted = residencyOverride;
	stateCache = D3D12_RESOURCE_STATE_COMMON;
	m_res = NULL;
}

d912pxy_resource::~d912pxy_resource()
{
	if (m_res)
		m_res->Release();
}

#define D912PXY_METHOD_IMPL_CN d912pxy_resource

D912PXY_METHOD_IMPL_NC_(D3DRESOURCETYPE, GetType)(THIS)
{	
	return (D3DRESOURCETYPE)m_tid;
}

#undef D912PXY_METHOD_IMPL_CN

HRESULT d912pxy_resource::d12res_zbuf(DXGI_FORMAT fmt, float clearV, UINT width, UINT height, DXGI_FORMAT clearVFmt)
{
	D3D12_CLEAR_VALUE optimizedClearValue = { clearVFmt, { clearV, 0 } };

	D3D12_HEAP_PROPERTIES rhCfg = d912pxy_s.dev.GetResourceHeap(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RESOURCE_DESC rsDesc = { 
		D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0, 
		width, height, 1, 1, 
		fmt, {1, 0}, 
		D3D12_TEXTURE_LAYOUT_UNKNOWN, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL 
	};

	m_res = d912pxy_s.pool.rtds.GetPlacedSurface(&rsDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	if (!m_res)
		LOG_ERR_THROW(d912pxy_s.dx12.dev->CreateCommittedResource(
			&rhCfg,
			D3D12_HEAP_FLAG_NONE,
			&rsDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&optimizedClearValue,
			IID_PPV_ARGS(&m_res)
		));

	LOG_DX_SET_NAME(m_res, L"z buffer");

	stateCache = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	return D3D_OK;
}

ID3D12Resource* d912pxy_resource::d12res_tex2d_target(UINT width, UINT height, DXGI_FORMAT fmt, UINT16* levels, UINT arrSz)
{
	ID3D12Resource* ret = 0;

	D3D12_HEAP_PROPERTIES rhCfg = d912pxy_s.dev.GetResourceHeap(D3D12_HEAP_TYPE_DEFAULT);
	
	D3D12_RESOURCE_DESC rsDesc = {
		D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0,
		width, height, (UINT16)arrSz, *levels,
		fmt, {1, 0},
		D3D12_TEXTURE_LAYOUT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE
	};

	if (*levels != 0)
		ret = d912pxy_s.pool.surface.GetPlacedSurface(&rsDesc, D3D12_RESOURCE_STATE_COMMON);

	HRESULT hr = !ret
		? d912pxy_s.dx12.dev->CreateCommittedResource(
				&rhCfg,
				D3D12_HEAP_FLAG_NONE,
				&rsDesc,
				D3D12_RESOURCE_STATE_COMMON,
				NULL,
				IID_PPV_ARGS(&ret))
		: S_OK;

	if (hr != S_OK)
	{
		LOG_ERR_DTDM("d12res_tex2d w %u h %u arsz %u lvls %u fmt %u", width, height, arrSz, *levels, fmt);
		LOG_ERR_THROW2(hr, "texture object create failed");
	}

	LOG_DX_SET_NAME(ret, L"texture obj");

	stateCache = D3D12_RESOURCE_STATE_COMMON;

	LOG_DBG_DTDM("mip levels after miplevels = 0 => %u", ret->GetDesc().MipLevels);

	*levels = ret->GetDesc().MipLevels;

	return ret;
}

void d912pxy_resource::EvictFromGPU()
{
	if (evicted || !m_res)
		return;

	FRAME_METRIC_RESIDENCY(1)

	ID3D12Pageable* mr = (ID3D12Pageable*)m_res;
	d912pxy_helper::ThrowIfFailed(d912pxy_s.dx12.dev->Evict(1, &mr), "Evict");
	evicted = 1;	

	FRAME_METRIC_RESIDENCY(0)
}

void d912pxy_resource::MakeGPUResident()
{
	if ((evicted != 1) || !m_res)
		return;

	FRAME_METRIC_RESIDENCY(1)

	ID3D12Pageable* mr = (ID3D12Pageable*)m_res;
	d912pxy_helper::ThrowIfFailed(d912pxy_s.dx12.dev->MakeResident(1, &mr), "MakeResident");
	evicted = 0;

	FRAME_METRIC_RESIDENCY(0)
}

void d912pxy_resource::BTransitGID(UINT subres, D3D12_RESOURCE_STATES to, d912pxy_gpu_cmd_list_group id)
{
	if (to == stateCache)
		return;

	ID3D12GraphicsCommandList* cl = d912pxy_s.dx12.cl->GID(id);

	BTransitTo(subres, to, cl);
}

void d912pxy_resource::BTransit(UINT subres, D3D12_RESOURCE_STATES to, D3D12_RESOURCE_STATES from, ID3D12GraphicsCommandList * cl)
{
	D3D12_RESOURCE_BARRIER bar = {
		D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE,
		{m_res, subres, from, to}
	};

	LOG_DBG_DTDM("rbarrier %016llX to %u from %u", m_res, to, from);

	cl->ResourceBarrier(1, &bar);
}

void d912pxy_resource::BTransitTo(UINT subres, D3D12_RESOURCE_STATES to, ID3D12GraphicsCommandList * cl)
{
	if (to == stateCache)
		return;

	BTransit(subres, to, stateCache, cl);

	stateCache = to;
}

void d912pxy_resource::BTransitOOC(UINT subres, D3D12_RESOURCE_STATES to, D3D12_RESOURCE_STATES from, ID3D12GraphicsCommandList * cl, ID3D12Resource* res)
{
	D3D12_RESOURCE_BARRIER bar = {
		D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE,
		{res, subres, from, to}
	};

	cl->ResourceBarrier(1, &bar);
}

void d912pxy_resource::ATransit(D3D12_RESOURCE_STATES to)
{
	stateCache = to;
}

HRESULT d912pxy_resource::d12res_rtgt(DXGI_FORMAT fmt, float * clearV, UINT width, UINT height)
{
	D3D12_CLEAR_VALUE optimizedClearValue = { fmt, {clearV[0], clearV[1], clearV[2], clearV[3]} };

	D3D12_HEAP_PROPERTIES rhCfg = d912pxy_s.dev.GetResourceHeap(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RESOURCE_DESC rsDesc = {
		D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0,
		width, height, 1, 1,
		fmt, {1, 0},
		D3D12_TEXTURE_LAYOUT_UNKNOWN, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	};
	
	m_res = d912pxy_s.pool.rtds.GetPlacedSurface(&rsDesc, D3D12_RESOURCE_STATE_RENDER_TARGET);

	if (!m_res)
		LOG_ERR_THROW(d912pxy_s.dx12.dev->CreateCommittedResource(
			&rhCfg,
			D3D12_HEAP_FLAG_NONE,
			&rsDesc,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			&optimizedClearValue,
			IID_PPV_ARGS(&m_res)
		));

	LOG_DX_SET_NAME(m_res, L"render target");

	stateCache = D3D12_RESOURCE_STATE_RENDER_TARGET;

	return D3D_OK;
}

ID3D12Resource * d912pxy_resource::d12res_buffer_target(size_t size, D3D12_HEAP_TYPE heap)
{
	D3D12_HEAP_PROPERTIES rhCfg = d912pxy_s.dev.GetResourceHeap(heap);

	D3D12_RESOURCE_DESC rsDesc = {
		D3D12_RESOURCE_DIMENSION_BUFFER, 0,
		size, 1, 1, 1,
		DXGI_FORMAT_UNKNOWN, {1, 0},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE
	};

	ID3D12Resource* ret;

	LOG_ERR_THROW(d912pxy_s.dx12.dev->CreateCommittedResource(
		&rhCfg,
		D3D12_HEAP_FLAG_NONE,
		&rsDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		0,
		IID_PPV_ARGS(&ret)
	));

	if (heap == D3D12_HEAP_TYPE_UPLOAD)
		LOG_DX_SET_NAME(ret, L"upload buffer");
	else
		LOG_DX_SET_NAME(ret, L"vmem buffer");

	stateCache = D3D12_RESOURCE_STATE_GENERIC_READ;

	return ret;
}

HRESULT d912pxy_resource::d12res_buffer(size_t size, D3D12_HEAP_TYPE heap)
{
	m_res = d12res_buffer_target(size, heap);
	return D3D_OK;
}

HRESULT d912pxy_resource::d12res_readback_buffer(size_t size)
{
	D3D12_HEAP_PROPERTIES rhCfg = d912pxy_s.dev.GetResourceHeap(D3D12_HEAP_TYPE_READBACK);
	
	D3D12_RESOURCE_DESC rsDesc = {
		D3D12_RESOURCE_DIMENSION_BUFFER, 0,
		size, 1, 1, 1,
		DXGI_FORMAT_UNKNOWN, {1, 0},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE
	};

	LOG_ERR_THROW(d912pxy_s.dx12.dev->CreateCommittedResource(
		&rhCfg,
		D3D12_HEAP_FLAG_NONE,
		&rsDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		0,
		IID_PPV_ARGS(&m_res)
	));

	LOG_DX_SET_NAME(m_res, L"readback buffer");

	stateCache = D3D12_RESOURCE_STATE_COPY_DEST;

	return D3D_OK;
}

HRESULT d912pxy_resource::d12res_uav_buffer(size_t size, D3D12_HEAP_TYPE heap)
{
	D3D12_HEAP_PROPERTIES rhCfg = d912pxy_s.dev.GetResourceHeap(heap);
	
	D3D12_RESOURCE_DESC rsDesc = {
		D3D12_RESOURCE_DIMENSION_BUFFER, 0,
		size, 1, 1, 1,
		DXGI_FORMAT_UNKNOWN, {1, 0},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	};

	LOG_ERR_THROW(d912pxy_s.dx12.dev->CreateCommittedResource(
		&rhCfg,
		D3D12_HEAP_FLAG_NONE,
		&rsDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		0,
		IID_PPV_ARGS(&m_res)
	));

	LOG_DX_SET_NAME(m_res, L"uav buffer");

	stateCache = D3D12_RESOURCE_STATE_GENERIC_READ;

	return D3D_OK;
}

HRESULT d912pxy_resource::d12res_tex2d(UINT width, UINT height, DXGI_FORMAT fmt, UINT16 * levels, UINT arrSz)
{
	m_res = d12res_tex2d_target(width, height, fmt, levels, arrSz);

	return D3D_OK;
}

void d912pxy_resource::ACopyTo(d912pxy_resource * dst, ID3D12GraphicsCommandList * cl)
{
	cl->CopyResource(dst->GetD12Obj(), m_res);
}

void d912pxy_resource::BCopyTo(d912pxy_resource * dst, UINT barriers, ID3D12GraphicsCommandList * cl)
{
	D3D12_RESOURCE_STATES dstStateCache = dst->GetCurrentState();
	D3D12_RESOURCE_STATES srcStateCache = GetCurrentState();

	BCopyToWStates(dst, barriers, cl, dstStateCache, srcStateCache);
}

void d912pxy_resource::BCopyToWStates(d912pxy_resource * dst, UINT barriers, ID3D12GraphicsCommandList * cl, D3D12_RESOURCE_STATES dstStateCache, D3D12_RESOURCE_STATES srcStateCache)
{
	if ((barriers & 1) && (srcStateCache != D3D12_RESOURCE_STATE_COPY_SOURCE))
		BTransit(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_COPY_SOURCE, srcStateCache, cl);
	else
		barriers &= 0x2;

	if ((barriers & 2) && (dstStateCache != D3D12_RESOURCE_STATE_COPY_DEST))
		dst->BTransit(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_COPY_DEST, dstStateCache, cl);
	else
		barriers &= 0x1;

	ACopyTo(dst, cl);

	if (barriers & 1)
		BTransit(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, srcStateCache, D3D12_RESOURCE_STATE_COPY_SOURCE, cl);

	if (barriers & 2)
		dst->BTransit(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, dstStateCache, D3D12_RESOURCE_STATE_COPY_DEST, cl);
}

UINT64 d912pxy_resource::GetVA_GPU()
{
	return m_res->GetGPUVirtualAddress();
}

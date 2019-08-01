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

d912pxy_dheap::d912pxy_dheap(d912pxy_device * dev, UINT idx) : d912pxy_noncom( L"dheap")
{
	const D3D12_DESCRIPTOR_HEAP_DESC* desc = &d912pxy_dx12_heap_config[idx];

	LOG_ERR_THROW(d912pxy_s.dx12.dev->CreateDescriptorHeap(desc, IID_PPV_ARGS(&heap)));

	handleSz = d912pxy_s.dx12.dev->GetDescriptorHandleIncrementSize(desc->Type);

	cpuBase = heap->GetCPUDescriptorHandleForHeapStart();
	gpuBase = heap->GetGPUDescriptorHandleForHeapStart();

	LOG_INFO_DTDM("DHeap %u limit: %u", idx, desc->NumDescriptors);

	for (int i = 0; PXY_DHEAP_STACK_COUNT != i; ++i)	
		stacks[i] = new d912pxy_dheap_slot_stack(desc->NumDescriptors);	

	for (int i = 0; i != desc->NumDescriptors; ++i)	
		stacks[PXY_DHEAP_STACK_FREE]->Push((desc->NumDescriptors - 1) - i);	
	
	m_desc = desc;

	selfIID = idx;

	LOG_DBG_DTDM("type %u cnt %u id %u", desc->Type, desc->NumDescriptors, selfIID);
}

d912pxy_dheap::~d912pxy_dheap()
{
	for (int i = 0; PXY_DHEAP_STACK_COUNT != i; ++i)
		delete stacks[i];

	heap->Release();
}

UINT d912pxy_dheap::OccupySlot()
{		
	FRAME_METRIC_DHEAP(selfIID, stacks[PXY_DHEAP_STACK_FREE]->Count());
	
	UINT ret = stacks[PXY_DHEAP_STACK_FREE]->Pop();

	LOG_DBG_DTDM("dheap[%u] used slot %u", selfIID, ret);

	return ret;
}

void d912pxy_dheap::FreeSlot(UINT slot)
{
	LOG_DBG_DTDM("dheap[%u] freed slot %u", selfIID, slot);

	stacks[PXY_DHEAP_STACK_CLEANUP]->Push(slot);
}

void d912pxy_dheap::FreeSlotByPtr(D3D12_CPU_DESCRIPTOR_HANDLE cptr)
{
	FreeSlot((UINT32)((cptr.ptr - cpuBase.ptr) / handleSz));
}

void d912pxy_dheap::CleanupSlots(UINT maxCount)
{
	UINT limit = 0;

	while (stacks[PXY_DHEAP_STACK_CLEANUP]->Count())
	{
		stacks[PXY_DHEAP_STACK_FREE]->Push(stacks[PXY_DHEAP_STACK_CLEANUP]->Pop());

		if (limit >= maxCount)
			break;
		else
			++limit;
	}

	FRAME_METRIC_DHEAP(selfIID, stacks[PXY_DHEAP_STACK_FREE]->Count());
}

D3D12_CPU_DESCRIPTOR_HANDLE d912pxy_dheap::GetDHeapHandle(UINT slot)
{
	D3D12_CPU_DESCRIPTOR_HANDLE ret;
	ret.ptr = cpuBase.ptr + slot * handleSz;

	return ret;
}

D3D12_GPU_DESCRIPTOR_HANDLE d912pxy_dheap::GetGPUDHeapHandle(UINT slot)
{
	D3D12_GPU_DESCRIPTOR_HANDLE ret;	
	ret.ptr = gpuBase.ptr + slot * handleSz;

	return ret;
}

UINT d912pxy_dheap::CreateSRV(ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* dsc)
{
	UINT ret = OccupySlot();

	if (dsc)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC constDsc;
		constDsc = *dsc;
		d912pxy_s.dx12.dev->CreateShaderResourceView(resource, &constDsc, GetDHeapHandle(ret));
	}
	else {
		d912pxy_s.dx12.dev->CreateShaderResourceView(resource, NULL, GetDHeapHandle(ret));
	}

	LOG_DBG_DTDM("new SRV @%u = %u", selfIID, ret);

	return ret;
}

UINT d912pxy_dheap::CreateCBV(D3D12_CONSTANT_BUFFER_VIEW_DESC * dsc)
{
	UINT ret = OccupySlot();

	D3D12_CONSTANT_BUFFER_VIEW_DESC constDsc;
	constDsc = *dsc;
	d912pxy_s.dx12.dev->CreateConstantBufferView(&constDsc, GetDHeapHandle(ret));

	LOG_DBG_DTDM("new CBV @%u = %u", selfIID, ret);
	
	return ret;
}

UINT d912pxy_dheap::CreateUAV(D3D12_UNORDERED_ACCESS_VIEW_DESC * dsc, ID3D12Resource* iRes)
{
	UINT ret = OccupySlot();

	D3D12_UNORDERED_ACCESS_VIEW_DESC constDsc;

	constDsc = *dsc;

	d912pxy_s.dx12.dev->CreateUnorderedAccessView(
		iRes,
		0,
		&constDsc,
		GetDHeapHandle(ret)
	);

	LOG_DBG_DTDM("new UAV @%u = %u", selfIID, ret);

	return ret;
}

UINT d912pxy_dheap::CreateSampler(D3D12_SAMPLER_DESC * dsc)
{
	UINT ret = OccupySlot();

	D3D12_SAMPLER_DESC constDsc;
	constDsc = *dsc;
	d912pxy_s.dx12.dev->CreateSampler(&constDsc, GetDHeapHandle(ret));

	LOG_DBG_DTDM("new SPL @%u = %u", selfIID, ret);

	return ret;
}

UINT d912pxy_dheap::CreateRTV(ID3D12Resource* resource, D3D12_RENDER_TARGET_VIEW_DESC* dsc)
{
	UINT ret = OccupySlot();

	if (dsc)
	{
		D3D12_RENDER_TARGET_VIEW_DESC constDsc;
		constDsc = *dsc;
		d912pxy_s.dx12.dev->CreateRenderTargetView(resource, &constDsc, GetDHeapHandle(ret));
	}
	else {
		d912pxy_s.dx12.dev->CreateRenderTargetView(resource, NULL, GetDHeapHandle(ret));
	}

	LOG_DBG_DTDM("new RTV @%u = %u", selfIID, ret);

	return ret;
}

UINT d912pxy_dheap::CreateDSV(ID3D12Resource* resource, D3D12_DEPTH_STENCIL_VIEW_DESC * dsc)
{
	UINT ret = OccupySlot();
	

	if (dsc)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC constDsc;
		constDsc = *dsc;
		d912pxy_s.dx12.dev->CreateDepthStencilView(resource, &constDsc, GetDHeapHandle(ret));
	}
	else {
		d912pxy_s.dx12.dev->CreateDepthStencilView(resource, NULL, GetDHeapHandle(ret));
	}

	LOG_DBG_DTDM("new DSV @%u = %u", selfIID, ret);

	return ret;
}

UINT d912pxy_dheap::CreateSRV_at(ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC * dsc, UINT32 slot)
{
	UINT ret = slot;

	if (dsc)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC constDsc;
		constDsc = *dsc;
		d912pxy_s.dx12.dev->CreateShaderResourceView(resource, &constDsc, GetDHeapHandle(ret));
	}
	else {
		d912pxy_s.dx12.dev->CreateShaderResourceView(resource, NULL, GetDHeapHandle(ret));
	}

	LOG_DBG_DTDM("reusing SRV @%u = %u", selfIID, ret);

	return ret;
}

d912pxy_dheap_slot_stack::d912pxy_dheap_slot_stack(UINT32 size)
{

	PXY_MALLOC(data, size * sizeof(d912pxy_dheap_slot_type), UINT32*);
	top = 0;
}

d912pxy_dheap_slot_stack::~d912pxy_dheap_slot_stack()
{
	PXY_FREE(data);
}

void d912pxy_dheap_slot_stack::Push(d912pxy_dheap_slot_type val)
{
	lock.Hold();
	
	data[top] = val;
	++top;

	lock.Release();
}

d912pxy_dheap_slot_type d912pxy_dheap_slot_stack::Pop()
{
	d912pxy_dheap_slot_type ret = 0;

	lock.Hold();

	LONG idx = --top;

	if (idx < 0)
	{
		d912pxy_helper::ThrowIfFailed(-1, "d912pxy_dheap_slot_stack::Pop()");
	}
	else
		ret = data[idx];

	lock.Release();

	return ret;
}

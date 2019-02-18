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

d912pxy_dheap::d912pxy_dheap(d912pxy_device * dev, UINT idx) : d912pxy_noncom(dev, L"dheap")
{
	const D3D12_DESCRIPTOR_HEAP_DESC* desc = &d912pxy_dx12_heap_config[idx];

	LOG_ERR_THROW(d912pxy_s(DXDev)->CreateDescriptorHeap(desc, IID_PPV_ARGS(&heap)));

	handleSz = d912pxy_s(DXDev)->GetDescriptorHandleIncrementSize(desc->Type);

	cpuBase = heap->GetCPUDescriptorHandleForHeapStart();
	gpuBase = heap->GetGPUDescriptorHandleForHeapStart();

	m_log->P7_INFO(LGC_DEFAULT, TM("DHeap %u limit: %u"), idx, desc->NumDescriptors);

	size_t alcSize = sizeof(UINT32)*desc->NumDescriptors;

	for (int i = 0; PXY_DHEAP_STACK_COUNT != i; ++i)
	{
		stacks[i].data = (UINT32*)malloc(alcSize);
		stacks[i].top = 0;
	}

	for (int i = 0; i != desc->NumDescriptors; ++i)
	{
		stacks[PXY_DHEAP_STACK_FREE].data[i] = (desc->NumDescriptors - 1) - i;
	}

	stacks[PXY_DHEAP_STACK_FREE].top = desc->NumDescriptors;

	m_desc = desc;

	selfIID = idx;

	LOG_DBG_DTDM("type %u cnt %u id %u", desc->Type, slots, selfIID);

	heapStartCache = heap->GetGPUDescriptorHandleForHeapStart();
}

d912pxy_dheap::~d912pxy_dheap()
{
	for (int i = 0; PXY_DHEAP_STACK_COUNT != i; ++i)
		free(stacks[i].data);
}

UINT d912pxy_dheap::OccupySlot()
{		
	LONG stackSlot = InterlockedDecrement(&stacks[PXY_DHEAP_STACK_FREE].top);

	if (stackSlot < 0)	
	{
		m_log->P7_ERROR(LGC_DEFAULT, TM("DHeap %u slot limit exceeded"), selfIID);
		LOG_ERR_THROW2(-1, "dheapslots == 0");
	}

	FRAME_METRIC_DHEAP(selfIID, stackSlot);

	return stacks[PXY_DHEAP_STACK_FREE].data[stackSlot];
}

void d912pxy_dheap::FreeSlot(UINT slot)
{
	LONG stackSlot = InterlockedIncrement(&stacks[PXY_DHEAP_STACK_CLEANUP].top) - 1;

	if (stackSlot >= (LONG)m_desc->NumDescriptors)
	{
		m_log->P7_ERROR(LGC_DEFAULT, TM("DHeap %u imm cleanup limit exceeded"), selfIID);
		LOG_ERR_THROW2(-1, "dheapslots == 0 @ FreeSlot");
	}

	stacks[PXY_DHEAP_STACK_CLEANUP].data[stackSlot] = slot;
}

void d912pxy_dheap::CleanupSlots(UINT maxCount)
{
	//megai2: should be safe to clean whole stack without thread safety, as this is called on "should-be" sync blocked conditions

	LONG stackSlot = InterlockedAdd(&stacks[PXY_DHEAP_STACK_CLEANUP].top, 0);
	UINT limit = 0;

	while (stackSlot)
	{
		stackSlot = InterlockedDecrement(&stacks[PXY_DHEAP_STACK_CLEANUP].top);

		UINT slot = stacks[PXY_DHEAP_STACK_CLEANUP].data[stackSlot];

		LONG writeSlot = InterlockedIncrement(&stacks[PXY_DHEAP_STACK_FREE].top) - 1;

		if (writeSlot >= (LONG)m_desc->NumDescriptors)
		{
			m_log->P7_ERROR(LGC_DEFAULT, TM("DHeap %u free slots limit exceeded"), selfIID);
			LOG_ERR_THROW2(-1, "dheapslots == 0 @ CleanupSlots");
		}

		stacks[PXY_DHEAP_STACK_FREE].data[writeSlot] = slot;

		FRAME_METRIC_DHEAP(selfIID, writeSlot)

		if (limit >= maxCount)
			break;
		else
			++limit;
	}
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
	ret = heapStartCache;
	ret.ptr = ret.ptr + slot * handleSz;
	return ret;
}

UINT d912pxy_dheap::CreateSRV(ComPtr<ID3D12Resource> resource, D3D12_SHADER_RESOURCE_VIEW_DESC* dsc)
{
	UINT ret = OccupySlot();

	if (dsc)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC constDsc;
		constDsc = *dsc;
		d912pxy_s(DXDev)->CreateShaderResourceView(resource.Get(), &constDsc, GetDHeapHandle(ret));
	}
	else {
		d912pxy_s(DXDev)->CreateShaderResourceView(resource.Get(), NULL, GetDHeapHandle(ret));
	}

	LOG_DBG_DTDM("new SRV @%u = %u", selfIID, ret);

	return ret;
}

UINT d912pxy_dheap::CreateCBV(D3D12_CONSTANT_BUFFER_VIEW_DESC * dsc)
{
	UINT ret = OccupySlot();

	D3D12_CONSTANT_BUFFER_VIEW_DESC constDsc;
	constDsc = *dsc;
	d912pxy_s(DXDev)->CreateConstantBufferView(&constDsc, GetDHeapHandle(ret));
	
	return ret;
}

UINT d912pxy_dheap::CreateUAV(D3D12_UNORDERED_ACCESS_VIEW_DESC * dsc, ID3D12Resource* iRes)
{
	UINT ret = OccupySlot();

	D3D12_UNORDERED_ACCESS_VIEW_DESC constDsc;

	constDsc = *dsc;

	d912pxy_s(DXDev)->CreateUnorderedAccessView(
		iRes,
		0,
		&constDsc,
		GetDHeapHandle(ret)
	);

	return ret;
}

UINT d912pxy_dheap::CreateSampler(D3D12_SAMPLER_DESC * dsc)
{
	UINT ret = OccupySlot();

	D3D12_SAMPLER_DESC constDsc;
	constDsc = *dsc;
	d912pxy_s(DXDev)->CreateSampler(&constDsc, GetDHeapHandle(ret));

	return ret;
}

UINT d912pxy_dheap::CreateRTV(ComPtr<ID3D12Resource> resource, D3D12_RENDER_TARGET_VIEW_DESC* dsc)
{
	UINT ret = OccupySlot();

	if (dsc)
	{
		D3D12_RENDER_TARGET_VIEW_DESC constDsc;
		constDsc = *dsc;
		d912pxy_s(DXDev)->CreateRenderTargetView(resource.Get(), &constDsc, GetDHeapHandle(ret));
	}
	else {
		d912pxy_s(DXDev)->CreateRenderTargetView(resource.Get(), NULL, GetDHeapHandle(ret));
	}

	LOG_DBG_DTDM("new RTV @%u = %u", selfIID, ret);

	return ret;
}

UINT d912pxy_dheap::CreateDSV(ComPtr<ID3D12Resource> resource, D3D12_DEPTH_STENCIL_VIEW_DESC * dsc)
{
	UINT ret = OccupySlot();
	

	if (dsc)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC constDsc;
		constDsc = *dsc;
		d912pxy_s(DXDev)->CreateDepthStencilView(resource.Get(), &constDsc, GetDHeapHandle(ret));
	}
	else {
		d912pxy_s(DXDev)->CreateDepthStencilView(resource.Get(), NULL, GetDHeapHandle(ret));
	}

	LOG_DBG_DTDM("new DSV @%u = %u", selfIID, ret);

	return ret;
}

UINT d912pxy_dheap::CreateSRV_at(ComPtr<ID3D12Resource> resource, D3D12_SHADER_RESOURCE_VIEW_DESC * dsc, UINT32 slot)
{
	UINT ret = slot;

	if (dsc)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC constDsc;
		constDsc = *dsc;
		d912pxy_s(DXDev)->CreateShaderResourceView(resource.Get(), &constDsc, GetDHeapHandle(ret));
	}
	else {
		d912pxy_s(DXDev)->CreateShaderResourceView(resource.Get(), NULL, GetDHeapHandle(ret));
	}

	LOG_DBG_DTDM("reusing SRV @%u = %u", selfIID, ret);

	return ret;
}

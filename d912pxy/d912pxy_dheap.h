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

#define d912pxy_dheap_slot_type UINT32

class d912pxy_dheap_slot_stack {

public: 
	d912pxy_dheap_slot_stack(UINT32 size);
	~d912pxy_dheap_slot_stack();

	void Push(d912pxy_dheap_slot_type val);
	d912pxy_dheap_slot_type Pop();

	LONG Count() { return top; };

private:

	d912pxy_dheap_slot_type* data;
	LONG top;

	d912pxy_thread_lock lock;
	d912pxy_mem_mgr memMgr;
};

#define PXY_DHEAP_STACK_FREE 0
#define PXY_DHEAP_STACK_CLEANUP 1
#define PXY_DHEAP_STACK_COUNT 2

class d912pxy_dheap : public d912pxy_noncom
{
public:
	d912pxy_dheap(d912pxy_device* dev, UINT idx);
	~d912pxy_dheap();

	UINT OccupySlot();
	void FreeSlot(UINT slot);	
	void FreeSlotByPtr(D3D12_CPU_DESCRIPTOR_HANDLE cptr);

	void CleanupSlots(UINT maxCount);

	D3D12_CPU_DESCRIPTOR_HANDLE GetDHeapHandle(UINT slot);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDHeapHandle(UINT slot);

	UINT CreateRTV(ID3D12Resource* resource, D3D12_RENDER_TARGET_VIEW_DESC* dsc);
	UINT CreateDSV(ID3D12Resource* resource, D3D12_DEPTH_STENCIL_VIEW_DESC* dsc);
	UINT CreateSRV_at(ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* dsc, UINT32 slot);
	UINT CreateSRV(ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* dsc);
	UINT CreateCBV(D3D12_CONSTANT_BUFFER_VIEW_DESC* dsc);
	UINT CreateUAV(D3D12_UNORDERED_ACCESS_VIEW_DESC* dsc, ID3D12Resource* iRes);
	UINT CreateSampler(D3D12_SAMPLER_DESC* dsc);

	ID3D12DescriptorHeap* GetHeapObj() { 
		return heap; 
	};

	const D3D12_DESCRIPTOR_HEAP_DESC* GetDesc() {
		return m_desc;
	}

private:
	UINT selfIID;
	
	//megai2: slot status filled in stacks
	d912pxy_dheap_slot_stack* stacks[PXY_DHEAP_STACK_COUNT];
	
	ID3D12DescriptorHeap* heap;

	const D3D12_DESCRIPTOR_HEAP_DESC* m_desc;
	UINT handleSz;	
	
	D3D12_CPU_DESCRIPTOR_HANDLE cpuBase;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuBase;

};


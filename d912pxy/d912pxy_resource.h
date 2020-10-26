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
#pragma once
#include "stdafx.h"

#define PXY_INNER_MAX_LOCK_DEPTH 4

class d912pxy_device;

typedef struct d912pxy_resource_ptr {
	intptr_t host;
	UINT64 dev;
} d912pxy_resource_ptr;

enum d912pxy_resource_typeid {
	RTID_SURFACE = 1,
	RTID_VOLUME = 2,
	RTID_TEX = 3,
	RTID_VTEX = 4,
	RTID_CTEX = 5,
	RTID_VBUF = 6,
	RTID_IBUF = 7,	
	RTID_UL_BUF,
	RTID_CBUFFER,
	RTID_RB_BUF,
	RTID_UNK
};

class d912pxy_resource : public d912pxy_comhandler
{
public:
	d912pxy_resource(d912pxy_resource_typeid type, d912pxy_com_obj_typeid tid, const wchar_t* cat);
	~d912pxy_resource();

	
	/*** IDirect3DResource9 methods ***/
	D912PXY_METHOD(SetPrivateData)(PXY_THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	D912PXY_METHOD(GetPrivateData)(PXY_THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData);
	D912PXY_METHOD(FreePrivateData)(PXY_THIS_ REFGUID refguid);
	D912PXY_METHOD_(DWORD, SetPriority)(PXY_THIS_ DWORD PriorityNew);
	D912PXY_METHOD_(DWORD, GetPriority)(PXY_THIS);
	D912PXY_METHOD_(void, PreLoad)(PXY_THIS);
	D912PXY_METHOD_(D3DRESOURCETYPE, GetType)(PXY_THIS);

	D912PXY_METHOD_NC_(D3DRESOURCETYPE, GetType)(THIS);

	/////////////////////

	HRESULT	d12res_zbuf(DXGI_FORMAT fmt, float clearV, UINT width, UINT height, DXGI_FORMAT clearVFmt);
	HRESULT	d12res_rtgt(DXGI_FORMAT fmt, float* clearV, UINT width, UINT height);
	ID3D12Resource*	d12res_buffer_target(size_t size, D3D12_HEAP_TYPE heap);
	HRESULT	d12res_buffer(size_t size, D3D12_HEAP_TYPE heap);
	HRESULT	d12res_readback_buffer(size_t size);
	HRESULT	d12res_uav_buffer(size_t size, D3D12_HEAP_TYPE heap);
	HRESULT d12res_tex2d(UINT width, UINT height, DXGI_FORMAT fmt, UINT16* levels, UINT arrSz);
	ID3D12Resource*	d12res_tex2d_target(UINT width, UINT height, DXGI_FORMAT fmt, UINT16* levels, UINT arrSz);

	void EvictFromGPU();
	void MakeGPUResident();

	ID3D12Resource* GetD12Obj() { return m_res; };
			
	void BTransitGID(UINT subres, D3D12_RESOURCE_STATES to, d912pxy_gpu_cmd_list_group id);
	void BTransit(UINT subres, D3D12_RESOURCE_STATES to, D3D12_RESOURCE_STATES from, ID3D12GraphicsCommandList* cl);
	void BTransitTo(UINT subres, D3D12_RESOURCE_STATES to, ID3D12GraphicsCommandList* cl);
	static void BTransitOOC(UINT subres, D3D12_RESOURCE_STATES to, D3D12_RESOURCE_STATES from, ID3D12GraphicsCommandList * cl, ID3D12Resource* res);

	void ATransit(D3D12_RESOURCE_STATES to);

	D3D12_RESOURCE_STATES GetCurrentState() {
		return stateCache;
	};
	
	void ACopyTo(d912pxy_resource* dst, ID3D12GraphicsCommandList* cl);
	void BCopyTo(d912pxy_resource* dst, UINT barriers, ID3D12GraphicsCommandList* cq);
	void BCopyToWStates(d912pxy_resource* dst, UINT barriers, ID3D12GraphicsCommandList* cq, D3D12_RESOURCE_STATES dstStateCache, D3D12_RESOURCE_STATES srcStateCache);
		
	UINT64 GetVA_GPU();
	
	static UINT residencyOverride;

private: 
	d912pxy_resource_typeid m_tid;

protected:
	ID3D12Resource* m_res;
	
	d912pxy_dheap* dHeap=nullptr;

	D3D12_RESOURCE_STATES stateCache;	

	UINT8 evicted;

	d912pxy_thread_lock ctorSync;
};


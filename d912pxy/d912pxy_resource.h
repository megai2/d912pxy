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

#define PXY_INNER_MAX_LOCK_DEPTH 4

class d912pxy_device;

enum d912pxy_resource_typeid {
	RTID_TEXTURE,
	RTID_VBUF,
	RTID_IBUF,
	RTID_SURFACE,
	RTID_UL_BUF,
	RTID_CBUFFER,
	RTID_RB_BUF,
	RTID_UNK
};

class d912pxy_resource : public IDirect3DResource9, public d912pxy_comhandler
{
public:
	d912pxy_resource(d912pxy_device* dev, d912pxy_resource_typeid type, const wchar_t* cat);
	~d912pxy_resource();

	HRESULT WINAPI QueryInterface(REFIID riid, void** ppvObj);
	ULONG WINAPI AddRef(void);
	ULONG WINAPI Release(void);

	/*** IDirect3DResource9 methods ***/
	HRESULT WINAPI GetDevice(IDirect3DDevice9** ppDevice);
	HRESULT WINAPI SetPrivateData(REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	HRESULT WINAPI GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData);
	HRESULT WINAPI FreePrivateData(REFGUID refguid);
	DWORD WINAPI SetPriority(DWORD PriorityNew);
	DWORD WINAPI GetPriority();
	void WINAPI PreLoad();
	D3DRESOURCETYPE WINAPI GetType();

	HRESULT	d12res_zbuf(DXGI_FORMAT fmt, float clearV, UINT width, UINT height, DXGI_FORMAT clearVFmt);
	HRESULT	d12res_rtgt(DXGI_FORMAT fmt, float* clearV, UINT width, UINT height);
	HRESULT	d12res_buffer(size_t size, D3D12_HEAP_TYPE heap);
	HRESULT	d12res_readback_buffer(size_t size);
	HRESULT	d12res_uav_buffer(size_t size, D3D12_HEAP_TYPE heap);
	HRESULT d12res_tex2d(UINT width, UINT height, DXGI_FORMAT fmt, UINT16* levels, UINT arrSz);

	HRESULT d12_buffer_mapnwrite(UINT64 offset, UINT64 rowPitch, UINT64 height, UINT64 size, void* memsrc2, const D3D12_RANGE* wofs);

	void UpdateDescCache();

	void EvictFromGPU();
	void MakeGPUResident();

	ComPtr<ID3D12Resource> GetD12Obj() { return m_res; };

	void AsyncBufferCopy(d912pxy_upload_item* src, UINT offset, UINT size, ID3D12GraphicsCommandList* cl);
	void AsyncBufferCopyPrepared(d912pxy_upload_item* src, UINT offset, UINT size, ID3D12GraphicsCommandList* cl);
	
	void IFrameBarrierTrans(UINT subres, D3D12_RESOURCE_STATES to, d912pxy_gpu_cmd_list_group id);
	void IFrameBarrierTrans2(UINT subres, D3D12_RESOURCE_STATES to, D3D12_RESOURCE_STATES from, ID3D12GraphicsCommandList* cl);
	static void IFrameBarrierTrans3(UINT subres, D3D12_RESOURCE_STATES to, D3D12_RESOURCE_STATES from, ID3D12GraphicsCommandList * cl, ComPtr<ID3D12Resource> res);
	void IFrameTrans(D3D12_RESOURCE_STATES to);

	D3D12_RESOURCE_STATES GetCurrentState() {
		return stateCache;
	};

	void CopyTo2(d912pxy_resource* dst, ID3D12GraphicsCommandList* cq);
	void CopyTo3(ComPtr<ID3D12Resource> dst, ID3D12GraphicsCommandList* cq);
	
	void CopyTo(d912pxy_resource* dst, UINT srcIsReady, ID3D12GraphicsCommandList* cq);
	void CopyBuffer(d912pxy_resource* dst, UINT srcIsReady, UINT offset, UINT size);
	
	void ClearDirtyFlag() { opFlags = 0; };

	void IFrameEndRefSwap();
	UINT IsFirstFrameUploadRef() { return swapRef; };

	virtual void CreateUploadBuffer(UINT id, UINT size);

private: 
	d912pxy_resource_typeid m_tid;

protected:
	ComPtr<ID3D12Resource> m_res;
	
	d912pxy_upload_item* uploadRes[2];
	d912pxy_dheap* dHeap;

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT* subresFootprints;
	size_t* subresSizes;

	UINT subresCountCache;
	D3D12_RESOURCE_DESC descCache;
	D3D12_RESOURCE_STATES stateCache;	

	void* bufferContent;
	void* mappedMem[2];
	
	UINT8 swapRef;
	UINT8 opFlags;
	UINT8 uploadResSel;
	UINT8 evicted;
};


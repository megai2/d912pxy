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

class d912pxy_surface : public IDirect3DSurface9, public d912pxy_resource
{
public:
	d912pxy_surface(d912pxy_device* dev, UINT Width, UINT Height, D3DFORMAT Format, DWORD Usage, UINT* levels, UINT arrSz, UINT32* srvFeedback);
	d912pxy_surface(d912pxy_device* dev, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, INT surfType);	
	~d912pxy_surface();

	/*** IUnknown methods ***/
	D912PXY_METHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	D912PXY_METHOD_(ULONG, AddRef)(THIS);
	D912PXY_METHOD_(ULONG, Release)(THIS);

	/*** IDirect3DResource9 methods ***/
	D912PXY_METHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice);
	D912PXY_METHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	D912PXY_METHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData);
	D912PXY_METHOD(FreePrivateData)(THIS_ REFGUID refguid);
	D912PXY_METHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew);
	D912PXY_METHOD_(DWORD, GetPriority)(THIS);
	D912PXY_METHOD_(void, PreLoad)(THIS);
	D912PXY_METHOD_(D3DRESOURCETYPE, GetType)(THIS);
	D912PXY_METHOD(GetContainer)(THIS_ REFIID riid, void** ppContainer);
	D912PXY_METHOD(GetDesc)(THIS_ D3DSURFACE_DESC *pDesc);
	D912PXY_METHOD(LockRect)(THIS_ D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);
	D912PXY_METHOD(UnlockRect)(THIS);
	D912PXY_METHOD(GetDC)(THIS_ HDC *phdc);
	D912PXY_METHOD(ReleaseDC)(THIS_ HDC hdc);

	//inner metods

	void ClearAsRTV(FLOAT* color4f, ID3D12GraphicsCommandList* cl, D3D12_RECT* clearRect);
	void ClearAsDSV(FLOAT Depth, UINT8 Stencil, D3D12_CLEAR_FLAGS flag, ID3D12GraphicsCommandList* cl, D3D12_RECT* clearRect);

	D3D12_CPU_DESCRIPTOR_HANDLE GetDHeapHandle();
	UINT GetSRVHeapId();
	d912pxy_surface_layer* GetLayer(UINT32 mip, UINT32 ar);
	D3DSURFACE_DESC GetDX9DescAtLevel(UINT level);

	DXGI_FORMAT GetDSVFormat();
	DXGI_FORMAT GetSRVFormat();
	DXGI_FORMAT ConvertInnerDSVFormat();

	void CopySurfaceDataToCPU();
	void UploadSurfaceData(d912pxy_upload_item* ul, UINT lv, ID3D12GraphicsCommandList* cl);
	void DelayedLoad(void* mem, UINT lv);
	
	size_t GetFootprintMemSz();
	UINT GetWPitchDX9(UINT lv);
	UINT GetWPitchLV(UINT lv);
	   	
	UINT FinalReleaseCB();
	UINT32 PooledAction(UINT32 use);
	void MarkPooled(UINT uid) { isPooled = uid; };
	void SetDHeapIDFeedbackPtr(UINT32* feedbackPtr) { dheapIdFeedback = feedbackPtr; };
	
	void initInternalBuf();
	void UpdateDescCache();
	UINT32 AllocateSRV();
	void AllocateLayers();
	void FreeLayers();
	void FreeObjAndSlot();

	void FinishUpload();

	static UINT32 threadedCtor;

private:
	D3D12_CPU_DESCRIPTOR_HANDLE rtdsHPtr;
	UINT dheapId;
	UINT32* dheapIdFeedback;

	UINT isPooled;
	DXGI_FORMAT m_fmt;
	UINT mem_perPixel;

	D3DSURFACE_DESC surf_dx9dsc;
	D3D12_RESOURCE_DESC descCache;
	
	d912pxy_surface_layer** layers;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT* subresFootprints;
	size_t* subresSizes;
	UINT subresCountCache;	

	UINT ulMarked;
	d912pxy_upload_item** ul;
};


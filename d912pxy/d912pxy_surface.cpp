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

UINT32 d912pxy_surface::threadedCtor = 0;


d912pxy_surface * d912pxy_surface::CorrectLayerRepresent(d912pxy_com_object * obj)
{
	if ((intptr_t)obj & PXY_COM_OBJ_SIGNATURE_SURFACE_LAYER)
	{
		return &(obj->layer.GetBaseSurface()->surface);
	}
	else
		return &(obj->surface);
}

d912pxy_surface::d912pxy_surface(UINT Width, UINT Height, D3DFORMAT Format, DWORD Usage, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, UINT* levels, UINT arrSz, UINT32* srvFeedback) 
	: d912pxy_resource(RTID_SURFACE, PXY_COM_OBJ_SURFACE, L"surface")
	, isPooled(0)
	, ulMarked(0)
	, ul(NULL)
	, layers(NULL)
	, dheapId(-1)
	, dheapIdFeedback(srvFeedback)
{
	stateCache = D3D12_RESOURCE_STATE_COMMON;
	dHeap = d912pxy_s.dev.GetDHeap(PXY_INNER_HEAP_SRV);	

	surf_dx9dsc.Format = Format;
	surf_dx9dsc.Width = Width;
	surf_dx9dsc.Height = Height;
	surf_dx9dsc.MultiSampleType = MultiSample;
	surf_dx9dsc.MultiSampleQuality = MultisampleQuality;
	surf_dx9dsc.Pool = D3DPOOL_MANAGED;
	surf_dx9dsc.Type = D3DRTYPE_SURFACE;
	surf_dx9dsc.Usage = Usage;
	
	m_fmt = d912pxy_helper::DXGIFormatFromDX9FMT(Format);
	LOG_DBG_DTDM("fmt %u => %u", Format, m_fmt);	

	if (Format == D3DFMT_NULL)//FOURCC NULL DX9 no rendertarget trick
	{
		PXY_MALLOC(subresFootprints, sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) * 1, D3D12_PLACED_SUBRESOURCE_FOOTPRINT*);

		PXY_MALLOC(ul, sizeof(d912pxy_surface_ul), d912pxy_surface_ul*);
		ZeroMemory(ul, sizeof(d912pxy_surface_ul));

		LOG_DBG_DTDM("w %u h %u u %u FCC NULL", surf_dx9dsc.Width, surf_dx9dsc.Height, surf_dx9dsc.Usage);
		return;
	}

	if (Usage == D3DUSAGE_D912PXY_FORCE_RT)
	{
		float white[4] = { 1.0f,1.0f,1.0f,1.0f };
		d12res_rtgt(m_fmt, white, Width, Height);
	}
	else if (Usage == D3DUSAGE_DEPTHSTENCIL)
	{
		d12res_zbuf(ConvertInnerDSVFormat(), 1.0f, Width, Height, GetDSVFormat());
	} else if (Usage == D3DUSAGE_RENDERTARGET)
	{
		float white[4] = { 1.0f,1.0f,1.0f,1.0f };
		d12res_rtgt(m_fmt, white, Width, Height);		
	}
	else if (!threadedCtor || (*levels == 0)) {
		dheapId = 0;
		d12res_tex2d(Width, Height, m_fmt, (UINT16*)levels, arrSz);
	}
	else {
		descCache = {
			D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0,
			Width, Height, (UINT16)arrSz, *((UINT16*)levels),
			m_fmt, {1, 0},
			D3D12_TEXTURE_LAYOUT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE
		};
	}

	UpdateDescCache();
	initInternalBuf();

	if (Usage == D3DUSAGE_DEPTHSTENCIL)
	{
		d912pxy_dheap* dsvHeap = d912pxy_s.dev.GetDHeap(PXY_INNER_HEAP_DSV);

		D3D12_DEPTH_STENCIL_VIEW_DESC dsc2;
		dsc2.Format = GetDSVFormat();
		dsc2.Flags = D3D12_DSV_FLAG_NONE;
		dsc2.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsc2.Texture2D.MipSlice = 0;

		rtdsHPtr = dsvHeap->GetDHeapHandle(dsvHeap->CreateDSV(m_res, &dsc2));
	} else if (Usage == D3DUSAGE_RENDERTARGET)
	{
		d912pxy_dheap* rtvHeap = d912pxy_s.dev.GetDHeap(PXY_INNER_HEAP_RTV);

		rtdsHPtr = rtvHeap->GetDHeapHandle(rtvHeap->CreateRTV(m_res, NULL));		
	}
	else if (Usage == D3DUSAGE_D912PXY_FORCE_RT)
	{
		d912pxy_dheap* rtvHeap = d912pxy_s.dev.GetDHeap(PXY_INNER_HEAP_RTV);

		rtdsHPtr = rtvHeap->GetDHeapHandle(rtvHeap->CreateRTV(m_res, NULL));

		AllocateLayers();
	} 
	else {
		if (!threadedCtor || (dheapId == 0))
			dheapId = AllocateSRV(m_res);
		else
			dheapId = 0;

		AllocateLayers();

		rtdsHPtr.ptr = 0;
	}
	
	LOG_DBG_DTDM("w %u h %u u %u", surf_dx9dsc.Width, surf_dx9dsc.Height, surf_dx9dsc.Usage);
}

d912pxy_surface * d912pxy_surface::d912pxy_surface_com(UINT Width, UINT Height, D3DFORMAT Format, DWORD Usage, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, UINT * levels, UINT arrSz, UINT32 * srvFeedback)
{
	d912pxy_com_object* ret = d912pxy_s.com.AllocateComObj(PXY_COM_OBJ_SURFACE);
	
	new (&ret->surface)d912pxy_surface(Width, Height, Format, Usage, MultiSample, MultisampleQuality, Lockable, levels, arrSz, srvFeedback);
	ret->vtable = d912pxy_com_route_get_vtable(PXY_COM_ROUTE_SURFACE);

	return &ret->surface;
}

d912pxy_surface::~d912pxy_surface()
{
	PXY_FREE(subresFootprints);
	PXY_FREE(ul);
			
	if (rtdsHPtr.ptr == 0)
	{
		if (m_res)
		{			
			FreeObjAndSlot();		
			FreeLayers();
		}

		//megai2: some objects can stuck in m_res == NULL and AllocateLayers() called with threaded ctor, without pooling
		//so we need to clean them this way to avoid memleack
		if (layers)
			FreeLayers();

	} else 
	{
		LOG_DBG_DTDM2("rt/dsv srv freeing");

		if (m_res)
		{
			FreeObjAndSlot();

			if (descCache.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
				d912pxy_s.dev.GetDHeap(PXY_INNER_HEAP_DSV)->FreeSlotByPtr(rtdsHPtr);
			else if (surf_dx9dsc.Usage == D3DUSAGE_D912PXY_FORCE_RT)
			{
				FreeLayers();
				d912pxy_s.dev.GetDHeap(PXY_INNER_HEAP_RTV)->FreeSlotByPtr(rtdsHPtr);
			} else 
				d912pxy_s.dev.GetDHeap(PXY_INNER_HEAP_RTV)->FreeSlotByPtr(rtdsHPtr);
		}
	}
}

#define D912PXY_METHOD_IMPL_CN d912pxy_surface

D912PXY_METHOD_IMPL_NC(GetDesc)(THIS_ D3DSURFACE_DESC *pDesc)
{ 
	*pDesc = surf_dx9dsc;
	return D3D_OK; 
}

D912PXY_METHOD_IMPL_NC(LockRect)(THIS_ D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{ 
	return GetLayer(0, 0)->LockRect(pLockedRect, pRect, Flags);
}

D912PXY_METHOD_IMPL_NC(UnlockRect)(THIS)
{ 
	return GetLayer(0, 0)->UnlockRect();
}

#undef D912PXY_METHOD_IMPL_CN

void d912pxy_surface::ClearAsRTV(FLOAT * color4f, ID3D12GraphicsCommandList * cl, D3D12_RECT* clearRect)
{
	const float* cc4f;
	cc4f = color4f;

	PIXBeginEvent(cl, 0, "Clear RT");

	cl->ClearRenderTargetView(rtdsHPtr, cc4f, 1, (const D3D12_RECT*)clearRect);

	PIXEndEvent(cl);
}

void d912pxy_surface::ClearAsDSV(FLOAT Depth, UINT8 Stencil, D3D12_CLEAR_FLAGS flag, ID3D12GraphicsCommandList * cl, D3D12_RECT* clearRect)
{
	PIXBeginEvent(cl, 0, "Clear DS");

	cl->ClearDepthStencilView(rtdsHPtr, flag, Depth, Stencil, 1, (const D3D12_RECT*)clearRect);

	PIXEndEvent(cl);
}

D3D12_CPU_DESCRIPTOR_HANDLE d912pxy_surface::GetDHeapHandle()
{
	return rtdsHPtr;
}

void d912pxy_surface::initInternalBuf()
{		
	mem_perPixel = d912pxy_helper::BitsPerPixel(m_fmt)/8;
}

size_t d912pxy_surface::GetFootprintMemSz()
{
	size_t ret = 0;

	for (int i = 0; i != descCache.DepthOrArraySize; ++i)
	{
		for (int j = 0; j != descCache.MipLevels; ++j)
		{
			UINT subresId = i * descCache.MipLevels + j;
			ret += subresFootprints[subresId].Footprint.RowPitch*FixBlockHeight(subresId);
		}
	}
	
	return ret;
}

size_t d912pxy_surface::GetFootprintMemSzRaw()
{
	if (surf_dx9dsc.Format == D3DFMT_NULL)
		return 0;

	D3D12_RESOURCE_ALLOCATION_INFO allocInfo = d912pxy_s.dx12.dev->GetResourceAllocationInfo(0, 1, &descCache);

	return allocInfo.SizeInBytes;
}

DXGI_FORMAT d912pxy_surface::GetDSVFormat()
{
	DXGI_FORMAT ret = m_fmt;
	switch (surf_dx9dsc.Format)
	{
		//case D3DFMT_INTZ:
		case D3DFMT_D16:
			return DXGI_FORMAT_D16_UNORM;
		case D3DFMT_D32:			
			return DXGI_FORMAT_D32_FLOAT;
		case D3DFMT_INTZ:		
		case D3DFMT_D24X8:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		default:
			return ret;
	}
}

DXGI_FORMAT d912pxy_surface::GetSRVFormat()
{
	DXGI_FORMAT ret = m_fmt;
	switch (surf_dx9dsc.Format)
	{
		//case D3DFMT_INTZ:
		case D3DFMT_D16:
			return DXGI_FORMAT_R16_UNORM;
		case D3DFMT_D32:
			return DXGI_FORMAT_R32_UINT;
		case D3DFMT_INTZ:
		case D3DFMT_D24S8:
		case D3DFMT_D24X8:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		default:
			return ret;
	}
}

DXGI_FORMAT d912pxy_surface::ConvertInnerDSVFormat()
{
	DXGI_FORMAT ret = m_fmt;
	switch (surf_dx9dsc.Format)
	{
		//case D3DFMT_INTZ:
		case D3DFMT_D16:
			return DXGI_FORMAT_R16_TYPELESS;
		case D3DFMT_D32:
			return DXGI_FORMAT_R32_TYPELESS;
		case D3DFMT_INTZ:
		case D3DFMT_D24X8:
			return DXGI_FORMAT_R24G8_TYPELESS;
		default:
			return ret;
	}
}

void d912pxy_surface::DelayedLoad(void* mem, UINT lv)
{
	//megai2: skip render target fake upload
	if (surf_dx9dsc.Usage == D3DUSAGE_D912PXY_FORCE_RT)
	{
		ThreadRef(-1);
		return;
	}

	UINT64 blockHeight = FixBlockHeight(lv);

	UINT copyNeeded = 0;

	if (!ul[lv].item)
	{
		UINT64 ul_memory_space = d912pxy_helper::AlignValueByPow2(subresFootprints[lv].Footprint.RowPitch*blockHeight, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

		ul[lv].item = d912pxy_s.thread.texld.GetUploadMem((UINT32)ul_memory_space);
		ul[lv].item->AddRef();
		ul[lv].offset = ul[lv].item->GetCurrentOffset();
		ul[lv].item->AddSpaceUsed(ul_memory_space);
		if (!ulMarked)
		{
			ThreadRef(1);
			d912pxy_s.thread.texld.AddToFinishList(this);
		}
		ulMarked = 1;
		copyNeeded = 1;
	}
		
	UINT wPitch = GetWPitchLV(lv);
	
	ID3D12GraphicsCommandList* cl = d912pxy_s.dx12.cl->GID(CLG_TEX);

	d912pxy_upload_item* ul_obj = ul[lv].item;

	D3D12_TEXTURE_COPY_LOCATION srcR = { ul_obj->GetResourcePtr(), D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, 0 };

	if (!m_res)
		ConstructResource();

	srcR.PlacedFootprint = subresFootprints[lv];
	srcR.PlacedFootprint.Offset = ul[lv].offset;

	ul_obj->Reconstruct(
		mem,
		subresFootprints[lv].Footprint.RowPitch,
		blockHeight,
		wPitch,
		srcR.PlacedFootprint.Offset,
		0
	);	

	//megai2: actually as we upload whole texture from same upload memory area, we don't need to copy it more then 1 time
	if (copyNeeded)
	{
		D3D12_TEXTURE_COPY_LOCATION dstR = { m_res, D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, lv };

		BTransit(lv, D3D12_RESOURCE_STATE_COPY_DEST, stateCache, cl);
		cl->CopyTextureRegion(&dstR, 0, 0, 0, &srcR, NULL);
		BTransit(lv, stateCache, D3D12_RESOURCE_STATE_COPY_DEST, cl);
	}

	ThreadRef(-1);
}

UINT d912pxy_surface::FinalReleaseCB()
{
	if (isPooled)
	{
		if (d912pxy_s.pool.surface.IsRunning())
		{
			EvictFromGPU();

			d912pxy_surface* tv = this;
			d912pxy_s.pool.surface.PoolRW(isPooled, &tv, 1);
			return 0;
		}
		else {
			return 1;
		}
	}
	else
		return 1;	
}

UINT32 d912pxy_surface::PooledAction(UINT32 use)
{
	if (!d912pxy_comhandler::PooledAction(use))
	{
		if (use)
			MakeGPUResident();

		return 0;
	}

#ifdef ENABLE_METRICS
	d912pxy_s.pool.surface.ChangePoolSize((INT)GetFootprintMemSzRaw() * (use ? 1 : -1));
#endif

	if ((surf_dx9dsc.Usage != D3DUSAGE_DEPTHSTENCIL) && (surf_dx9dsc.Usage != D3DUSAGE_RENDERTARGET) && (surf_dx9dsc.Usage != D3DUSAGE_D912PXY_FORCE_RT))
	{
		if (use)
		{
			if (!threadedCtor)
			{
				d12res_tex2d(surf_dx9dsc.Width, surf_dx9dsc.Height, m_fmt, &descCache.MipLevels, descCache.DepthOrArraySize);
				dheapId = AllocateSRV(m_res);
			}
			else 
				dheapId = 0;

			stateCache = D3D12_RESOURCE_STATE_COMMON;

			AllocateLayers();
		}
		else {
			FreeObjAndSlot();
			FreeLayers();			
		}
	}
	else {
		if (surf_dx9dsc.Format == D3DFMT_NULL)
		{
			PooledActionExit();

			return 0;
		}

		if (use)
		{
			if (surf_dx9dsc.Usage == D3DUSAGE_DEPTHSTENCIL)
			{
				d12res_zbuf(ConvertInnerDSVFormat(), 1.0f, surf_dx9dsc.Width, surf_dx9dsc.Height, GetDSVFormat());

				d912pxy_dheap* dsvHeap = d912pxy_s.dev.GetDHeap(PXY_INNER_HEAP_DSV);

				D3D12_DEPTH_STENCIL_VIEW_DESC dsc2;
				dsc2.Format = GetDSVFormat();
				dsc2.Flags = D3D12_DSV_FLAG_NONE;
				dsc2.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				dsc2.Texture2D.MipSlice = 0;

				rtdsHPtr = dsvHeap->GetDHeapHandle(dsvHeap->CreateDSV(m_res, &dsc2));
			}
			else if (surf_dx9dsc.Usage == D3DUSAGE_RENDERTARGET)
			{
				float white[4] = { 1.0f,1.0f,1.0f,1.0f };
				d12res_rtgt(m_fmt, white, surf_dx9dsc.Width, surf_dx9dsc.Height);

				d912pxy_dheap* rtvHeap = d912pxy_s.dev.GetDHeap(PXY_INNER_HEAP_RTV);

				rtdsHPtr = rtvHeap->GetDHeapHandle(rtvHeap->CreateRTV(m_res, NULL));
			}

			dheapId = -1;
		} else {
			FreeObjAndSlot();

			if (descCache.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
				d912pxy_s.dev.GetDHeap(PXY_INNER_HEAP_DSV)->FreeSlotByPtr(rtdsHPtr);
			else
				d912pxy_s.dev.GetDHeap(PXY_INNER_HEAP_RTV)->FreeSlotByPtr(rtdsHPtr);
		}
	}

	PooledActionExit();

	return 1;
}

void d912pxy_surface::MarkPooled(UINT uid)
{
	isPooled = uid;
#ifdef ENABLE_METRICS
	d912pxy_s.pool.surface.ChangePoolSize((INT)GetFootprintMemSzRaw());
#endif
}

d912pxy_surface_layer * d912pxy_surface::GetLayer(UINT32 mip, UINT32 ar)
{
	return layers[descCache.MipLevels * ar + mip];
}

void d912pxy_surface::CopySurfaceDataToCPU()
{
	d912pxy_s.render.iframe.StateSafeFlush(0);

	d912pxy_resource* readbackBuffer = new d912pxy_resource(RTID_RB_BUF, PXY_COM_OBJ_NOVTABLE, L"readback buffer");
	readbackBuffer->d12res_readback_buffer(subresFootprints[0].Footprint.RowPitch*subresFootprints[0].Footprint.Height);
		
	D3D12_TEXTURE_COPY_LOCATION dstR = { readbackBuffer->GetD12Obj(), D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, 0 };

	UINT64 activeSize;
	D3D12_RESOURCE_DESC rDesc = m_res->GetDesc();
	d912pxy_s.dx12.dev->GetCopyableFootprints(&rDesc, 0, 1, 0, &dstR.PlacedFootprint, 0, 0, &activeSize);

	D3D12_TEXTURE_COPY_LOCATION srcR = { m_res, D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, 0 };

	ID3D12GraphicsCommandList* cl = d912pxy_s.dx12.cl->GID(CLG_SEQ);
	BTransit(0, D3D12_RESOURCE_STATE_COPY_SOURCE, stateCache, cl);
	cl->CopyTextureRegion(&dstR, 0, 0, 0, &srcR, NULL);
	BTransit(0, stateCache, D3D12_RESOURCE_STATE_COPY_SOURCE, cl);
		
	d912pxy_s.render.iframe.StateSafeFlush(1);

	intptr_t GPUdata = NULL;
	LOG_ERR_THROW2(readbackBuffer->GetD12Obj()->Map(0, 0, (void**)&GPUdata), "CopySurfaceDataToCPU map error");

	intptr_t CPUdata = (intptr_t)GetLayer(0, 0)->SurfacePixel(0, 0);

	UINT wPitch = GetWPitchDX9(0);

	for (int i = 0; i != subresFootprints[0].Footprint.Height; ++i)
	{
		memcpy((void*)CPUdata, (void*)GPUdata, subresFootprints[0].Footprint.Width*mem_perPixel);

		GPUdata += subresFootprints[0].Footprint.RowPitch;
		CPUdata += wPitch;
	}

	readbackBuffer->Release();
}

void d912pxy_surface::UpdateDescCache()
{
	if (m_res)
		descCache = m_res->GetDesc();

	subresCountCache = descCache.DepthOrArraySize * descCache.MipLevels;

	UINT32 ulArrSize = sizeof(d912pxy_surface_ul) * subresCountCache;

	PXY_MALLOC(ul, ulArrSize, d912pxy_surface_ul*);
	ZeroMemory(ul, ulArrSize);

	PXY_MALLOC(subresFootprints, sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT)*subresCountCache, D3D12_PLACED_SUBRESOURCE_FOOTPRINT*);
	
	d912pxy_s.dx12.dev->GetCopyableFootprints(
		&descCache,
		0,
		subresCountCache,
		0,
		subresFootprints,
		NULL,
		NULL,
		&subresSizes
	);	
}

UINT32 d912pxy_surface::AllocateSRV(ID3D12Resource* resPtr)
{
	UINT32 ret = 0;
	if (descCache.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
	{
		if (surf_dx9dsc.Format == D3DFMT_NULL)
			return ret;

		D3D12_SHADER_RESOURCE_VIEW_DESC newSrv;

		newSrv.Format = GetSRVFormat();
		newSrv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		newSrv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		newSrv.Texture2D.PlaneSlice = 0;
		newSrv.Texture2D.MipLevels = 1;
		newSrv.Texture2D.MostDetailedMip = 0;
		newSrv.Texture2D.ResourceMinLODClamp = 0;

		ret = dHeap->CreateSRV(resPtr, &newSrv);
	}
	else {
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDsc;
		srvDsc.Format = m_fmt;
		srvDsc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDsc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		switch (m_fmt)
		{
		case DXGI_FORMAT_BC5_UNORM:
			srvDsc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(1, 0, 5, 3);//megai2: need to find proof document on that mapping orders in dx9 hlsl
			break;
		case DXGI_FORMAT_R8G8_UNORM:
			srvDsc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0, 0, 0, 1);
			break;
		case DXGI_FORMAT_R8_UNORM:
			srvDsc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0, 0, 0, 3);
			break;
		default:
			;
			break;
		}

		if (descCache.DepthOrArraySize != 6)
		{

			srvDsc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;

			srvDsc.Texture2DArray.MipLevels = descCache.MipLevels;
			srvDsc.Texture2DArray.MostDetailedMip = 0;
			srvDsc.Texture2DArray.PlaneSlice = 0;
			srvDsc.Texture2DArray.ResourceMinLODClamp = 0;
			srvDsc.Texture2DArray.ArraySize = descCache.DepthOrArraySize;
			srvDsc.Texture2DArray.FirstArraySlice = 0;

		}
		else {
			srvDsc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

			srvDsc.TextureCube.MipLevels = descCache.MipLevels;
			srvDsc.TextureCube.MostDetailedMip = 0;
			srvDsc.TextureCube.ResourceMinLODClamp = 0;
		}

		ret = dHeap->CreateSRV(resPtr, &srvDsc);

		if (dheapIdFeedback)
			*dheapIdFeedback = ret;
	}

	return ret;
}

void d912pxy_surface::AllocateLayers()
{
	PXY_MALLOC(layers, sizeof(d912pxy_surface_layer*) * descCache.DepthOrArraySize * descCache.MipLevels, d912pxy_surface_layer**);

	for (int i = 0; i != descCache.DepthOrArraySize; ++i)
	{
		for (int j = 0; j != descCache.MipLevels; ++j)
		{
			UINT subresId = i * descCache.MipLevels + j;
			layers[subresId] = d912pxy_surface_layer::d912pxy_surface_layer_com(
				comBase,
				subresId,
				subresFootprints[subresId].Footprint.RowPitch*FixBlockHeight(subresId),
				GetWPitchDX9(subresId),
				subresFootprints[subresId].Footprint.Width,
				mem_perPixel
			);
		}
	}

	
}

void d912pxy_surface::FreeLayers()
{
	for (int i = 0; i != descCache.DepthOrArraySize; ++i)
	{
		for (int j = 0; j != descCache.MipLevels; ++j)
		{
			UINT subresId = i * descCache.MipLevels + j;
			d912pxy_surface_layer::DeAllocate(layers[subresId]);
			layers[subresId] = NULL;//megai2: set this NULL to crash out if we hit something bad, like -1 dereferencing
		}
	}

	PXY_FREE(layers);
	layers = NULL;
}

void d912pxy_surface::FreeObjAndSlot()
{
	if (m_res)
	{
#ifdef _DEBUG
		if (dheapId > 0)
		{
			for (int i = 0; i != 31; ++i)
				if (d912pxy_s.render.state.tex.GetTexStage(i) == dheapId)
				{
					LOG_ERR_DTDM("dheapId %u is active in tex stage %u at deletion time", dheapId, i);
				}
		}
#endif

		m_res->Release();
		m_res = NULL;

		if (dheapId != -1)
			dHeap->FreeSlot(dheapId);
	}	
}

void d912pxy_surface::FinishUpload()
{	
	UINT subCnt = descCache.DepthOrArraySize*descCache.MipLevels;

	for (int i = 0; i != subCnt; ++i)
	{
		if (!ul[i].item)
			continue;

		ul[i].item->Release();
		ul[i].item = NULL;
	}

	ulMarked = 0;

	ThreadRef(-1);
}

void d912pxy_surface::ConstructResource()
{
	ctorSync.Hold();

	if (m_res)
	{
		ctorSync.Release();
		return;
	}

	//megai2: tmp location is needed to drop into ConstructResource from other threads when we are in ConstructResource

	ID3D12Resource* tmpLocation = d12res_tex2d_target(surf_dx9dsc.Width, surf_dx9dsc.Height, m_fmt, &descCache.MipLevels, descCache.DepthOrArraySize);
	dheapId = AllocateSRV(tmpLocation);

	m_res = tmpLocation;

	ctorSync.Release();
}

UINT d912pxy_surface::GetSRVHeapId()
{
	if (descCache.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
	{
		return GetSRVHeapIdRTDS();
	}
	
	return dheapId;
}

UINT d912pxy_surface::GetSRVHeapIdRTDS()
{
	if (surf_dx9dsc.Format == D3DFMT_NULL)
		return 0;

	if (dheapId == -1)
		dheapId = AllocateSRV(m_res);

	//megai2: doin no transit here allows us to use surface as RTV and SRV in one time, but some drivers handle this bad
	if (d912pxy_s.render.replay.DoBarrier(this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE))
	{
		d912pxy_s.render.iframe.NoteBindedSurfaceTransit(this, (descCache.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) == 0);
	}

	return dheapId;
}

UINT d912pxy_surface::GetWPitchDX9(UINT lv)
{
	UINT w = subresFootprints[lv].Footprint.Width;

	switch (surf_dx9dsc.Format)
	{
	case D3DFMT_ATI2:
		return max(1, ((w + 3) / 4)) * 4;
	case D3DFMT_DXT1:
		return max(1, ((w + 3) / 4)) * 8;
	case D3DFMT_DXT2:		
	case D3DFMT_DXT3:		
	case D3DFMT_DXT4:
	case D3DFMT_DXT5:
		return max(1, ((w + 3) / 4)) * 16;
	case D3DFMT_V8U8:
	case D3DFMT_L8:
	{

		//megai2: do not ask my why, but retrace ask for same pitch as original d3d call returned. and it somehowtricky aligned
		UINT ret = w * mem_perPixel;

		while (ret & 3)
			++ret;

		return ret;
	}
	default:
		return w * mem_perPixel;
	}
}

UINT d912pxy_surface::GetWPitchLV(UINT lv)
{
	UINT w = subresFootprints[lv].Footprint.Width;

	switch (surf_dx9dsc.Format)
	{			
	case D3DFMT_DXT1:
		return max(1, ((w + 3) / 4)) * 8;
	case D3DFMT_ATI2:
	case D3DFMT_DXT2:
	case D3DFMT_DXT3:
	case D3DFMT_DXT4:
	case D3DFMT_DXT5:
		return max(1, ((w + 3) / 4)) * 16;
	case D3DFMT_V8U8:
	case D3DFMT_L8:
	{

		//megai2: do not ask my why, but retrace ask for same pitch as original d3d call returned. and it somehowtricky aligned
		UINT ret = w * mem_perPixel;

		while (ret & 3)
			++ret;

		return ret;
	}
	default:
		return w * mem_perPixel;
	}
}

UINT d912pxy_surface::FixBlockHeight(UINT lv)
{
	UINT blockHeight = subresFootprints[lv].Footprint.Height;

	switch (surf_dx9dsc.Format)
	{
	case D3DFMT_DXT1:
	case D3DFMT_DXT2:
	case D3DFMT_DXT3:
	case D3DFMT_DXT4:
	case D3DFMT_DXT5:
	case D3DFMT_ATI2:
		blockHeight = blockHeight >> 2;
	default:
		;;
	}

	return blockHeight;
}

D3DSURFACE_DESC d912pxy_surface::GetDX9DescAtLevel(UINT level)
{
	D3DSURFACE_DESC ret;

	ret = surf_dx9dsc;

	ret.Height = subresFootprints[level].Footprint.Height;
	ret.Width = subresFootprints[level].Footprint.Width;

	return ret;
}

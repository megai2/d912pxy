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

UINT32 d912pxy_surface::threadedCtor = 0;


d912pxy_surface::d912pxy_surface(d912pxy_device* dev, UINT Width, UINT Height, D3DFORMAT Format, DWORD Usage, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, UINT* levels, UINT arrSz, UINT32* srvFeedback) : d912pxy_resource(dev, RTID_SURFACE, L"surface texture")
{
	isPooled = 0;	
	ul = NULL;
	layers = NULL;
	dheapId = -1;
	dheapIdFeedback = srvFeedback;
	dHeap = dev->GetDHeap(PXY_INNER_HEAP_SRV);	

	surf_dx9dsc.Format = Format;
	surf_dx9dsc.Width = Width;
	surf_dx9dsc.Height = Height;
	surf_dx9dsc.MultiSampleType = MultiSample;
	surf_dx9dsc.MultiSampleQuality = MultisampleQuality;
	surf_dx9dsc.Pool = D3DPOOL_DEFAULT;
	surf_dx9dsc.Type = D3DRTYPE_SURFACE;
	surf_dx9dsc.Usage = Usage;
	
	m_fmt = d912pxy_helper::DXGIFormatFromDX9FMT(Format);
	LOG_DBG_DTDM("fmt %u => %u", Format, m_fmt);

	if (Format == D3DFMT_NULL)//FOURCC NULL DX9 no rendertarget trick
	{

		PXY_MALLOC(subresFootprints, sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) * 1, D3D12_PLACED_SUBRESOURCE_FOOTPRINT*);
		PXY_MALLOC(subresSizes, sizeof(size_t) * 1, size_t*);
		PXY_MALLOC(ul, 8, d912pxy_upload_item**);
		ZeroMemory(ul, 8);

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
		d912pxy_dheap* dsvHeap = dev->GetDHeap(PXY_INNER_HEAP_DSV);

		D3D12_DEPTH_STENCIL_VIEW_DESC dsc2;
		dsc2.Format = GetDSVFormat();
		dsc2.Flags = D3D12_DSV_FLAG_NONE;
		dsc2.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsc2.Texture2D.MipSlice = 0;

		rtdsHPtr = dsvHeap->GetDHeapHandle(dsvHeap->CreateDSV(m_res, &dsc2));
	} else if (Usage == D3DUSAGE_RENDERTARGET)
	{
		d912pxy_dheap* rtvHeap = dev->GetDHeap(PXY_INNER_HEAP_RTV);

		rtdsHPtr = rtvHeap->GetDHeapHandle(rtvHeap->CreateRTV(m_res, NULL));		
	}
	else {

		if (!threadedCtor)
			dheapId = AllocateSRV();
		else
			dheapId = 0;

		AllocateLayers();

		rtdsHPtr.ptr = 0;
	}
	
	LOG_DBG_DTDM("w %u h %u u %u", surf_dx9dsc.Width, surf_dx9dsc.Height, surf_dx9dsc.Usage);
}

d912pxy_surface::~d912pxy_surface()
{
	PXY_FREE(subresFootprints);
	PXY_FREE(subresSizes);
	PXY_FREE(ul);
	
	if (rtdsHPtr.ptr == 0)
	{
		if (m_res)
		{			
			FreeObjAndSlot();
			FreeLayers();
		}
	} else 
	{
		LOG_DBG_DTDM2("rt/dsv srv freeing");

		if (m_res)
		{
			FreeObjAndSlot();

			if (descCache.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
				m_dev->GetDHeap(PXY_INNER_HEAP_DSV)->FreeSlotByPtr(rtdsHPtr);
			else
				m_dev->GetDHeap(PXY_INNER_HEAP_RTV)->FreeSlotByPtr(rtdsHPtr);
		}
	}
}

#define D912PXY_METHOD_IMPL_CN d912pxy_surface

D912PXY_IUNK_IMPL

/*** IDirect3DResource9 methods ***/
D912PXY_METHOD_IMPL(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) { return d912pxy_resource::GetDevice(ppDevice); }
D912PXY_METHOD_IMPL(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags){ return d912pxy_resource::SetPrivateData(refguid, pData, SizeOfData, Flags); }
D912PXY_METHOD_IMPL(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData){ return d912pxy_resource::GetPrivateData(refguid, pData, pSizeOfData); }
D912PXY_METHOD_IMPL(FreePrivateData)(THIS_ REFGUID refguid){ return d912pxy_resource::FreePrivateData(refguid); }
D912PXY_METHOD_IMPL_(DWORD, SetPriority)(THIS_ DWORD PriorityNew){ return d912pxy_resource::SetPriority(PriorityNew); }
D912PXY_METHOD_IMPL_(DWORD, GetPriority)(THIS){ return d912pxy_resource::GetPriority(); }
D912PXY_METHOD_IMPL_(void, PreLoad)(THIS){ d912pxy_resource::PreLoad(); }
D912PXY_METHOD_IMPL_(D3DRESOURCETYPE, GetType)(THIS) { return d912pxy_resource::GetType(); }

//surface methods
D912PXY_METHOD_IMPL(GetContainer)(THIS_ REFIID riid, void** ppContainer)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	return D3DERR_INVALIDCALL; 
}

D912PXY_METHOD_IMPL(GetDesc)(THIS_ D3DSURFACE_DESC *pDesc)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	*pDesc = surf_dx9dsc;
	return D3D_OK; 
}

D912PXY_METHOD_IMPL(LockRect)(THIS_ D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{ 
	return GetLayer(0, 0)->LockRect(pLockedRect, pRect, Flags);
}

D912PXY_METHOD_IMPL(UnlockRect)(THIS)
{ 
	return GetLayer(0, 0)->UnlockRect();	
}

D912PXY_METHOD_IMPL(GetDC)(THIS_ HDC *phdc)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	return D3DERR_INVALIDCALL; 
}

D912PXY_METHOD_IMPL(ReleaseDC)(THIS_ HDC hdc)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	return D3D_OK; 
}

#undef D912PXY_METHOD_IMPL_CN

void d912pxy_surface::ClearAsRTV(FLOAT * color4f, ID3D12GraphicsCommandList * cl, D3D12_RECT* clearRect)
{
	const float* cc4f;
	cc4f = color4f;

	cl->ClearRenderTargetView(rtdsHPtr, cc4f, 1, (const D3D12_RECT*)clearRect);
}

void d912pxy_surface::ClearAsDSV(FLOAT Depth, UINT8 Stencil, D3D12_CLEAR_FLAGS flag, ID3D12GraphicsCommandList * cl, D3D12_RECT* clearRect)
{
	cl->ClearDepthStencilView(rtdsHPtr, flag, Depth, Stencil, 1, (const D3D12_RECT*)clearRect);
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
	LOG_DBG_DTDM(__FUNCTION__);

	size_t retSum = 0;

	for (int i = 0; i != subresCountCache; ++i)
		retSum += subresSizes[i];
	
	return retSum;
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

	if (!ul[lv])
	{
		ul[lv] = d912pxy_s(pool_upload)->GetUploadObject(subresFootprints[lv].Footprint.RowPitch*subresFootprints[lv].Footprint.Height);		
		if (!ulMarked)
		{
			ThreadRef(1);
			d912pxy_s(texloadThread)->AddToFinishList(this);
		}
		ulMarked = 1;		
	}

	UINT wPitch = GetWPitchLV(lv);
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

	ul[lv]->Reconstruct(
		mem,
		subresFootprints[lv].Footprint.RowPitch,
		blockHeight,
		wPitch,
		0
	);	

	UploadSurfaceData(ul[lv], lv, d912pxy_s(GPUcl)->GID(CLG_TEX));
	
	ThreadRef(-1);
}

UINT d912pxy_surface::FinalReleaseCB()
{
	if (isPooled)
	{
		if (d912pxy_s(pool_surface))
		{
			EvictFromGPU();

			d912pxy_surface* tv = this;
			d912pxy_s(pool_surface)->PoolRW(isPooled, &tv, 1);
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

	if ((surf_dx9dsc.Usage != D3DUSAGE_DEPTHSTENCIL) && (surf_dx9dsc.Usage != D3DUSAGE_RENDERTARGET))
	{
		if (use)
		{
			if (!threadedCtor)
			{
				d12res_tex2d(surf_dx9dsc.Width, surf_dx9dsc.Height, m_fmt, &descCache.MipLevels, descCache.DepthOrArraySize);
				dheapId = AllocateSRV();
			}
			else
				dheapId = 0;

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

				d912pxy_dheap* dsvHeap = m_dev->GetDHeap(PXY_INNER_HEAP_DSV);

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

				d912pxy_dheap* rtvHeap = m_dev->GetDHeap(PXY_INNER_HEAP_RTV);

				rtdsHPtr = rtvHeap->GetDHeapHandle(rtvHeap->CreateRTV(m_res, NULL));
			}

			dheapId = -1;
		} else {
			FreeObjAndSlot();

			if (descCache.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
				m_dev->GetDHeap(PXY_INNER_HEAP_DSV)->FreeSlotByPtr(rtdsHPtr);
			else
				m_dev->GetDHeap(PXY_INNER_HEAP_RTV)->FreeSlotByPtr(rtdsHPtr);
		}
	}

	PooledActionExit();

	return 1;
}

d912pxy_surface_layer * d912pxy_surface::GetLayer(UINT32 mip, UINT32 ar)
{
	return layers[descCache.MipLevels * ar + mip];
}

void d912pxy_surface::CopySurfaceDataToCPU()
{
	d912pxy_resource* readbackBuffer = new d912pxy_resource(m_dev, RTID_RB_BUF, L"readback buffer");
	readbackBuffer->d12res_readback_buffer(subresFootprints[0].Footprint.RowPitch*subresFootprints[0].Footprint.Height);
		
	D3D12_TEXTURE_COPY_LOCATION dstR = { readbackBuffer->GetD12Obj(), D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, 0 };

	UINT64 activeSize;
	d912pxy_s(DXDev)->GetCopyableFootprints(&m_res->GetDesc(), 0, 1, 0, &dstR.PlacedFootprint, 0, 0, &activeSize);

	D3D12_TEXTURE_COPY_LOCATION srcR = { m_res, D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, 0 };

	ID3D12GraphicsCommandList* cl = d912pxy_s(GPUcl)->GID(CLG_SEQ);
	BTransit(0, D3D12_RESOURCE_STATE_COPY_SOURCE, stateCache, cl);
	cl->CopyTextureRegion(&dstR, 0, 0, 0, &srcR, NULL);
	BTransit(0, stateCache, D3D12_RESOURCE_STATE_COPY_SOURCE, cl);
		
	d912pxy_s(iframe)->StateSafeFlush();

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

	UINT32 ulArrSize = sizeof(d912pxy_upload_item*) * subresCountCache;

	PXY_MALLOC(ul, ulArrSize, d912pxy_upload_item**);
	ZeroMemory(ul, ulArrSize);

	PXY_MALLOC(subresFootprints, sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT)*subresCountCache, D3D12_PLACED_SUBRESOURCE_FOOTPRINT*);
	PXY_MALLOC(subresSizes, sizeof(size_t)*subresCountCache, size_t*);
	
	d912pxy_s(DXDev)->GetCopyableFootprints(
		&descCache,
		0,
		subresCountCache,
		0,
		subresFootprints,
		NULL,
		NULL,
		subresSizes
	);	
}

UINT32 d912pxy_surface::AllocateSRV()
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

		ret = dHeap->CreateSRV(m_res, &newSrv);
	}
	else {
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDsc;
		srvDsc.Format = m_fmt;
		srvDsc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDsc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		switch (m_fmt)
		{
		case DXGI_FORMAT_BC5_UNORM:
			srvDsc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(1, 0, 1, 3);//megai2: need to find proof document on that mapping orders in dx9 hlsl
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

		ret = dHeap->CreateSRV(m_res, &srvDsc);

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
			layers[subresId] = new d912pxy_surface_layer(
				this,
				subresId,
				subresFootprints[subresId].Footprint.RowPitch*subresFootprints[subresId].Footprint.Height,
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
			delete layers[subresId];
			layers[subresId] = NULL;//megai2: set this NULL to crash out if we hit something bad, like -1 dereferencing
		}
	}

	PXY_FREE(layers);
}

void d912pxy_surface::FreeObjAndSlot()
{
	if (m_res)
	{
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
		if (!ul[i])
			continue;

		ul[i]->Release();
		ul[i] = NULL;
	}

	ulMarked = 0;

	ThreadRef(-1);
}

UINT d912pxy_surface::GetSRVHeapId()
{
	if (descCache.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
	{
		if (surf_dx9dsc.Format == D3DFMT_NULL)
			return 0;

		if (dheapId == -1)
			dheapId = AllocateSRV();
			   
		if (descCache.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
		{
			if (d912pxy_s(CMDReplay)->StateTransit(this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE))
			{
				d912pxy_s(iframe)->NoteBindedSurfaceTransit(this, 0);
			}		
		}
		else {
			//megai2: doin no transit here allows us to use surface as RTV and SRV in one time, but some drivers handle this bad
			if (d912pxy_s(CMDReplay)->StateTransit(this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE))
			{
				d912pxy_s(iframe)->NoteBindedSurfaceTransit(this, 1);
			}
		}				
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

void d912pxy_surface::UploadSurfaceData(d912pxy_upload_item* inUl, UINT lv, ID3D12GraphicsCommandList* cl)
{
	D3D12_TEXTURE_COPY_LOCATION srcR = { inUl->GetResourcePtr(), D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, 0 };

	if (!m_res)
	{
		d12res_tex2d(surf_dx9dsc.Width, surf_dx9dsc.Height, m_fmt, &descCache.MipLevels, descCache.DepthOrArraySize);
		dheapId = AllocateSRV();
	}

	GetCopyableFootprints(lv, &srcR.PlacedFootprint);

	D3D12_TEXTURE_COPY_LOCATION dstR = { m_res, D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, lv };
	
	BTransit(lv, D3D12_RESOURCE_STATE_COPY_DEST, stateCache, cl);
	cl->CopyTextureRegion(&dstR, 0, 0, 0, &srcR, NULL);
	BTransit(lv, stateCache, D3D12_RESOURCE_STATE_COPY_DEST, cl);	
}

D3DSURFACE_DESC d912pxy_surface::GetDX9DescAtLevel(UINT level)
{
	D3DSURFACE_DESC ret;

	ret = surf_dx9dsc;

	ret.Height = subresFootprints[level].Footprint.Height;
	ret.Width = subresFootprints[level].Footprint.Width;

	return ret;
}

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

using namespace Microsoft::WRL;

UINT64 d912pxy_device::apiOverhead = 0;

d912pxy_device::d912pxy_device(IDirect3DDevice9Proxy * dev) : IDirect3DDevice9Proxy(dev), d912pxy_comhandler(L"device")
{
	IP7_Client *l_pClient = P7_Get_Shared(TM("logger"));

	IP7_Telemetry* tTel = P7_Create_Telemetry(l_pClient, TM("dheap stats"));
	tTel->Share(L"tel_dheap");

	m_logMetrics = P7_Create_Telemetry(l_pClient, TM("iframe stats"));
	m_logMetrics->Share(L"iframe_stats");

	m_logMetrics->Create(TM("iframe1 / DIP"), 0, PXY_INNER_MAX_IFRAME_BATCH_COUNT, PXY_INNER_MAX_IFRAME_BATCH_COUNT/2, 1, &metricIFrameDraws);
	m_logMetrics->Create(TM("iframe1 / GPU"), 0, 10000, 10000, 1, &metricIFrameExec);
	m_logMetrics->Create(TM("iframe1 / CPU"), 0, 60000, 60000, 1, &metricIFramePrep);
	m_logMetrics->Create(TM("iframe1 / AOH"), 0, 60000, 60000, 1, &metricIFrameAPIOverhead);

	m_logMetrics->Create(TM("iframe2 / SYN"), 0, 5000, 5000, 1, &metricIFrameSync);
	m_logMetrics->Create(TM("iframe2 / RPL"), 0, 5000, 5000, 1, &metricIFrameReplayTime);

	m_logMetrics->Create(TM("iframe3 / PPB"), 0, 3000, 2000, 1, &metricIFramePrepPerBatch);
	m_logMetrics->Create(TM("iframe3 / OPB"), 0, 3000, 2000, 1, &metricIFrameOverheadPerBatch);	
	m_logMetrics->Create(TM("iframe3 / APT"), 0, 3000, 2000, 1, &metricIFrameAppCPU); 

	l_pClient->Release();
	
	iframeExecTime = new Stopwatch();
	iframePrepTime = new Stopwatch();
	iframeReplTime = new Stopwatch();
	iframeSyncTime = new Stopwatch();

	m_dev = NULL;

	new d912pxy_vfs();

	d912pxy_s(vfs)->SetRoot("./d912pxy/pck");
	if (!d912pxy_s(vfs)->LoadVFS(PXY_VFS_BID_CSO, "shader_cso"))
	{
		m_log->P7_ERROR(LGC_DEFAULT, TM("shader_cso VFS not loaded"));
		LOG_ERR_THROW2(-1, "VFS error");
	}

	if (!d912pxy_s(vfs)->LoadVFS(PXY_VFS_BID_SHADER_PROFILE, "shader_profiles"))
	{
		m_log->P7_ERROR(LGC_DEFAULT, TM("shader_profiles VFS not loaded"));
		LOG_ERR_THROW2(-1, "VFS error");
	}
	
	ZeroMemory(swapchains, sizeof(intptr_t)*PXY_INNER_MAX_SWAP_CHAINS);
	
	LOG_DBG_DTDM("dx9 tmp device handling");	
		
	IDirect3DDevice9* tmpDev;
	LOG_ERR_THROW(dev->PostInit(&tmpDev));	

	LOG_DBG_DTDM("dx9 tmp postini %016llX", tmpDev);
	
	LOG_ERR_THROW(tmpDev->GetDeviceCaps(&cached_dx9caps));

	//if (origD3D_create_call.pPresentationParameters->Windowed)
	LOG_ERR_THROW(tmpDev->GetDisplayMode(0, &cached_dx9displaymode));

	//tmpDev->Release();
	dev->Release();

	LOG_DBG_DTDM("dx9 tmp device ok");

	m_log->P7_INFO(LGC_DEFAULT, TM("d912pxy(Direct3D9 to Direct3D12 api proxy) loaded"));	
	m_log->P7_INFO(LGC_DEFAULT, BUILD_VERSION_NAME);
	m_log->P7_INFO(LGC_DEFAULT, TM("Batch Limit: %u"), PXY_INNER_MAX_IFRAME_BATCH_COUNT);
	m_log->P7_INFO(LGC_DEFAULT, TM("Recreation Limit: %u"), PXY_INNER_MAX_IFRAME_CLEANUPS);
	m_log->P7_INFO(LGC_DEFAULT, TM("TextureBind Limit: %u"), PXY_INNER_MAX_TEXTURE_STAGES);
	m_log->P7_INFO(LGC_DEFAULT, TM("RenderTargets Limit: %u"), PXY_INNER_MAX_RENDER_TARGETS);
	m_log->P7_INFO(LGC_DEFAULT, TM("ShaderConst Limit: %u"), PXY_INNER_MAX_SHADER_CONSTS);
	m_log->P7_INFO(LGC_DEFAULT, TM("Streams Limit: %u"), PXY_INNER_MAX_VBUF_STREAMS);
	
	m_log->P7_INFO(LGC_DEFAULT, TM("!!!NOT INTENDED TO PERFORM ALL DIRECT3D9 FEATURES!!!"));
	
	d912pxy_helper::d3d12_EnableDebugLayer();

	for (int i = 0; i != PXY_INNER_THREADID_MAX; ++i)
	{
		InitializeCriticalSection(&threadLockdEvents[i]);
	}
	InitializeCriticalSection(&threadLock);
	InitializeCriticalSection(&cleanupLock);

	ComPtr<IDXGIAdapter3> gpu = d912pxy_helper::GetAdapter();
	
	DXGI_ADAPTER_DESC2 pDesc;
	LOG_ERR_THROW(gpu->GetDesc2(&pDesc));

	gpu_totalVidmemMB = (DWORD)pDesc.DedicatedVideoMemory >> 20;
	
	m_log->P7_INFO(LGC_DEFAULT, TM("GPU name: %s vidmem: %u Mb"), pDesc.Description, gpu_totalVidmemMB);
	m_log->P7_INFO(LGC_DEFAULT, TM("original dx9 display mode width %u height %u"), cached_dx9displaymode.Width, cached_dx9displaymode.Height);

	m_d12evice = d912pxy_helper::CreateDevice(gpu);	

	m_d12evice_ptr = m_d12evice.Get();

	d912pxy_s(DXDev) = m_d12evice_ptr;

	D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT vaSizes;
	m_d12evice->CheckFeatureSupport(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, &vaSizes, sizeof(vaSizes));

	DXGI_QUERY_VIDEO_MEMORY_INFO vaMem;

	gpu->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &vaMem);
	
	m_log->P7_INFO(LGC_DEFAULT, TM("VA BPR %lu VA BPP %lu BU %u AR %u CR %u CU %u"), 
		1 << (vaSizes.MaxGPUVirtualAddressBitsPerResource - 20), 1 << (vaSizes.MaxGPUVirtualAddressBitsPerProcess - 20), 
		vaMem.Budget >> 20, vaMem.AvailableForReservation >> 20, vaMem.CurrentReservation >> 20, vaMem.CurrentUsage >> 20
	);
	
	m_log->P7_INFO(LGC_DEFAULT, TM("Adapter Nodes: %u"), m_d12evice->GetNodeCount());

	LOG_DBG_DTDM("dev %016llX", m_d12evice.Get());

	//heaps
	device_heap_config[PXY_INNER_HEAP_RTV] = { D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 512, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 };
	device_heap_config[PXY_INNER_HEAP_DSV] = { D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 64, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 };
	device_heap_config[PXY_INNER_HEAP_SRV] = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, PXY_INNER_MAX_IFRAME_BATCH_COUNT*10 + 1024, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 };
	//device_heap_config[PXY_INNER_HEAP_CBV] = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 256, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 };
	device_heap_config[PXY_INNER_HEAP_SPL] = { D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 64, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 };

	for (int i = 0; i != PXY_INNER_MAX_DSC_HEAPS; ++i)
	{
		m_dheaps[i] = new d912pxy_dheap(this, &device_heap_config[i]);
	}

	mGPUque = new d912pxy_gpu_que(this, 2, PXY_INNER_MAX_CLEANUPS_PER_SYNC, PXY_INNER_MAX_IFRAME_CLEANUPS, 0);

	d912pxy_s(GPUque) = mGPUque;

	replayer = new d912pxy_replay(this);
	mShaderDB = new d912pxy_shader_db(this);

	iframe = new d912pxy_iframe(this, m_dheaps);

	d912pxy_s(iframe) = iframe;

	d912pxy_s(textureState)->SetStatePointer(&mTextureState);

	texLoader = new d912pxy_texture_loader(this);
	bufLoader = new d912pxy_buffer_loader(this);		
	
	swapchains[0] = NULL;

	//origD3D_create_call.pPresentationParameters->Windowed = !origD3D_create_call.pPresentationParameters->Windowed;

	swapchains[0] = new d912pxy_swapchain(
		this,
		0,
		origD3D_create_call.pPresentationParameters->hDeviceWindow,
		mGPUque->GetDXQue(),
		origD3D_create_call.pPresentationParameters->BackBufferWidth,
		origD3D_create_call.pPresentationParameters->BackBufferHeight,
		origD3D_create_call.pPresentationParameters->BackBufferCount,
		!origD3D_create_call.pPresentationParameters->Windowed,
		(origD3D_create_call.pPresentationParameters->PresentationInterval != D3DPRESENT_INTERVAL_IMMEDIATE)
	);

	iframe->SetSwapper(swapchains[0]);

	new d912pxy_upload_pool(this);
	new d912pxy_vstream_pool(this);
	new d912pxy_surface_pool(this);

	d912pxy_s(thread_cleanup) = new d912pxy_cleanup_thread(this);

	iframe->Start();

	UINT uuLc = 1;
	mNullTexture = new d912pxy_surface(this, 1, 1, D3DFMT_A8B8G8R8, 0, &uuLc, 6);
	D3DLOCKED_RECT lr;

	for (int i = 0; i != 6; ++i)
	{
		mNullTexture->GetLayer(0, i)->LockRect(&lr, 0, i);
		*(UINT32*)lr.pBits = 0xFF000000;
		mNullTexture->GetLayer(0, i)->UnlockRect();
	}

	mNullTextureSRV = mNullTexture->GetSRVHeapId();

	for (int i = 0; i != 16; ++i)
		SetTexture(i, 0);

	UINT32 tmpUPbufSpace = 0xFFFF;

	mDrawUPVbuf = d912pxy_s(pool_vstream)->GetVStreamObject(tmpUPbufSpace, 0, 0)->AsDX9VB();
	mDrawUPIbuf = d912pxy_s(pool_vstream)->GetVStreamObject(tmpUPbufSpace *2, D3DFMT_INDEX16,1)->AsDX9IB();

	UINT16* ibufDt;
	mDrawUPIbuf->Lock(0, 0, (void**)&ibufDt, 0);

	for (int i = 0; i != tmpUPbufSpace; ++i)
	{
		ibufDt[i] = i;
	}

	mDrawUPIbuf->Unlock();
	mDrawUPStreamPtr = 0;

}

d912pxy_device::~d912pxy_device(void)
{	
	m_log->P7_INFO(LGC_DEFAULT, TM("d912pxy exiting"));

	replayer->Finish();
	m_log->P7_INFO(LGC_DEFAULT, TM("replayer finished"));

	iframe->End();
	m_log->P7_INFO(LGC_DEFAULT, TM("last iframe ended"));

	mDrawUPIbuf->Release();
	mDrawUPVbuf->Release();
	mNullTexture->Release();

	mGPUque->Flush(0);
	m_log->P7_INFO(LGC_DEFAULT, TM("last gpu cmd lists flushed"));

	swapchains[0]->Release();
	m_log->P7_INFO(LGC_DEFAULT, TM("swapchain stopped"));

	//megai2: we have some tree like deletions of objects, so we must call this multiple times
	for (int i = 0; i != 100; ++i)
		mGPUque->Flush(0);

	m_log->P7_INFO(LGC_DEFAULT, TM("pending GPU cleanups processed"));

	delete replayer;
	m_log->P7_INFO(LGC_DEFAULT, TM("replay thread stopped"));

	delete bufLoader;
	m_log->P7_INFO(LGC_DEFAULT, TM("buffer load thread stopped"));

	delete iframe;
	m_log->P7_INFO(LGC_DEFAULT, TM("iframe deconstructed"));

	delete mShaderDB;
	m_log->P7_INFO(LGC_DEFAULT, TM("shader database freed"));

	delete d912pxy_s(thread_cleanup);
	m_log->P7_INFO(LGC_DEFAULT, TM("cleanup thread finished"));

	delete d912pxy_s(pool_vstream);
	m_log->P7_INFO(LGC_DEFAULT, TM("vstream pool freed"));

	delete d912pxy_s(pool_upload);
	m_log->P7_INFO(LGC_DEFAULT, TM("upload pool freed"));

	delete d912pxy_s(pool_surface);
	m_log->P7_INFO(LGC_DEFAULT, TM("surface pool freed"));

	delete mGPUque;
	m_log->P7_INFO(LGC_DEFAULT, TM("GPU queue freed"));

	delete texLoader;		
	m_log->P7_INFO(LGC_DEFAULT, TM("texture load thread stopped"));

	delete iframeExecTime;
	delete iframePrepTime;
	delete iframeReplTime;
	delete iframeSyncTime;

	m_log->P7_INFO(LGC_DEFAULT, TM("stopwatches freed"));
	
	for (int i = 0; i != PXY_INNER_MAX_DSC_HEAPS; ++i)
		delete m_dheaps[i];

	m_log->P7_INFO(LGC_DEFAULT, TM("heaps deleted"));

	m_logMetrics->Release();

	m_log->P7_INFO(LGC_DEFAULT, TM("lock objects freed"));

	m_log->P7_DEBUG(LGC_DEFAULT, TM("max encountered consts PS %u VS %u"), last_ps_fvconsts, last_vs_fvconsts);

	m_log->P7_INFO(LGC_DEFAULT, TM("d912pxy vfs closed"));

	delete d912pxy_s(vfs);
	
	m_log->P7_INFO(LGC_DEFAULT, TM("d912pxy exited"));

#ifdef _DEBUG
	d912pxy_helper::d3d12_ReportLeaks();
#endif
}

HRESULT WINAPI d912pxy_device::QueryInterface(REFIID riid, void** ppvObj)
{ 
	return d912pxy_comhandler::QueryInterface(riid, ppvObj);
}

ULONG WINAPI d912pxy_device::AddRef(void)
{ 
	return d912pxy_comhandler::AddRef();
}

ULONG WINAPI d912pxy_device::Release(void)
{ 	
	return d912pxy_comhandler::Release();
}

HRESULT WINAPI d912pxy_device::TestCooperativeLevel(void)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	return swapchains[0]->TestCoopLevel();
}

UINT WINAPI d912pxy_device::GetAvailableTextureMem(void)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	return gpu_totalVidmemMB; 
}

HRESULT WINAPI d912pxy_device::EvictManagedResources(void)
{ 
	//megai2: ignore this for now
	LOG_DBG_DTDM(__FUNCTION__);
	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::GetDeviceCaps(D3DCAPS9* pCaps)
{
	LOG_DBG_DTDM(__FUNCTION__);
	if (!pCaps)
		return D3DERR_INVALIDCALL;
	memcpy(pCaps, &cached_dx9caps, sizeof(D3DCAPS9));
	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode)
{ 
	LOG_DBG_DTDM("GetDisplayMode swCh %u ", iSwapChain);

	if (!pMode)
		return D3DERR_INVALIDCALL;

	if (iSwapChain == 0)
	{
		if (origD3D_create_call.pPresentationParameters->Windowed)
			memcpy(pMode, &cached_dx9displaymode, sizeof(D3DDISPLAYMODE));
		else
		{
			pMode->Width = origD3D_create_call.pPresentationParameters->BackBufferWidth;
			pMode->Height = origD3D_create_call.pPresentationParameters->BackBufferHeight;
			pMode->RefreshRate = origD3D_create_call.pPresentationParameters->FullScreen_RefreshRateInHz;
			pMode->Format = origD3D_create_call.pPresentationParameters->BackBufferFormat;
		}
	} 
	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	LOG_DBG_DTDM(__FUNCTION__);

	if (!pParameters)
		return D3DERR_INVALIDCALL;
	pParameters->AdapterOrdinal = origD3D_create_call.Adapter;
	pParameters->DeviceType = origD3D_create_call.DeviceType;
	pParameters->hFocusWindow = origD3D_create_call.hFocusWindow;
	pParameters->BehaviorFlags = origD3D_create_call.BehaviorFlags;

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap)
{ 
	//megai2: not for full d3d9 porting here
	LOG_DBG_DTDM(__FUNCTION__);
	return D3DERR_INVALIDCALL;
}

void WINAPI d912pxy_device::SetCursorPosition(int X, int Y, DWORD Flags)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	SetCursorPos(X, Y);	 
}

BOOL WINAPI d912pxy_device::ShowCursor(BOOL bShow)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	//ShowCursor(bShow); <= insanity
	return true; 
}

HRESULT WINAPI d912pxy_device::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	//zero is always present
	for (int i = 1; i != PXY_INNER_MAX_SWAP_CHAINS; ++i)
	{
		if (swapchains[i])
			continue;
		
		swapchains[i] = new d912pxy_swapchain(this, i, pPresentationParameters->hDeviceWindow, mGPUque->GetDXQue(), pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight, pPresentationParameters->BackBufferCount, !pPresentationParameters->Windowed, pPresentationParameters->PresentationInterval != D3DPRESENT_INTERVAL_IMMEDIATE);

		*pSwapChain = swapchains[i];

		return D3D_OK;
	}
	return D3DERR_OUTOFVIDEOMEMORY;
}

HRESULT WINAPI d912pxy_device::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	if (iSwapChain >= PXY_INNER_MAX_SWAP_CHAINS)
		return D3DERR_INVALIDCALL;

	*pSwapChain = swapchains[iSwapChain];

	return D3D_OK; 
}

UINT WINAPI d912pxy_device::GetNumberOfSwapChains(void)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	//This method returns the number of swap chains created by CreateDevice.
	return 1; 
}

HRESULT WINAPI d912pxy_device::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	replayer->Finish();

	iframe->End();
	mGPUque->Flush(0);

	//pPresentationParameters->Windowed = !pPresentationParameters->Windowed;
	
	//swapchains[0]->SetFullscreen(!pPresentationParameters->Windowed);

	swapchains[0]->Resize(pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight, !origD3D_create_call.pPresentationParameters->Windowed, (pPresentationParameters->PresentationInterval != D3DPRESENT_INTERVAL_IMMEDIATE));
	
	iframe->Start();

	API_OVERHEAD_TRACK_END(0)
		
	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

#ifdef FRAME_METRIC_PRESENT
	iframeReplTime->Reset();
#endif
	replayer->Finish();

#ifdef FRAME_METRIC_PRESENT
	UINT64 replTime = iframeReplTime->Elapsed().count();
#endif

	iframe->End();

#ifdef PERFORMANCE_GRAPH_WRITE
	perfGraph->RecordPresent(iframe->GetBatchCount());
#endif

#ifdef FRAME_METRIC_PRESENT
	UINT64 prepCPUtime = iframePrepTime->Elapsed().count() - replTime;

	m_logMetrics->Add(metricIFramePrep, prepCPUtime);
	m_logMetrics->Add(metricIFrameReplayTime, replTime);
	m_logMetrics->Add(metricIFrameDraws, iframe->GetBatchCount());

	m_logMetrics->Add(metricIFramePrepPerBatch, prepCPUtime / (iframe->GetBatchCount()+1));

#ifdef FRAME_METRIC_API_OVERHEAD

	API_OVERHEAD_TRACK_END(0)

	m_logMetrics->Add(metricIFrameAPIOverhead, d912pxy_device::apiOverhead);
	m_logMetrics->Add(metricIFrameOverheadPerBatch, d912pxy_device::apiOverhead / (iframe->GetBatchCount() + 1));
	m_logMetrics->Add(metricIFrameAppCPU, prepCPUtime - d912pxy_device::apiOverhead);

	d912pxy_device::apiOverhead = 0;
#endif
	
	iframeExecTime->Reset();
#endif

	LOG_DBG_DTDM2("Present Exec GPU");

	HRESULT ret = mGPUque->ExecuteCommands(1);

#ifdef FRAME_METRIC_PRESENT
	m_logMetrics->Add(metricIFrameExec, iframeExecTime->Elapsed().count());

	iframePrepTime->Reset();
#endif

	mDrawUPStreamPtr = 0;

	iframe->Start();
	
	return ret;
}

HRESULT WINAPI d912pxy_device::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{ 
	
	LOG_DBG_DTDM(__FUNCTION__);
	
	return swapchains[iSwapChain]->GetBackBuffer(iBackBuffer, Type, ppBackBuffer);
}

HRESULT WINAPI d912pxy_device::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	return swapchains[iSwapChain]->GetRasterStatus(pRasterStatus);
}

HRESULT WINAPI d912pxy_device::SetDialogBoxMode(BOOL bEnableDialogs)
{
	//ignore
	LOG_DBG_DTDM(__FUNCTION__);
	return D3D_OK;
}

void WINAPI d912pxy_device::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	//megai2: should be handled after fullscreen changes, so skip it for now
	//if (origD3D_create_call.pPresentationParameters->Windowed)
		return;

	//get output info to convert ramp formats
	ComPtr<IDXGISwapChain4> d12sw = swapchains[iSwapChain]->GetD12swpc();
	ComPtr<IDXGIOutput> odata;
	LOG_ERR_THROW2(d12sw->GetContainingOutput(&odata), "set gamma ramp GetContainingOutput");

	DXGI_GAMMA_CONTROL_CAPABILITIES gammaCaps;
	LOG_ERR_THROW2(odata->GetGammaControlCapabilities(&gammaCaps), "set gamma ramp GetGammaControlCapabilities");

	//0 = 0
	//255 = 1.0f

	DXGI_GAMMA_CONTROL newRamp;

	newRamp.Scale.Red = 1.0f;
	newRamp.Scale.Green = 1.0f;
	newRamp.Scale.Blue = 1.0f;
	newRamp.Offset.Red = 0.0f;
	newRamp.Offset.Green = 0.0f;
	newRamp.Offset.Blue = 0.0f;
		
	float dlt = (gammaCaps.MaxConvertedValue - gammaCaps.MinConvertedValue) / gammaCaps.NumGammaControlPoints;
	float base = gammaCaps.MinConvertedValue;

	for (int i = 0; i != gammaCaps.NumGammaControlPoints; ++i)
	{
		float dxgiFI = base + dlt * i;
		for (int j = 0; j != 256; ++j)
		{
			float d9FI = j / 255.0f;
			float d9FIn = (j+1) / 255.0f;

			if ((dxgiFI >= d9FI) && (dxgiFI < d9FIn))
			{
				newRamp.GammaCurve[i].Red = pRamp->red[j] / 65535.0f;
				newRamp.GammaCurve[i].Green = pRamp->green[j] / 65535.0f;
				newRamp.GammaCurve[i].Blue = pRamp->blue[j] / 65535.0f;
				break;
			}
		}

		if (dxgiFI < 0)
		{
			newRamp.GammaCurve[i].Red = pRamp->red[0] / 65535.0f;
			newRamp.GammaCurve[i].Green = pRamp->green[0] / 65535.0f;
			newRamp.GammaCurve[i].Blue = pRamp->blue[0] / 65535.0f;
		}
		else if (dxgiFI > 1.0f) {
			newRamp.GammaCurve[i].Red = pRamp->red[255] / 65535.0f;
			newRamp.GammaCurve[i].Green = pRamp->green[255] / 65535.0f;
			newRamp.GammaCurve[i].Blue = pRamp->blue[255] / 65535.0f;
		}


	}

	LOG_ERR_THROW2(odata->SetGammaControl(&newRamp), "set gamma ramp SetGammaControl ");	

	API_OVERHEAD_TRACK_END(0)
}

void WINAPI d912pxy_device::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	//if (origD3D_create_call.pPresentationParameters->Windowed)
		return;

	//get output info to convert ramp formats
	ComPtr<IDXGISwapChain4> d12sw = swapchains[iSwapChain]->GetD12swpc();
	ComPtr<IDXGIOutput> odata;
	LOG_ERR_THROW2(d12sw->GetContainingOutput(&odata), "get gamma ramp GetContainingOutput");

	DXGI_GAMMA_CONTROL_CAPABILITIES gammaCaps;
	LOG_ERR_THROW2(odata->GetGammaControlCapabilities(&gammaCaps), "get gamma ramp GetGammaControlCapabilities");

	DXGI_GAMMA_CONTROL curRamp;
	LOG_ERR_THROW(odata->GetGammaControl(&curRamp));

	float dlt = (gammaCaps.MaxConvertedValue - gammaCaps.MinConvertedValue) / gammaCaps.NumGammaControlPoints;
	float base = gammaCaps.MinConvertedValue;

	for (int j = 0; j != 256; ++j)
	{
		float d9FI = j / 255.0f;

		for (int i = 0; i != gammaCaps.NumGammaControlPoints; ++i)
		{
			float dxgiFI = base + dlt * i;
			
			if ((dxgiFI >= d9FI) && ((dxgiFI + dlt) < d9FI))
			{
				pRamp->red[j] = (WORD)(curRamp.GammaCurve[i].Red * 65535);
				pRamp->green[j] = (WORD)(curRamp.GammaCurve[i].Green * 65535);
				pRamp->blue[j] = (WORD)(curRamp.GammaCurve[i].Blue * 65535);
				break;
			}

		}
	}

	API_OVERHEAD_TRACK_END(0)
}

HRESULT WINAPI d912pxy_device::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	void* extendedPlace = (void*)((intptr_t)malloc(sizeof(d912pxy_texture) + 8) + 8);

	*ppTexture = new (extendedPlace) d912pxy_texture(this, Width, Height, Levels, Usage, Format);

	API_OVERHEAD_TRACK_END(0)
	
	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppVolumeTexture = new d912pxy_vtexture(this, Width, Height, Depth, Levels, Usage, Format);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	void* extendedPlace = (void*)((intptr_t)malloc(sizeof(d912pxy_ctexture) + 8) + 8);

	*ppCubeTexture = new (extendedPlace) d912pxy_ctexture(this, EdgeLength, Levels, Usage, Format);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppVertexBuffer = d912pxy_s(pool_vstream)->GetVStreamObject(Length, FVF, 0)->AsDX9VB();

	//*ppVertexBuffer = new d912pxy_vbuf(this, Length, Usage, FVF);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppIndexBuffer = d912pxy_s(pool_vstream)->GetVStreamObject(Length, Format, 1)->AsDX9IB();

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	LOG_DBG_DTDM("new RT FMT: %u", Format);

	API_OVERHEAD_TRACK_START(0)

	*ppSurface = new d912pxy_surface(this, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, 0);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppSurface = new d912pxy_surface(this, Width, Height, Format, MultiSample, MultisampleQuality, Discard, 1);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)
	
	if (RenderTargetIndex >= PXY_INNER_MAX_RENDER_TARGETS)
		return D3DERR_INVALIDCALL;

	d912pxy_surface* rtSurf = (d912pxy_surface*)pRenderTarget;

	iframe->BindSurface(1 + RenderTargetIndex, rtSurf);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppRenderTarget = iframe->GetBindedSurface(RenderTargetIndex + 1);
	(*ppRenderTarget)->AddRef();

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{ 
	LOG_DBG_DTDM("depth surface set to %016llX", pNewZStencil);
	
	API_OVERHEAD_TRACK_START(0)

	iframe->BindSurface(0, (d912pxy_surface*)pNewZStencil);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppZStencilSurface = iframe->GetBindedSurface(0);
	(*ppZStencilSurface)->AddRef();	

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

//scissors

HRESULT WINAPI d912pxy_device::SetScissorRect(CONST RECT* pRect) 
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	
	API_OVERHEAD_TRACK_START(0)

	iframe->SetScissors((D3D12_RECT*)pRect);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::GetScissorRect(RECT* pRect) { 
	LOG_DBG_DTDM(__FUNCTION__);
	
	return D3DERR_INVALIDCALL; 
}

HRESULT WINAPI d912pxy_device::SetViewport(CONST D3DVIEWPORT9* pViewport)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	D3D12_VIEWPORT main_viewport;
	main_viewport.Height = pViewport->Height * 1.0f;
	main_viewport.Width = pViewport->Width * 1.0f;
	main_viewport.TopLeftX = pViewport->X * 1.0f;
	main_viewport.TopLeftY = pViewport->Y * 1.0f;
	main_viewport.MaxDepth = pViewport->MaxZ;
	main_viewport.MinDepth = pViewport->MinZ;

	iframe->SetViewport(&main_viewport);

	API_OVERHEAD_TRACK_END(0)
	
	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::GetViewport(D3DVIEWPORT9* pViewport)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	return D3DERR_INVALIDCALL;
}

HRESULT WINAPI d912pxy_device::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{ 
	LOG_DBG_DTDM("RS %u = %u", State, Value);

	API_OVERHEAD_TRACK_START(0)

	if (State > D3DRS_BLENDOPALPHA)
		return D3DERR_INVALIDCALL;

	switch (State)
	{
		case D3DRS_SCISSORTESTENABLE: 
			if (Value)
				d912pxy_s(iframe)->RestoreScissor();
			else 
				d912pxy_s(iframe)->IgnoreScissor();
			//LOG_DBG_DTDM("RS unimpl 174"); 
		break;

		case D3DRS_STENCILREF:
			replayer->OMStencilRef(Value);			
		break; //57,   /* Reference value used in stencil test */

		case D3DRS_BLENDFACTOR:
		{
			DWORD Color = Value;

			float fvClra[4];

			for (int i = 0; i != 4; ++i)
			{
				fvClra[i] = ((Color >> (i << 3)) & 0xFF) / 255.0f;
			}

			replayer->OMBlendFac(fvClra);
		}
		break; //193,   /* D3DCOLOR used for a constant blend factor during alpha blending for devices that support D3DPBLENDCAPS_BLENDFACTOR */
		
//megai2: various fixed pipeline states and obsolete functions that noone use nowdays, but must be done sometime
//also things that overly complex and don't used by target app
		case D3DRS_SHADEMODE: LOG_DBG_DTDM("RS shademode unimpl"); break; //9,    /* D3DSHADEMODE */
		case D3DRS_LASTPIXEL: LOG_DBG_DTDM("RS lastpixel unimpl"); break; //16,   /* TRUE for last-pixel on lines */
		case D3DRS_DITHERENABLE: LOG_DBG_DTDM("RS dither unimpl"); break; //26,   /* TRUE to enable dithering */
		case D3DRS_FOGENABLE: LOG_DBG_DTDM("RS fog unimpl"); break; //28,   /* TRUE to enable fog blending */
		case D3DRS_SPECULARENABLE: LOG_DBG_DTDM("RS fog unimpl"); break; //29,   /* TRUE to enable specular */
		case D3DRS_FOGCOLOR: LOG_DBG_DTDM("RS fog unimpl"); break; //34,   /* D3DCOLOR */
		case D3DRS_FOGTABLEMODE: LOG_DBG_DTDM("RS fog unimpl"); break; //35,   /* D3DFOGMODE */
		case D3DRS_FOGSTART: LOG_DBG_DTDM("RS fog unimpl"); break; //36,   /* Fog start (for both vertex and pixel fog) */
		case D3DRS_FOGEND: LOG_DBG_DTDM("RS fog unimpl"); break; //37,   /* Fog end      */
		case D3DRS_FOGDENSITY: LOG_DBG_DTDM("RS fog unimpl"); break; //38,   /* Fog density  */
		case D3DRS_RANGEFOGENABLE: LOG_DBG_DTDM("RS fog unimpl"); break; //48,   /* Enables range-based fog */
		case D3DRS_WRAP0: LOG_DBG_DTDM("RS wrapN unimpl"); break; //128,  /* wrap for 1st texture coord. set */
		case D3DRS_WRAP1: LOG_DBG_DTDM("RS wrapN unimpl"); break; //129,  /* wrap for 2nd texture coord. set */
		case D3DRS_WRAP2: LOG_DBG_DTDM("RS wrapN unimpl"); break; //130,  /* wrap for 3rd texture coord. set */
		case D3DRS_WRAP3: LOG_DBG_DTDM("RS wrapN unimpl"); break; //131,  /* wrap for 4th texture coord. set */
		case D3DRS_WRAP4: LOG_DBG_DTDM("RS wrapN unimpl"); break; //132,  /* wrap for 5th texture coord. set */
		case D3DRS_WRAP5: LOG_DBG_DTDM("RS wrapN unimpl"); break; //133,  /* wrap for 6th texture coord. set */
		case D3DRS_WRAP6: LOG_DBG_DTDM("RS wrapN unimpl"); break; //134,  /* wrap for 7th texture coord. set */
		case D3DRS_WRAP7: LOG_DBG_DTDM("RS wrapN unimpl"); break; //135,  /* wrap for 8th texture coord. set */
		case D3DRS_WRAP8: LOG_DBG_DTDM("RS wrapN unimpl"); break; //198,   /* Additional wrap states for vs_3_0+ attributes with D3DDECLUSAGE_TEXCOORD */
		case D3DRS_WRAP9: LOG_DBG_DTDM("RS wrapN unimpl"); break; //199,
		case D3DRS_WRAP10: LOG_DBG_DTDM("RS wrapN unimpl"); break; //200,
		case D3DRS_WRAP11: LOG_DBG_DTDM("RS wrapN unimpl"); break; //201,
		case D3DRS_WRAP12: LOG_DBG_DTDM("RS wrapN unimpl"); break; //202,
		case D3DRS_WRAP13: LOG_DBG_DTDM("RS wrapN unimpl"); break; //203,
		case D3DRS_WRAP14: LOG_DBG_DTDM("RS wrapN unimpl");  break; //204,
		case D3DRS_WRAP15: LOG_DBG_DTDM("RS wrapN unimpl"); break; //205,
		case D3DRS_TEXTUREFACTOR: LOG_DBG_DTDM("RS texturefactor unimpl"); break; //60,   /* D3DCOLOR used for multi-texture blend */
		case D3DRS_CLIPPING: LOG_DBG_DTDM("RS clipping unimpl"); break; //136,
		case D3DRS_LIGHTING: LOG_DBG_DTDM("RS unimpl"); break; //137,
		case D3DRS_AMBIENT: LOG_DBG_DTDM("RS unimpl 139"); break;
		case D3DRS_FOGVERTEXMODE: LOG_DBG_DTDM("RS unimpl 140"); break;
		case D3DRS_COLORVERTEX: LOG_DBG_DTDM("RS unimpl 141"); break;
		case D3DRS_LOCALVIEWER: LOG_DBG_DTDM("RS unimpl 142"); break;
		case D3DRS_NORMALIZENORMALS: LOG_DBG_DTDM("RS unimpl 143"); break;
		case D3DRS_DIFFUSEMATERIALSOURCE: LOG_DBG_DTDM("RS unimpl 145"); break;
		case D3DRS_SPECULARMATERIALSOURCE: LOG_DBG_DTDM("RS unimpl 146"); break;
		case D3DRS_AMBIENTMATERIALSOURCE: LOG_DBG_DTDM("RS unimpl 147"); break;
		case D3DRS_EMISSIVEMATERIALSOURCE: LOG_DBG_DTDM("RS unimpl 148"); break;
		case D3DRS_VERTEXBLEND: LOG_DBG_DTDM("RS unimpl 151"); break;		
		case D3DRS_POINTSIZE: LOG_DBG_DTDM("RS unimpl 154"); break;   /* float point size */
		case D3DRS_POINTSIZE_MIN: LOG_DBG_DTDM("RS unimpl 155"); break;   /* float point size min threshold */
		case D3DRS_POINTSPRITEENABLE: LOG_DBG_DTDM("RS unimpl 156"); break;   /* BOOL point texture coord control */
		case D3DRS_POINTSCALEENABLE: LOG_DBG_DTDM("RS unimpl 157"); break;   /* BOOL point size scale enable */
		case D3DRS_POINTSCALE_A: LOG_DBG_DTDM("RS unimpl 158"); break;   /* float point attenuation A value */
		case D3DRS_POINTSCALE_B: LOG_DBG_DTDM("RS unimpl 159"); break;   /* float point attenuation B value */
		case D3DRS_POINTSCALE_C: LOG_DBG_DTDM("RS unimpl 160"); break;   /* float point attenuation C value */			
		case D3DRS_MULTISAMPLEANTIALIAS: LOG_DBG_DTDM("RS unimpl 161"); break;  // BOOL - set to do FSAA with multisample buffer
		case D3DRS_MULTISAMPLEMASK: LOG_DBG_DTDM("RS unimpl 162"); break;  // DWORD - per-sample enable/disable		
		case D3DRS_PATCHEDGESTYLE: LOG_DBG_DTDM("RS unimpl 163"); break;  // Sets whether patch edges will use float style tessellation
		case D3DRS_DEBUGMONITORTOKEN: LOG_DBG_DTDM("RS unimpl 165"); break;  // DEBUG ONLY - token to debug monitor
		case D3DRS_POINTSIZE_MAX: LOG_DBG_DTDM("RS unimpl 166"); break;   /* float point size max threshold */
		case D3DRS_INDEXEDVERTEXBLENDENABLE: LOG_DBG_DTDM("RS unimpl 167"); break;
		case D3DRS_TWEENFACTOR: LOG_DBG_DTDM("RS unimpl 170"); break;   // float tween factor
		case D3DRS_POSITIONDEGREE: LOG_DBG_DTDM("RS unimpl 172"); break;   // NPatch position interpolation degree. D3DDEGREE_LINEAR or D3DDEGREE_CUBIC (default)
		case D3DRS_NORMALDEGREE: LOG_DBG_DTDM("RS unimpl 173"); break;   // NPatch normal interpolation degree. D3DDEGREE_LINEAR (default) or D3DDEGREE_QUADRATIC		
		case D3DRS_MINTESSELLATIONLEVEL: LOG_DBG_DTDM("RS unimpl 178"); break;
		case D3DRS_MAXTESSELLATIONLEVEL: LOG_DBG_DTDM("RS unimpl 179"); break;
		case D3DRS_ADAPTIVETESS_X: LOG_DBG_DTDM("RS unimpl 180"); break;
		case D3DRS_ADAPTIVETESS_Y: LOG_DBG_DTDM("RS unimpl 181"); break;
		case D3DRS_ADAPTIVETESS_Z: LOG_DBG_DTDM("RS unimpl 182"); break;
		case D3DRS_ADAPTIVETESS_W: LOG_DBG_DTDM("RS unimpl 183"); break;
	//	case D3DRS_SRGBWRITEENABLE: LOG_DBG_DTDM("RS unimpl 194"); break;   /* Enable rendertarget writes to be DE-linearized to SRGB (for formats that expose D3DUSAGE_QUERY_SRGBWRITE) */
		case D3DRS_ENABLEADAPTIVETESSELLATION: LOG_DBG_DTDM("RS unimpl 184"); break;

		default:
			d912pxy_s(psoCache)->State(State,Value);
	}

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	//megai2: normal apps don't call this shit and i'm lazy to do it for now
	return D3DERR_INVALIDCALL; 
}

HRESULT WINAPI d912pxy_device::ValidateDevice(DWORD* pNumPasses)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	//megai2: pretend we can do anything! YES!
	*pNumPasses = 1;
	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::SetSoftwareVertexProcessing(BOOL bSoftware)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	return D3DERR_INVALIDCALL; 
}

BOOL WINAPI d912pxy_device::GetSoftwareVertexProcessing(void) 
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	return D3DERR_INVALIDCALL; 
}

//state blocks

HRESULT WINAPI d912pxy_device::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) 
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	d912pxy_sblock* ret = new d912pxy_sblock(this, Type);
	*ppSB = ret;

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::BeginStateBlock(void) 
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::EndStateBlock(IDirect3DStateBlock9** ppSB) 
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	d912pxy_sblock* ret = new d912pxy_sblock(this, D3DSBT_ALL);

	*ppSB = ret;

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

//scene terminators

HRESULT WINAPI d912pxy_device::BeginScene(void)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::EndScene(void)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	d912pxy_s(iframe)->EndSceneReset();

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	LOG_DBG_DTDM("Clear Rects: %u", Count);

	API_OVERHEAD_TRACK_START(0)

	if (Flags & D3DCLEAR_TARGET)
	{
		float fvColor[4] =
		{
			((Color >> 24) & 0xFF) / 255.0f,
			((Color >> 16) & 0xFF) / 255.0f,
			((Color >> 8) & 0xFF) / 255.0f,
			((Color >> 0) & 0xFF) / 255.0f
		};

		d912pxy_surface* surf = iframe->GetBindedSurface(1);

		if (surf)
			replayer->RTClear(surf, fvColor);
			//iframe->GetBindedSurface(1)->d912_rtv_clear(fvColor, Count, (D3D12_RECT*)pRects);//megai2: rect is 4 uint structure, may comply
	}

	if (Flags & (D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER))
	{
		DWORD cvtCf = ((D3D12_CLEAR_FLAG_DEPTH * ((Flags & D3DCLEAR_ZBUFFER) != 0)) | (D3D12_CLEAR_FLAG_STENCIL * ((Flags & D3DCLEAR_STENCIL) != 0)));

		d912pxy_surface* surf = iframe->GetBindedSurface(0);

		if (surf)
			replayer->DSClear(surf, Z, Stencil & 0xFF, (D3D12_CLEAR_FLAGS)cvtCf);

		//	surf->d912_dsv_clear(Z, Stencil & 0xFF, Count, (D3D12_RECT*)pRects, (D3D12_CLEAR_FLAGS)cvtCf);
	}

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

//textures

HRESULT WINAPI d912pxy_device::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture)
{ 
	LOG_DBG_DTDM(__FUNCTION__);	
	
	return D3DERR_INVALIDCALL;
}

HRESULT WINAPI d912pxy_device::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture)
{
	API_OVERHEAD_TRACK_START(0)

	Stage = (Stage & 0xF) + 16 * ((Stage >> 4) != 0);

	UINT64 srvId = 0;//megai2: make this to avoid memory reading. but we must be assured that mNullTextureSRV is equal to this constant!

	if (pTexture)
	{
		srvId = *(UINT64*)((intptr_t)pTexture - 0x8);
		if (srvId & 0x100000000)
		{
			srvId = pTexture->GetPriority();
		}
	}
		
	mTextureState.dirty |= (1 << (Stage >> 2));
	mTextureState.texHeapID[Stage] = (UINT32)srvId;

#ifdef TRACK_SHADER_BUGS_PROFILE
	if (pTexture)
	{
		d912pxy_basetexture* btex = dynamic_cast<d912pxy_basetexture*>(pTexture);

		stageFormatsTrack[Stage] = btex->GetBaseSurface()->GetDX9DescAtLevel(0).Format;
	}
	else
		stageFormatsTrack[Stage] = D3DFMT_UNKNOWN;
#endif

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

//megai2: texture stage states are fixed pipeline and won't work if we use shaders, is that correct?

HRESULT WINAPI d912pxy_device::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	return D3DERR_INVALIDCALL;
}

HRESULT WINAPI d912pxy_device::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	
	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
{ 
	return D3DERR_INVALIDCALL;
}

HRESULT WINAPI d912pxy_device::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{ 
	API_OVERHEAD_TRACK_START(0)

	LOG_DBG_DTDM("Sampler[%u][%u] = %u", Sampler, Type, Value);

	d912pxy_s(samplerState)->ModSampler(Sampler, Type, Value);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

//drawers

HRESULT WINAPI d912pxy_device::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	if (1)
	{
		LOG_DBG_DTDM("DP NON INDEXED SKIPPING");
		return D3D_OK;
	}
}

HRESULT WINAPI d912pxy_device::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{ 
	LOG_DBG_DTDM("DrawIndexed PT %u BV %u MV %u NV %u SI %u PC %u", PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

	API_OVERHEAD_TRACK_START(0)

#ifdef _DEBUG
	if (PrimitiveType == D3DPT_TRIANGLEFAN)
	{
		LOG_DBG_DTDM("DP TRIFAN skipping");
		return D3D_OK;
	}
#endif

	d912pxy_s(iframe)->CommitBatch(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

	/*if (mPSO->GetPShader()->GetID() == 0x7E0715D1F372444A)
	{
		float tmpFv4[4] = { -1, 0, 0, 0 };

		for (int i = 0; i != 256; ++i)
		{
			mBatch->SetShaderConstF(1, 254, 1, tmpFv4);
			iframe->CommitBatch(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
			tmpFv4[0] += (2.0f / 256.0f)*1;
		}
	}*/

#ifdef PER_BATCH_FLUSH_DEBUG
	replayer->Finish();

	iframe->End();
	mGPUque->Flush(0);

	iframe->Start();
#endif

#ifdef TRACK_SHADER_BUGS_PROFILE
	for (int i = 0; i!=32;++i)
		if (stageFormatsTrack[i] == D3DFMT_D24X8)
		{
			TrackShaderCodeBugs(PXY_INNER_SHDR_BUG_PCF_SAMPLER, i+1, d912pxy_s(psoCache)->GetPShader()->GetID());
		}

	UINT srgbState = d912pxy_s(textureState)->GetTexStage(30);
	if (srgbState)
		for (int i = 0; i != 32; ++i)
		{
			if (srgbState & 1)
			{
				if (d912pxy_s(textureState)->GetTexStage(i) != mNullTextureSRV)
				{
					TrackShaderCodeBugs(PXY_INNER_SHDR_BUG_SRGB_READ, 1, d912pxy_s(psoCache)->GetPShader()->GetID());
					break;
				}
			}
			srgbState = srgbState >> 1;
		}

	if (d912pxy_s(psoCache)->GetDX9RsValue(D3DRS_SRGBWRITEENABLE))
		TrackShaderCodeBugs(PXY_INNER_SHDR_BUG_SRGB_WRITE, 1, d912pxy_s(psoCache)->GetPShader()->GetID());

	if (d912pxy_s(psoCache)->GetDX9RsValue(D3DRS_ALPHATESTENABLE))
		TrackShaderCodeBugs(PXY_INNER_SHDR_BUG_ALPHA_TEST, 1, d912pxy_s(psoCache)->GetPShader()->GetID());

	if (d912pxy_s(psoCache)->GetDX9RsValue(D3DRS_CLIPPLANEENABLE))
	{
		UINT32 cp = d912pxy_s(psoCache)->GetDX9RsValue(D3DRS_CLIPPLANEENABLE);
		if (cp & 1)
		{
			TrackShaderCodeBugs(PXY_INNER_SHDR_BUG_CLIPPLANE0, 1, d912pxy_s(psoCache)->GetVShader()->GetID());
		}
	}
#endif

	API_OVERHEAD_TRACK_END(0)
		
	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	return D3D_OK; 
}

//vdecl 

HRESULT WINAPI d912pxy_device::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppDecl = (IDirect3DVertexDeclaration9*)(new d912pxy_vdecl(this, pVertexElements));

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	if (pDecl)
	{
		d912pxy_s(psoCache)->IAFormat((d912pxy_vdecl*)pDecl);			
	}

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	return D3DERR_INVALIDCALL; 
}

HRESULT WINAPI d912pxy_device::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppShader = (IDirect3DVertexShader9*)(new d912pxy_vshader(this, pFunction, mShaderDB));

	API_OVERHEAD_TRACK_END(0)
	
	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::SetVertexShader(IDirect3DVertexShader9* pShader)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

//	if (!pShader)
	//	return D3D_OK;

	d912pxy_vshader* shd = (d912pxy_vshader*)pShader;

	d912pxy_s(psoCache)->VShader(shd);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
	LOG_DBG_DTDM(__FUNCTION__);

	return D3DERR_INVALIDCALL;
}

HRESULT WINAPI d912pxy_device::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppShader = (IDirect3DPixelShader9*)(new d912pxy_pshader(this, pFunction, mShaderDB));

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::SetPixelShader(IDirect3DPixelShader9* pShader) 
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

//	if (!pShader)
	//	return D3D_OK;

	d912pxy_pshader* shd = (d912pxy_pshader*)pShader;

	d912pxy_s(psoCache)->PShader(shd);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::GetPixelShader(IDirect3DPixelShader9** ppShader) 
{
	LOG_DBG_DTDM(__FUNCTION__);
	
	return D3DERR_INVALIDCALL;
}


HRESULT WINAPI d912pxy_device::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

#ifdef _DEBUG
	if (last_vs_fvconsts < ((StartRegister + Vector4fCount) << 2))
		last_vs_fvconsts = (StartRegister + Vector4fCount) << 2;

	if (PXY_INNER_MAX_SHADER_CONSTS <= ((StartRegister + Vector4fCount) * 4))
	{
		LOG_DBG_DTDM("too many shader consts, trimming");
		Vector4fCount = PXY_INNER_MAX_SHADER_CONSTS/4 - StartRegister;
	}
#endif

	d912pxy_s(batch)->SetShaderConstF(0, StartRegister, Vector4fCount, (float*)pConstantData);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount){ LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount){ LOG_DBG_DTDM(__FUNCTION__); return 0; }

HRESULT WINAPI d912pxy_device::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) 
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

#ifdef _DEBUG
	if (last_ps_fvconsts < ((StartRegister + Vector4fCount) << 2))
		last_ps_fvconsts = (StartRegister + Vector4fCount) << 2;

	if (PXY_INNER_MAX_SHADER_CONSTS <= ((StartRegister + Vector4fCount) * 4))
	{
		LOG_DBG_DTDM3("too many shader consts, trimming");
		Vector4fCount = PXY_INNER_MAX_SHADER_CONSTS/4 - StartRegister;
	}
#endif

	d912pxy_s(batch)->SetShaderConstF(1, StartRegister, Vector4fCount, (float*)pConstantData);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount) { LOG_DBG_DTDM(__FUNCTION__); return 0; }

HRESULT WINAPI d912pxy_device::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount){ LOG_DBG_DTDM(__FUNCTION__); return 0; }

HRESULT WINAPI d912pxy_device::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) { LOG_DBG_DTDM(__FUNCTION__); return 0; }

//buffer binders

HRESULT WINAPI d912pxy_device::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	LOG_DBG_DTDM("bind @%u with %u : %u", StreamNumber, OffsetInBytes, Stride);

	if (StreamNumber >= PXY_INNER_MAX_VBUF_STREAMS)
		return D3DERR_INVALIDCALL;

	iframe->SetVBuf((d912pxy_vbuf*)pStreamData, StreamNumber, OffsetInBytes, Stride);

	API_OVERHEAD_TRACK_END(0)
	
	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::SetStreamSourceFreq(UINT StreamNumber, UINT Divider)
{ 
	API_OVERHEAD_TRACK_START(0)

	LOG_DBG_DTDM("stream %u div %u", StreamNumber, Divider);

	if (StreamNumber >= PXY_INNER_MAX_VBUF_STREAMS)
		return D3DERR_INVALIDCALL;

	iframe->SetStreamFreq(StreamNumber, Divider);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	if (pIndexData)
		iframe->SetIBuf((d912pxy_ibuf*)pIndexData);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* OffsetInBytes, UINT* pStride) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::GetStreamSourceFreq(UINT StreamNumber, UINT* Divider) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::GetIndices(IDirect3DIndexBuffer9** ppIndexData){ LOG_DBG_DTDM(__FUNCTION__); return 0; }

//query!

HRESULT WINAPI d912pxy_device::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppQuery = (IDirect3DQuery9*)new d912pxy_query(this, Type);

	API_OVERHEAD_TRACK_END(0)

	return 0; 
}

//UNIMPLEMENTED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

HRESULT WINAPI d912pxy_device::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) { LOG_DBG_DTDM(__FUNCTION__); return D3DERR_INVALIDCALL; }
HRESULT WINAPI d912pxy_device::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) { LOG_DBG_DTDM(__FUNCTION__); return D3DERR_INVALIDCALL; }
HRESULT WINAPI d912pxy_device::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) { LOG_DBG_DTDM(__FUNCTION__); return D3DERR_INVALIDCALL; }
HRESULT WINAPI d912pxy_device::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) { LOG_DBG_DTDM(__FUNCTION__); return D3DERR_INVALIDCALL; }
HRESULT WINAPI d912pxy_device::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{ 	
	API_OVERHEAD_TRACK_START(0)
	replayer->StretchRect((d912pxy_surface*)pSourceSurface, (d912pxy_surface*)pDestSurface);
	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}
HRESULT WINAPI d912pxy_device::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) { LOG_DBG_DTDM(__FUNCTION__); return D3DERR_INVALIDCALL; }
HRESULT WINAPI d912pxy_device::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) { LOG_DBG_DTDM(__FUNCTION__); return D3DERR_INVALIDCALL; }

HRESULT WINAPI d912pxy_device::SetClipPlane(DWORD Index, CONST float* pPlane) 
{ 
	API_OVERHEAD_TRACK_START(0)
	d912pxy_s(batch)->SetShaderConstF(1, PXY_INNER_MAX_SHADER_CONSTS_IDX - 2 - Index, 1, (float*)pPlane);  return D3D_OK;
	API_OVERHEAD_TRACK_END(0)
}

//clipping
//^ done in shaders

HRESULT WINAPI d912pxy_device::GetClipPlane(DWORD Index, float* pPlane) { LOG_DBG_DTDM(__FUNCTION__); return D3DERR_INVALIDCALL; }
HRESULT WINAPI d912pxy_device::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::GetClipStatus(D3DCLIPSTATUS9* pClipStatus) { LOG_DBG_DTDM(__FUNCTION__); return D3DERR_INVALIDCALL; }

//fixed pipe states

HRESULT WINAPI d912pxy_device::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) { LOG_DBG_DTDM(__FUNCTION__); return 0; }

HRESULT WINAPI d912pxy_device::SetMaterial(CONST D3DMATERIAL9* pMaterial) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::GetMaterial(D3DMATERIAL9* pMaterial) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::SetLight(DWORD Index, CONST D3DLIGHT9* pLight) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::GetLight(DWORD Index, D3DLIGHT9* pLight) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::LightEnable(DWORD Index, BOOL Enable) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::GetLightEnable(DWORD Index, BOOL* pEnable) { LOG_DBG_DTDM(__FUNCTION__); return 0; }

//palette

HRESULT WINAPI d912pxy_device::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::SetCurrentTexturePalette(UINT PaletteNumber) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::GetCurrentTexturePalette(UINT *PaletteNumber) { LOG_DBG_DTDM(__FUNCTION__); return 0; }

//npatch

HRESULT WINAPI d912pxy_device::SetNPatchMode(float nSegments) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
float WINAPI d912pxy_device::GetNPatchMode(void) { LOG_DBG_DTDM(__FUNCTION__); return 0; }

HRESULT WINAPI d912pxy_device::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::DeletePatch(UINT Handle) { LOG_DBG_DTDM(__FUNCTION__); return 0; }

//megai2: you should know, that there is no apps, that can't storage their data in vertex buffers 
HRESULT WINAPI d912pxy_device::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) 
{
	API_OVERHEAD_TRACK_START(0)

	LOG_DBG_DTDM2("DPUP %u %u %016llX %u", PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);

	void* dstPtr;
	mDrawUPVbuf->Lock(mDrawUPStreamPtr, 0, &dstPtr, 0);
	memcpy(dstPtr, pVertexStreamZeroData, VertexStreamZeroStride * 3 * PrimitiveCount);
	mDrawUPVbuf->Unlock();
	
	d912pxy_ibuf* oi = iframe->GetIBuf();
	d912pxy_device_streamsrc oss = iframe->GetStreamSource(0);
	d912pxy_device_streamsrc ossi = iframe->GetStreamSource(1);
	
	iframe->SetIBuf((d912pxy_ibuf*)mDrawUPIbuf);
	iframe->SetVBuf((d912pxy_vbuf*)mDrawUPVbuf, 0, mDrawUPStreamPtr, VertexStreamZeroStride);	
	iframe->SetStreamFreq(0, 1);
	iframe->SetStreamFreq(1, 0);

	mDrawUPStreamPtr += PrimitiveCount * 3 * VertexStreamZeroStride;

	DrawIndexedPrimitive(PrimitiveType, 0, 0, 0, 0, PrimitiveCount);
	
	iframe->SetIBuf(oi);
	iframe->SetVBuf(oss.buffer, 0, oss.offset, oss.stride);
	iframe->SetStreamFreq(0, oss.divider);
	iframe->SetStreamFreq(1, ossi.divider);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) { 
	LOG_DBG_DTDM(__FUNCTION__); return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::SetFVF(DWORD FVF) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::GetFVF(DWORD* pFVF) { LOG_DBG_DTDM(__FUNCTION__); return 0; }

////////////////////////////////////////////

HRESULT d912pxy_device::PostInit(IDirect3DDevice9** realDev)
{
	//skip call to Id3d9
	return D3D_OK;
}

D3D12_HEAP_PROPERTIES d912pxy_device::GetResourceHeap(D3D12_HEAP_TYPE Type)
{
	D3D12_HEAP_PROPERTIES ret;

	ret.Type = Type;
	ret.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	ret.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	ret.CreationNodeMask = 1;
	ret.VisibleNodeMask = 1;

	return ret;
}

d912pxy_dheap * d912pxy_device::GetDHeap(UINT slot)
{
	return m_dheaps[slot];
}

void d912pxy_device::IFrameCleanupEnqeue(d912pxy_comhandler * obj)
{
	EnterCriticalSection(&cleanupLock);
	mGPUque->EnqueueCleanup(obj);
	LeaveCriticalSection(&cleanupLock);
}

void d912pxy_device::LockThread(UINT thread)
{
	LOG_DBG_DTDM("thread %u locked", thread);
	LeaveCriticalSection(&threadLockdEvents[thread]);

	EnterCriticalSection(&threadLock);
	LeaveCriticalSection(&threadLock);

	EnterCriticalSection(&threadLockdEvents[thread]);
}

void d912pxy_device::InitLockThread(UINT thread)
{
//	EnterCriticalSection(&threadLockdEvents[thread]);
}

void d912pxy_device::LockAsyncThreads()
{
#ifdef FRAME_METRIC_SYNC
	iframeSyncTime->Reset();	
#endif

	EnterCriticalSection(&threadLock);

	InterlockedIncrement(&threadInterruptState);

	texLoader->SignalWork();
	bufLoader->SignalWork();
	//iframe->PSO()->SignalWork();

	for (int i = 0; i != PXY_INNER_THREADID_MAX; ++i)
	{
		EnterCriticalSection(&threadLockdEvents[i]);			
	}
	
#ifdef FRAME_METRIC_SYNC
	m_logMetrics->Add(metricIFrameSync, iframeSyncTime->Elapsed().count());
#endif
}

void d912pxy_device::UnLockAsyncThreads()
{
	for (int i = 0; i != PXY_INNER_THREADID_MAX; ++i)
	{
		LeaveCriticalSection(&threadLockdEvents[i]);
	}

 	InterlockedDecrement(&threadInterruptState);
	LeaveCriticalSection(&threadLock);
}

#ifdef TRACK_SHADER_BUGS_PROFILE

void d912pxy_device::TrackShaderCodeBugs(UINT type, UINT val, d912pxy_shader_uid faultyId)
{
	char buf[1024];
	sprintf(buf, "%s/%016llX.bin", d912pxy_shader_db_bugs_dir, faultyId);

	UINT32 size;
	UINT32* data = (UINT32*)d912pxy_s(vfs)->LoadFile(buf, &size, PXY_VFS_BID_SHADER_PROFILE);

	if (data == NULL)
	{
		data = (UINT32*)malloc(PXY_INNER_SHDR_BUG_FILE_SIZE);
		ZeroMemory(data, PXY_INNER_SHDR_BUG_FILE_SIZE);
		data[type] = val;

		d912pxy_s(vfs)->WriteFile(buf, data, PXY_INNER_SHDR_BUG_FILE_SIZE, PXY_VFS_BID_SHADER_PROFILE);
	}
	else {

		if (size != PXY_INNER_SHDR_BUG_FILE_SIZE)
		{
			LOG_ERR_THROW2(-1, "wrong shader profile file size");
		}

		if (data[type] != val)
		{
			data[type] = val;

			d912pxy_s(vfs)->ReWriteFile(buf, data, PXY_INNER_SHDR_BUG_FILE_SIZE, PXY_VFS_BID_SHADER_PROFILE);
		}	
	}

	free(data);

	/*
	FILE* bf = fopen(buf, "rb");

	//have a bug file, check for contents
	if (bf)
	{
		fseek(bf, 0, SEEK_END);
		int sz = ftell(bf);
		fseek(bf, 0, SEEK_SET);
		sz = sz >> 3;

		for (int i = 0; i != sz; ++i)
		{
			UINT bty;
			fread(&bty, 1, 4, bf);

			if (type == bty)
			{
				UINT bva;
				fread(&bva, 1, 4, bf);

				if (bva == val)
				{
					fclose(bf);
					return;
				}
			}
			else
				fseek(bf, 4, SEEK_CUR);			
		}

		fclose(bf);
	}

	bf = fopen(buf, "ab");

	fwrite(&type, 1, 4, bf);
	fwrite(&val, 1, 4, bf);

	fflush(bf);
	fclose(bf);*/
}

#endif


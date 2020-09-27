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

void d912pxy_device::Init(IDirect3DDevice9* dev, void* par)
{
	d912pxy_comhandler::Init(PXY_COM_OBJ_STATIC, L"device");
	
	PrintInfoBanner();
	
	d912pxy_s.com.Init();
	
	UINT64 pow2sizes[PXY_INNER_MAX_POW2_ALLOC_POW - PXY_INNER_MIN_POW2_ALLOC_POW];

	for (int i = PXY_INNER_MIN_POW2_ALLOC_POW; i != PXY_INNER_MAX_POW2_ALLOC_POW; ++i)
	{
		pow2sizes[i - PXY_INNER_MIN_POW2_ALLOC_POW] = 1ULL << i;
	}

	d912pxy_s.pool.hostPow2.Init(pow2sizes, d912pxy_s.config.GetValueUI32(PXY_CFG_POOLING_HOST_VA_RESERVE), PXY_INNER_MAX_POW2_ALLOC_POW - PXY_INNER_MIN_POW2_ALLOC_POW);

	initPtr = par;
	CopyOriginalDX9Data(dev, &creationData, &initialPresentParameters);

#ifdef ENABLE_METRICS
	d912pxy_s.log.metrics.Init();
	FRAME_METRIC_PRESENT(1)
#endif

	if (d912pxy_s.config.GetValueUI32(PXY_CFG_LOG_PERF_GRAPH) || d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_ENABLE))
		perfGraph = new d912pxy_performance_graph(0);
	else
		perfGraph = NULL;

	LOG_INFO_DTDM2(InitClassFields(), "Startup step  1/10");
	LOG_INFO_DTDM2(InitVFS(), "Startup step  2/10");
	LOG_INFO_DTDM2(InitThreadSyncObjects(), "Startup step  3/10");
	LOG_INFO_DTDM2(SetupDevice(SelectSuitableGPU()), "Startup step  4/10");
	LOG_INFO_DTDM2(InitDescriptorHeaps(), "Startup step  5/10");
	LOG_INFO_DTDM2(InitSingletons(), "Startup step  6/10");
	LOG_INFO_DTDM2(InitComPatches(), "Startup step  7/10");
	LOG_INFO_DTDM2(InitNullSRV(), "Startup step  8/10");
	LOG_INFO_DTDM2(InitDrawUPBuffers(), "Startup step  9/10");
	LOG_INFO_DTDM2(InitDefaultSwapChain(&initialPresentParameters), "Startup step 10/10");
	LOG_INFO_DTDM2(d912pxy_s.render.iframe.Start(), "Started first IFrame");

	if (d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_ENABLE))
	{
		LOG_INFO_DTDM2(d912pxy_s.extras.Init(), "Initializing extras");
	}

	isRunning.SetValue(1);
}

void d912pxy_device::CopyOriginalDX9Data(IDirect3DDevice9* dev, D3DDEVICE_CREATION_PARAMETERS* origPars, D3DPRESENT_PARAMETERS* origPP)
{
	if (dev)
	{
		LOG_INFO_DTDM("Using dx9 startup");

		LOG_DBG_DTDM("dx9 tmp device handling");

		LOG_ERR_THROW2(dev->GetCreationParameters(origPars), "dx9 dev->GetCreationParameters");

		IDirect3DSwapChain9* dx9swc;
		LOG_ERR_THROW2(dev->GetSwapChain(0, &dx9swc), "dx9 dev->GetSwapChain");
		dx9swc->GetPresentParameters(origPP);		

		if (!origPP->hDeviceWindow)
			origPP->hDeviceWindow = origPars->hFocusWindow;

		if (!origPP->BackBufferHeight)
			origPP->BackBufferHeight = 1;

		if (!origPP->BackBufferWidth)
			origPP->BackBufferWidth = 1;

		LOG_ERR_THROW(dev->GetDeviceCaps(&cached_dx9caps));
		LOG_ERR_THROW(dev->GetDisplayMode(0, &cached_dx9displaymode));

		((IDirect3D9*)initPtr)->AddRef();//megai2: keep original d3d9 object

		try {

			dx9swc->Release();
			dev->Release();
		}
		catch (...)
		{
			LOG_ERR_DTDM("Something wrong on original dx9 objects cleanup. Ignoring that.");
		}
	}
	else {
		LOG_INFO_DTDM("Using no-dx9 startup");

		*origPP = *((D3DPRESENT_PARAMETERS*)initPtr);

		ZeroMemory(&cached_dx9caps, sizeof(D3DCAPS9));
		ZeroMemory(&cached_dx9displaymode, sizeof(D3DDISPLAYMODE));

		//megai2: TODO 
		//fill D3DCAPS9
		//fill D3DDISPLAYMODE

		initPtr = 0;
	}

	LOG_DBG_DTDM("Original DX9 data ackquried");
}

void d912pxy_device::InitVFS()
{
	d912pxy_s.vfs.SetWriteMask((UINT32)d912pxy_s.config.GetValueXI64(PXY_CFG_VFS_WRITE_MASK));

	d912pxy_s.vfs.Init(d912pxy_helper::GetFilePath(FP_VFS_LOCK_FILE)->s);

	if (!d912pxy_s.vfs.IsWriteAllowed())
	{
		LOG_WARN_DTDM("VFS is in read-only mode, no data will be saved on disk");
	}	

	d912pxy_s.vfs.SetRoot(d912pxy_s.config.GetValueRaw(PXY_CFG_VFS_ROOT));

	UINT64 memcacheMask = d912pxy_s.config.GetValueXI64(PXY_CFG_VFS_MEMCACHE_MASK);
	
	d912pxy_s.vfs.LoadVFS();
}

void d912pxy_device::InitClassFields()
{
	ZeroMemory(swapchains, sizeof(intptr_t)*PXY_INNER_MAX_SWAP_CHAINS);

	d912pxy_hlsl_generator::FillHandlers();
	d912pxy_hlsl_generator::allowPP_suffix = d912pxy_s.config.GetValueUI32(PXY_CFG_SDB_ALLOW_PP_SUFFIX);
	d912pxy_hlsl_generator::NaNguard_flag = d912pxy_s.config.GetValueUI32(PXY_CFG_SDB_NAN_GUARD_FLAG);

	d912pxy_vstream::threadedCtor = d912pxy_s.config.GetValueUI32(PXY_CFG_MT_VSTREAM_CTOR);
	d912pxy_surface::threadedCtor = d912pxy_s.config.GetValueUI32(PXY_CFG_MT_SURFACE_CTOR);

	d912pxy_resource::residencyOverride = (d912pxy_s.config.GetValueUI32(PXY_CFG_POOLING_KEEP_RESIDENT) != 0) * 2;
}

void d912pxy_device::InitThreadSyncObjects()
{
	activeThreadCount = 0;
}

void d912pxy_device::InitSingletons()
{	
	d912pxy_s.dx12.que.Init(PXY_INNER_MAX_CLEANUPS_PER_SYNC, PXY_INNER_MAX_IFRAME_CLEANUPS, 0);

	d912pxy_s.render.replay.Init();

	if (!d912pxy_s.config.GetValueUI64(PXY_CFG_REPLAY_BEHAIVOUR))
		LOG_ERR_DTDM("This feature is no longer available, sorry!");

	d912pxy_s.render.db.shader.Init();

	d912pxy_s.render.iframe.Init(m_dheaps);

	d912pxy_s.thread.texld.Init();
	d912pxy_s.thread.bufld.Init();
	d912pxy_s.pool.upload.Init();
	d912pxy_s.pool.vstream.Init();
	d912pxy_s.pool.surface.Init(D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES);
	d912pxy_s.pool.rtds.Init(D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES);
	d912pxy_s.thread.cleanup.Init();	
}

void d912pxy_device::InitComPatches()
{
	if (d912pxy_s.config.GetValueUI32(PXY_CFG_COMPAT_TRACK_RS))
	{
		d912pxy_com_route_set(PXY_COM_ROUTE_DEVICE, PXY_COM_METHOD_DEV_SETRENDERSTATE, &d912pxy_device::com_SetRenderState_Tracked);
		d912pxy_com_route_set(PXY_COM_ROUTE_DEVICE, PXY_COM_METHOD_DEV_SETSAMPLERSTATE, &d912pxy_device::com_SetSamplerState_Tracked);
	}

	if (!d912pxy_s.config.GetValueUI64(PXY_CFG_SDB_KEEP_PAIRS))
	{
		d912pxy_com_route_set(PXY_COM_ROUTE_SHADER, PXY_COM_METHOD_UNK_RELEASE, &d912pxy_shader::com_ReleaseWithPairRemoval);		
	}

	if (d912pxy_s.config.GetValueUI64(PXY_CFG_COMPAT_CLEAR))
	{
		d912pxy_com_route_set(PXY_COM_ROUTE_DEVICE, PXY_COM_METHOD_DEV_CLEAR, &d912pxy_device::com_Clear_Emulated);
	}

	if (d912pxy_s.config.GetValueUI64(PXY_CFG_COMPAT_OMRT_VIEWPORT_RESET))
	{
		d912pxy_com_route_set(PXY_COM_ROUTE_DEVICE, PXY_COM_METHOD_DEV_SETRENDERTARGET, &d912pxy_device::com_SetRenderTarget_Compat);		
	}

	if (d912pxy_s.config.GetValueUI64(PXY_CFG_COMPAT_CPU_API_REDUCTION))
	{	
		d912pxy_com_route_set(PXY_COM_ROUTE_DEVICE, PXY_COM_METHOD_DEV_SETVIEWPORT, &d912pxy_device::com_SetViewport_CAR);
		d912pxy_com_route_set(PXY_COM_ROUTE_DEVICE, PXY_COM_METHOD_DEV_SETSCISSORRECT, &d912pxy_device::com_SetScissorRect_CAR);
		d912pxy_com_route_set(PXY_COM_ROUTE_DEVICE, PXY_COM_METHOD_DEV_SETSTREAMSOURCE, &d912pxy_device::com_SetStreamSource_CAR);
		d912pxy_com_route_set(PXY_COM_ROUTE_DEVICE, PXY_COM_METHOD_DEV_SETINDICES, &d912pxy_device::com_SetIndices_CAR);
	}

	{		
		UINT64 occCfgValue = d912pxy_s.config.GetValueUI64(PXY_CFG_COMPAT_OCCLUSION);

		if ((d912pxy_s.config.GetValueUI32(PXY_CFG_REPLAY_THREADS) > 1) && (occCfgValue > 1))
		{
			LOG_ERR_DTDM("Occlusion query emulation forced to 0 instead of %u due to replay_threads > 1", occCfgValue);
			occCfgValue = 0;
		}

		switch (occCfgValue)
		{
			case 2:
			case 3:
				d912pxy_query_occlusion::InitOccQueryEmulation();				
				break;
			case 1:				
				d912pxy_com_route_set(PXY_COM_ROUTE_QUERY_OCC, PXY_COM_METHOD_QUERY_ISSUE, &d912pxy_query::com_IssueNOP);
				d912pxy_com_route_set(PXY_COM_ROUTE_QUERY_OCC, PXY_COM_METHOD_QUERY_GETDATA, &d912pxy_query::com_GetDataOneOverride);
				break;
			case 0:				
				d912pxy_com_route_set(PXY_COM_ROUTE_QUERY_OCC, PXY_COM_METHOD_QUERY_ISSUE, &d912pxy_query::com_IssueNOP);
				d912pxy_com_route_set(PXY_COM_ROUTE_QUERY_OCC, PXY_COM_METHOD_QUERY_GETDATA, &d912pxy_query::com_GetDataZeroOverride);
				break;
			default:
				LOG_ERR_THROW2(-1, "PXY_CFG_COMPAT_OCCLUSION config entry is bad");
				break;
		}			

		if ((occCfgValue < 2) && d912pxy_s.config.GetValueUI64(PXY_CFG_COMPAT_OCCLUSION_OPT_CONSTRUCTOR))
		{
			d912pxy_com_route_set(PXY_COM_ROUTE_DEVICE, PXY_COM_METHOD_DEV_CREATEQUERY, &d912pxy_device::com_CreateQuery_Optimized);
			CreateQuery(D3DQUERYTYPE_OCCLUSION, &mFakeOccQuery);
		}
		else
			mFakeOccQuery = NULL;
	}

	if (d912pxy_s.config.GetValueUI32(PXY_CFG_COMPAT_BATCH_COMMIT))
	{
		d912pxy_com_route_set(PXY_COM_ROUTE_DEVICE, PXY_COM_METHOD_DEV_DRAWINDEXEDPRIMITIVE, &d912pxy_device::com_DrawIndexedPrimitive_Compat);
		d912pxy_com_route_set(PXY_COM_ROUTE_DEVICE, PXY_COM_METHOD_DEV_DRAWPRIMITIVE, &d912pxy_device::com_DrawPrimitive_Compat);
	}

	if (d912pxy_s.config.GetValueUI32(PXY_CFG_COMPAT_DUP_UNSAFE))
	{
		d912pxy_com_route_set(PXY_COM_ROUTE_DEVICE, PXY_COM_METHOD_DEV_DRAWINDEXEDPRIMITIVEUP, &d912pxy_device::com_DrawIndexedPrimitiveUP_StateUnsafe);
		d912pxy_com_route_set(PXY_COM_ROUTE_DEVICE, PXY_COM_METHOD_DEV_DRAWPRIMITIVEUP, &d912pxy_device::com_DrawPrimitiveUP_StateUnsafe);
	}

	if (d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_ENABLE))
	{
		d912pxy_com_route_set(PXY_COM_ROUTE_DEVICE, PXY_COM_METHOD_DEV_PRESENT, &d912pxy_device::com_Present_Extra);		
	} else if (d912pxy_s.config.GetValueUI32(PXY_CFG_LOG_PERF_GRAPH))
	{		
		d912pxy_com_route_set(PXY_COM_ROUTE_DEVICE, PXY_COM_METHOD_DEV_PRESENT, &d912pxy_device::com_Present_PG);
	}	
}

void d912pxy_device::InitNullSRV()
{
	UINT uuLc = 1;
	mNullTexture = d912pxy_surface::d912pxy_surface_com(1, 1, D3DFMT_A8B8G8R8, 0, D3DMULTISAMPLE_NONE, 0, 0, &uuLc, 6, NULL);
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
}

void d912pxy_device::InitDrawUPBuffers()
{
	d912pxy_s.render.draw_up.Init();

	m_emulatedSurfaceOps = new d912pxy_surface_ops(this);
}

void d912pxy_device::InitDescriptorHeaps()
{
	for (int i = 0; i != PXY_INNER_MAX_DSC_HEAPS; ++i)
	{
		m_dheaps[i] = new d912pxy_dheap(this, i);
	}
}

void d912pxy_device::PrintInfoBanner()
{
	LOG_INFO_DTDM("d912pxy(Direct3D9 to Direct3D12 api proxy) starting up");
	LOG_INFO_DTDM("Original project link: https://github.com/megai2/d912pxy/");
	LOG_INFO_DTDM(BUILD_VERSION_NAME);

#if _WIN64
	LOG_INFO_DTDM("Platform: win x64");	
#elif _WIN32
	LOG_INFO_DTDM("Platform: win x86(32bit)");
#else 
	LOG_INFO_DTDM("Platform: unknown");
#endif

#ifdef __AVX2__	
	LOG_INFO_DTDM("CPU arch: AVX2");
#else 
	#ifdef __AVX__
		LOG_INFO_DTDM("CPU arch: AVX");
    #else
		LOG_INFO_DTDM("CPU arch: SSE");
	#endif	
#endif
	LOG_INFO_DTDM("Batch Limit: %u", d912pxy_s.config.GetValueUI32(PXY_CFG_BATCHING_MAX_BATCHES_PER_IFRAME));
	LOG_INFO_DTDM("Recreation Limit: %u", PXY_INNER_MAX_IFRAME_CLEANUPS);
	LOG_INFO_DTDM("TextureBind Limit: %u", PXY_INNER_MAX_TEXTURE_STAGES);
	LOG_INFO_DTDM("RenderTargets Limit: %u", PXY_INNER_MAX_RENDER_TARGETS);
	LOG_INFO_DTDM("ShaderConst Limit: %u", PXY_INNER_MAX_SHADER_CONSTS);
	LOG_INFO_DTDM("Streams Limit: %u", PXY_INNER_MAX_VBUF_STREAMS);
	LOG_INFO_DTDM("!!!NOT INTENDED TO PERFORM ALL DIRECT3D9 FEATURES!!!");
	LOG_INFO_DTDM("DX9: original display mode width %u height %u", cached_dx9displaymode.Width, cached_dx9displaymode.Height);

	d912pxy_helper::InitLogModule();

	if (d912pxy_s.config.GetValueUI32(PXY_CFG_LOG_ENABLE_VEH))
	{
		LOG_INFO_DTDM("Adding vectored exception handler");
		d912pxy_helper::InstallVehHandler();		
		LOG_INFO_DTDM("dbg messages and errors are now redirected to log file");
	}
	
	OSVERSIONINFOEX info;
	ZeroMemory(&info, sizeof(OSVERSIONINFOEX));
	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if (d912pxy_helper::GetTrueWindowsVersion(&info))
	{
		LOG_INFO_DTDM("OS: v%u.%u %s r%u", info.dwMajorVersion, info.dwMinorVersion, info.szCSDVersion,info.dwBuildNumber);
	}
	else {
		LOG_ERR_DTDM("OS: unknown");
	}
	
	MEMORYSTATUSEX ramInfo;

	ramInfo.dwLength = sizeof(ramInfo);

	if (GlobalMemoryStatusEx(&ramInfo))
	{
		LOG_INFO_DTDM("System RAM info");
		LOG_INFO_DTDM("- phys: %0.3f Gb", (ramInfo.ullTotalPhys >> 20llu)/1024.0f);
		LOG_INFO_DTDM("- commit limit: %0.3f Gb", (ramInfo.ullTotalPageFile >> 20llu)/ 1024.0f);
		LOG_INFO_DTDM("- usable: %0.3f Gb", (ramInfo.ullAvailPageFile >> 20llu) / 1024.0f);
		LOG_INFO_DTDM("- phys usable: %0.3f Gb", (ramInfo.ullAvailPhys >> 20llu) / 1024.0f);
	}
	else {
		LOG_ERR_DTDM("No info about system RAM is available");
	}

	//string includes manufacturer, model and clockspeed
	LOG_INFO_DTDM("CPU: %S", d912pxy_helper::GetCPUBrandString());

	SYSTEM_INFO sysInf = { 0 };
	GetSystemInfo(&sysInf);
	LOG_INFO_DTDM("CPU cores: %u", sysInf.dwNumberOfProcessors);

	cpuCoreCount = sysInf.dwNumberOfProcessors;

	LOG_INFO_DTDM("=========================================== Config data");

	for (int i = 0; i != PXY_CFG_CNT; ++i)
	{
		d912pxy_config_value_dsc* entry = d912pxy_s.config.GetEntryRaw((d912pxy_config_value)i);

		LOG_INFO_DTDM("%s.%s = %s", entry->section, entry->name, entry->value);
	}

	LOG_INFO_DTDM("======================================================");
}

void d912pxy_device::InitDefaultSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	swapchains[0] = &d912pxy_swapchain::d912pxy_swapchain_com(0, pPresentationParameters)->swapchain;

	d912pxy_s.render.iframe.SetSwapper(swapchains[0]);
}

ComPtr<ID3D12Device> d912pxy_device::SelectSuitableGPU()
{
	ComPtr<IDXGIFactory1> dxgiFactory;

#ifdef _DEBUG
	UINT createFactoryFlags = 0;

	if (d912pxy_s.config.GetValueUI32(PXY_CFG_DX_DBG_RUNTIME))
	{
		d912pxy_helper::d3d12_EnableDebugLayer();
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
	}

	LOG_ERR_THROW2(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)), "DXGI factory 2 @ GetAdapter");

#else

	LOG_ERR_THROW2(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)), "DXGI factory @ GetAdapter");

#endif

	ComPtr<IDXGIAdapter1> dxgiAdapter1;
	ComPtr<IDXGIAdapter2> dxgiAdapter4;
	ComPtr<IDXGIAdapter2> gpu = nullptr;

	SIZE_T maxVidmem = 0;
	D3D_FEATURE_LEVEL usingFeatures = D3D_FEATURE_LEVEL_12_1;

	const D3D_FEATURE_LEVEL featureToCreate[] = {
		D3D_FEATURE_LEVEL_9_1,//megai2: should never happen
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	LOG_INFO_DTDM("Enum DXGI adapters");
	{
		for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			LOG_ERR_THROW2(dxgiAdapter1.As(&dxgiAdapter4), "dxgiAdapter 1->4 as");

			DXGI_ADAPTER_DESC2 dxgiAdapterDesc2;
			dxgiAdapter4->GetDesc2(&dxgiAdapterDesc2);

			UINT operational = (dxgiAdapterDesc2.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0;

			//megai2: temporary until all feature level deps found correctly and rewrited to CheckForNeededFeatureLevel()
			const char* flText[] = {
				"not supported           ",
				"FL_12_1 should work 100%",
				"FL_12_0 should work  99%",
				"FL_11_1 should work  80%",
				"FL_11_0 expect problems ",
				"FL_10_1 expect problems ",
				"FL_10_0 expect problems ",
				"FL_9_3 expect problems ",
				"FL_9_2 expect problems ",
				"FL_9_1 expect problems "
			};

			UINT flTestMask = 0;

			if (operational)
			{
				flTestMask = SUCCEEDED(d912pxy_s.imports.dx12.CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), nullptr));
				flTestMask |= SUCCEEDED(d912pxy_s.imports.dx12.CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)) << 1;
				flTestMask |= SUCCEEDED(d912pxy_s.imports.dx12.CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_1, __uuidof(ID3D12Device), nullptr)) << 2;
				flTestMask |= SUCCEEDED(d912pxy_s.imports.dx12.CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) << 3;
				flTestMask |= SUCCEEDED(d912pxy_s.imports.dx12.CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_10_1, __uuidof(ID3D12Device), nullptr)) << 4;
				flTestMask |= SUCCEEDED(d912pxy_s.imports.dx12.CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_10_0, __uuidof(ID3D12Device), nullptr)) << 5;
				flTestMask |= SUCCEEDED(d912pxy_s.imports.dx12.CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_9_3, __uuidof(ID3D12Device), nullptr)) << 6;
				flTestMask |= SUCCEEDED(d912pxy_s.imports.dx12.CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_9_2, __uuidof(ID3D12Device), nullptr)) << 7;
				flTestMask |= SUCCEEDED(d912pxy_s.imports.dx12.CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_9_1, __uuidof(ID3D12Device), nullptr)) << 8;

				while ((flTestMask & 1) == 0)
				{
					flTestMask = flTestMask >> 1;
					++operational;

					if (operational > 9)
					{
						operational = 0;
						break;
					}
				}
			}

			LOG_INFO_DTDM("%u: VRAM: %06u Mb | FL: %S (%u) | %s",
				i,
				(DWORD)(dxgiAdapterDesc2.DedicatedVideoMemory >> 20llu),
				flText[operational], flTestMask,
				dxgiAdapterDesc2.Description
			);

			if (operational && (maxVidmem < dxgiAdapterDesc2.DedicatedVideoMemory))
			{				
				if ((maxVidmem > 0) && (dxgiAdapterDesc2.VendorId == 0x8086))
				{
					LOG_WARN_DTDM("Under prioritized Intel GPU have more VRAM than other non-Intel GPU. Odd, skipping Intel GPU.");
				}
				else {
					maxVidmem = dxgiAdapterDesc2.DedicatedVideoMemory;
					gpu = dxgiAdapter4;

					usingFeatures = featureToCreate[operational];
				}
			}
		}
	}
	LOG_INFO_DTDM("Selecting DXGI adapter by vidmem size");

	if (gpu == nullptr)
	{
		LOG_ERR_THROW2(-1, "No suitable GPU found. Exiting.");
	}

	DXGI_ADAPTER_DESC2 pDesc;
	LOG_ERR_THROW(gpu->GetDesc2(&pDesc));

	gpu_totalVidmemMB = (DWORD)(pDesc.DedicatedVideoMemory >> 20llu);

	LOG_INFO_DTDM("GPU name: %s vidmem: %u Mb", pDesc.Description, gpu_totalVidmemMB);

	//nvidia 
	if (pDesc.VendorId == 0x10de)
	{
		NvGPU_force_highpower();
	}
	else
		nvapi = 0;
	
	for (int i = 0; i != 128; ++i)
	{		
		GPUNameA[i] = (char)pDesc.Description[i];
	}
	
	/*
	DXGI_QUERY_VIDEO_MEMORY_INFO vaMem;
	gpu->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &vaMem);

	LOG_INFO_DTDM("Adapter local memory: BU %u AR %u CR %u CU %u",
		vaMem.Budget >> 20, vaMem.AvailableForReservation >> 20, vaMem.CurrentReservation >> 20, vaMem.CurrentUsage >> 20
	);
	*/

	//megai2: create device actually

	ComPtr<ID3D12Device> ret;
	LOG_ERR_THROW2(d912pxy_s.imports.dx12.CreateDevice(gpu.Get(), usingFeatures, IID_PPV_ARGS(&ret)), "dx12CreateDevice");

	// Enable debug messages in debug mode.
#ifdef _DEBUG
	ComPtr<ID3D12InfoQueue> pInfoQueue;
	if (SUCCEEDED(ret.As(&pInfoQueue)))
	{
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		//pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
		// Suppress whole categories of messages
		//D3D12_MESSAGE_CATEGORY Categories[] = {};

		// Suppress messages based on their severity level
		D3D12_MESSAGE_SEVERITY Severities[] =
		{
			D3D12_MESSAGE_SEVERITY_INFO
		};

		// Suppress individual messages by their ID
		D3D12_MESSAGE_ID DenyIds[] = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};
		//NewFilter.DenyList.NumCategories = _countof(Categories);
		//NewFilter.DenyList.pCategoryList = Categories;
		NewFilter.DenyList.NumSeverities = _countof(Severities);
		NewFilter.DenyList.pSeverityList = Severities;
		NewFilter.DenyList.NumIDs = _countof(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		LOG_ERR_THROW2(pInfoQueue->PushStorageFilter(&NewFilter), "dx12CreateDevice dbg filters");
	}
#endif

	return ret;
}

void d912pxy_device::SetupDevice(ComPtr<ID3D12Device> device)
{
	m_d12evice = device;
	m_d12evice_ptr = m_d12evice.Get();
	
	d912pxy_s.dx12.dev = m_d12evice_ptr;

	D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT vaSizes;
	m_d12evice->CheckFeatureSupport(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, &vaSizes, sizeof(vaSizes));

	LOG_INFO_DTDM("Device virtual address info: BPR %lu BPP %lu",
		1 << (vaSizes.MaxGPUVirtualAddressBitsPerResource - 20), 1 << (vaSizes.MaxGPUVirtualAddressBitsPerProcess - 20)
	);


	D3D12_FEATURE_DATA_CROSS_NODE crossNode;
	if (!FAILED(m_d12evice->CheckFeatureSupport(D3D12_FEATURE_CROSS_NODE, &crossNode, sizeof(crossNode))))
	{
		LOG_INFO_DTDM("Device cross node sharing tier: %u", crossNode.SharingTier);
	} else 
		LOG_INFO_DTDM("Device cross node sharing tier: not available", crossNode.SharingTier);
	

	LOG_INFO_DTDM("Adapter Nodes: %u", m_d12evice->GetNodeCount());
	
	LOG_DBG_DTDM("dev %016llX", m_d12evice.Get());
}

void d912pxy_device::NvGPU_force_highpower()
{
	nvapi = init_nv_api_oc();

	nvapi_dynPstateChanged = 0;

	if (!nvapi)
	{
		LOG_ERR_DTDM("Can't load nv_api!");
		return;
	}

	LOG_INFO_DTDM("============ nv api =============");

	int nGPU = 0, systype = 0, memsize = 0, memtype = 0;
	int *hdlGPU[64] = { 0 };
	char sysname[64] = { 0 }, biosname[64] = { 0 };
	char drvname[64] = { 0 }; NvU32 drv_ver;
	NV_GPU_PERF_PSTATES20_INFO_V1 pstates_info;
	pstates_info.version = 0x11c94;

	nvapi->Init();
	nvapi->EnumGPUs(hdlGPU, &nGPU);
	nvapi->GetSysType(hdlGPU[0], &systype);
	nvapi->GetName(hdlGPU[0], sysname);
	nvapi->GetMemSize(hdlGPU[0], &memsize);
	nvapi->GetMemType(hdlGPU[0], &memtype);
	nvapi->GetBiosName(hdlGPU[0], biosname);
	nvapi->GetPstates(hdlGPU[0], &pstates_info);
	nvapi->GetDriverAndBranchVersion(&drv_ver, drvname);

	LOG_INFO_DTDM("Name: %S", sysname);

	switch (systype) {
		case 1:     LOG_INFO_DTDM("Type: Laptop"); break;
		case 2:     LOG_INFO_DTDM("Type: Desktop"); break;
		default:    LOG_INFO_DTDM("Type: Unknown"); break;
	}
	
	LOG_INFO_DTDM("VRAM: %dMB GDDR%d", memsize / 1024, memtype <= 7 ? 3 : 5);
	LOG_INFO_DTDM("BIOS: %S", biosname);
	LOG_INFO_DTDM("GPU: %dMHz", (int)((pstates_info.pstates[0].clocks[0]).data.range.maxFreq_kHz) / 1000);
	LOG_INFO_DTDM("RAM: %dMHz", (int)((pstates_info.pstates[0].clocks[1]).data.single.freq_kHz) / 1000);
	LOG_INFO_DTDM("Current GPU OC: %dMHz", (int)((pstates_info.pstates[0].clocks[0]).freqDelta_kHz.value) / 1000);
	LOG_INFO_DTDM("Current RAM OC: %dMHz", (int)((pstates_info.pstates[0].clocks[1]).freqDelta_kHz.value) / 1000);
	LOG_INFO_DTDM("Driver: ver %u branch %S", drv_ver, drvname);
	
	if (d912pxy_s.config.GetValueUI32(PXY_CFG_MISC_NV_DISABLE_THROTTLE))
	{
		//megai2: this is not working in terms of "//!< bit 0 indicates if the dynamic Pstate is enabled or not", at least for me
		/*
		NV_GPU_DYNAMIC_PSTATES_INFO_EX dynPstateInfo;

		dynPstateInfo.version = 0x10048;

		if (nvapi->GetDynamicPstatesInfoEx(hdlGPU[0], &dynPstateInfo))
		{
			LOG_ERR_DTDM("Can't get dynamic pstate info");
			return;
		}
		
		if (dynPstateInfo.flags & 1)*/

		{			
			LOG_INFO_DTDM("**UNSAFE** Disabling dynamic power managment");
			if (nvapi->EnableDynamicPstates(hdlGPU[0], 0))
			{
				LOG_ERR_DTDM("Can't disable dynamic pstate, sorry");
				return;
			}
			nvapi_dynPstateChanged = 1;
		}
	}
	
	LOG_INFO_DTDM("=================================");
}

void d912pxy_device::NvGPU_restore()
{
	if (!nvapi)
		return;

	if (nvapi_dynPstateChanged)
	{
		LOG_INFO_DTDM("Restoring dynamic power managment");

		int nGPU = 0;
		int *hdlGPU[64] = { 0 };

		nvapi->EnumGPUs(hdlGPU, &nGPU);

		if (nvapi->EnableDynamicPstates(hdlGPU[0], 1))
		{
			LOG_ERR_DTDM("Can't enable back dynamic pstate! Something is broken, this is very very bad!");
		}
	}

	nvapi->Unload();
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
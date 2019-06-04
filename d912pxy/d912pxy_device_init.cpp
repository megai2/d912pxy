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
		dx9swc->Release();

		if (!origPP->hDeviceWindow)
			origPP->hDeviceWindow = origPars->hFocusWindow;

		if (!origPP->BackBufferHeight)
			origPP->BackBufferHeight = 1;

		if (!origPP->BackBufferWidth)
			origPP->BackBufferWidth = 1;

		LOG_ERR_THROW(dev->GetDeviceCaps(&cached_dx9caps));
		LOG_ERR_THROW(dev->GetDisplayMode(0, &cached_dx9displaymode));

		((IDirect3D9*)initPtr)->AddRef();//megai2: keep original d3d9 object

		dev->Release();
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
	new d912pxy_vfs(d912pxy_vfs_lock_file);

	if (!d912pxy_s(vfs)->IsWriteAllowed())
	{
		LOG_INFO_DTDM("VFS is locked by another process, no data will be saved on disk");
	}

	d912pxy_s(vfs)->SetRoot("./d912pxy/pck");

	InitVFSitem(PXY_VFS_BID_CSO,						"shader_cso",			1);
	InitVFSitem(PXY_VFS_BID_SHADER_PROFILE,				"shader_profiles",		1);
	InitVFSitem(PXY_VFS_BID_PSO_CACHE_KEYS,				"pso_cache",			1);
	InitVFSitem(PXY_VFS_BID_PSO_PRECOMPILE_LIST,		"pso_precompile",		1);
	InitVFSitem(PXY_VFS_BID_SHADER_SOURCES,				"shader_sources",		0);
	InitVFSitem(PXY_VFS_BID_DERIVED_CSO_VS,				"derived_cso_vs",	    1);
	InitVFSitem(PXY_VFS_BID_DERIVED_CSO_PS,				"derived_cso_ps",       1);
}

void d912pxy_device::InitVFSitem(UINT id, const char* name, UINT memCache)
{
	if (!d912pxy_s(vfs)->LoadVFS(id, name, memCache))
	{
		LOG_ERR_DTDM("%S VFS not loaded", name);
		LOG_ERR_THROW2(-1, "VFS error");
	}
}

void d912pxy_device::InitClassFields()
{
	ZeroMemory(swapchains, sizeof(intptr_t)*PXY_INNER_MAX_SWAP_CHAINS);

	d912pxy_hlsl_generator::FillHandlers();
	d912pxy_hlsl_generator::allowPP_suffix = d912pxy_s(config)->GetValueUI32(PXY_CFG_SDB_ALLOW_PP_SUFFIX);

	d912pxy_vstream::threadedCtor = d912pxy_s(config)->GetValueUI32(PXY_CFG_MT_VSTREAM_CTOR);
	d912pxy_surface::threadedCtor = d912pxy_s(config)->GetValueUI32(PXY_CFG_MT_SURFACE_CTOR);
}

void d912pxy_device::InitThreadSyncObjects()
{
	activeThreadCount = 0;
}

void d912pxy_device::InitSingletons()
{	
	new d912pxy_gpu_que(this, PXY_INNER_MAX_CLEANUPS_PER_SYNC, PXY_INNER_MAX_IFRAME_CLEANUPS, 0);

	if (d912pxy_s(config)->GetValueUI64(PXY_CFG_MT_REPLAY_BEHAIVOUR))
		new d912pxy_replay(this);
	else
		new d912pxy_replay_passthru(this);

	new d912pxy_shader_db(this);

	new d912pxy_iframe(this, m_dheaps);
	
	new d912pxy_texture_loader(this);
	new d912pxy_buffer_loader(this);
	new d912pxy_upload_pool(this);
	new d912pxy_vstream_pool(this);
	new d912pxy_surface_pool(this);
	new d912pxy_cleanup_thread(this);
}

void d912pxy_device::InitComPatches()
{
	if (!d912pxy_s(config)->GetValueUI64(PXY_CFG_SDB_KEEP_PAIRS))
	{
		DWORD fakeShaderCode[] = {
			0xFFFF0300, 0x001FFFFE, 0x42415443, 0x0000001C, 0x0000004F, 0xFFFF0300, 0x00000001, 0x0000001C, 0x00000100, 0x00000048,
			0x00000030, 0x00000003, 0x00000001, 0x00000038, 0x00000000, 0x44325F73, 0xABABAB00, 0x000C0004, 0x00010001, 0x00000001,
			0x00000000, 0x335F7370, 0x4D00305F, 0x6F726369, 0x74666F73, 0x29522820, 0x534C4820, 0x6853204C, 0x72656461, 0x6D6F4320,
			0x656C6970, 0x30312072, 0xAB00312E, 0x0200001F, 0x8000000A, 0x900F0000, 0x0200001F, 0x80000005, 0x90030001, 0x0200001F,
			0x90000000, 0xA00F0800, 0x03000042, 0x800F0000, 0x90E40001, 0xA0E40800, 0x03000005, 0x800F0800, 0x80E40000, 0x90E40000,
			0x0000FFFF
		};

		d912pxy_vshader* vsPatch = new d912pxy_vshader(this, fakeShaderCode);
		d912pxy_com_set_method((IDirect3DVertexShader9*)vsPatch, 2, &d912pxy_vshader::ReleaseWithPairRemoval);
		vsPatch->Release();

		d912pxy_pshader* psPatch = new d912pxy_pshader(this, fakeShaderCode);
		d912pxy_com_set_method((IDirect3DPixelShader9*)psPatch, 2, &d912pxy_pshader::ReleaseWithPairRemoval);
		psPatch->Release();
	}

	if (!d912pxy_s(config)->GetValueUI64(PXY_CFG_QUERY_OCCLUSION))
	{
		d912pxy_query* queryObj = new d912pxy_query(this, D3DQUERYTYPE_OCCLUSION);
		d912pxy_com_set_method((IDirect3DQuery9*)queryObj, 7, &d912pxy_query::GetDataZeroOverride);

		queryObj->Release();
	}

	if (d912pxy_s(config)->GetValueUI32(PXY_CFG_SDB_ENABLE_PROFILING))
	{
		d912pxy_com_set_method((IDirect3DDevice9*)this, 0x41, &d912pxy_device::SetTexture_PS);
		d912pxy_com_set_method((IDirect3DDevice9*)this, 0x52, &d912pxy_device::DrawIndexedPrimitive_PS);
	}

	if (d912pxy_s(config)->GetValueUI32(PXY_CFG_LOG_PERF_GRAPH))
	{
		d912pxy_com_set_method((IDirect3DDevice9*)this, 0x11, &d912pxy_device::Present_PG);
	}
}

void d912pxy_device::InitNullSRV()
{
	UINT uuLc = 1;
	mNullTexture = new d912pxy_surface(this, 1, 1, D3DFMT_A8B8G8R8, 0, D3DMULTISAMPLE_NONE, 0, 0, &uuLc, 6, NULL);
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
	UINT32 tmpUPbufSpace = d912pxy_s(config)->GetValueXI64(PXY_CFG_MISC_DRAW_UP_BUFFER_LENGTH) & 0xFFFFFFFF;

	mDrawUPVbuf = d912pxy_s(pool_vstream)->GetVStreamObject(tmpUPbufSpace, 0, 0)->AsDX9VB();
	mDrawUPIbuf = d912pxy_s(pool_vstream)->GetVStreamObject(tmpUPbufSpace * 4, D3DFMT_INDEX32, 1)->AsDX9IB();

	UINT32* ibufDt;
	mDrawUPIbuf->Lock(0, 0, (void**)&ibufDt, 0);

	for (int i = 0; i != tmpUPbufSpace; ++i)
	{
		ibufDt[i] = i;
	}

	mDrawUPIbuf->Unlock();
	mDrawUPStreamPtr = 0;
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
	LOG_INFO_DTDM("d912pxy(Direct3D9 to Direct3D12 api proxy) loaded");
	LOG_INFO_DTDM(BUILD_VERSION_NAME);
	LOG_INFO_DTDM("Batch Limit: %u", PXY_INNER_MAX_IFRAME_BATCH_COUNT);
	LOG_INFO_DTDM("Recreation Limit: %u", PXY_INNER_MAX_IFRAME_CLEANUPS);
	LOG_INFO_DTDM("TextureBind Limit: %u", PXY_INNER_MAX_TEXTURE_STAGES);
	LOG_INFO_DTDM("RenderTargets Limit: %u", PXY_INNER_MAX_RENDER_TARGETS);
	LOG_INFO_DTDM("ShaderConst Limit: %u", PXY_INNER_MAX_SHADER_CONSTS);
	LOG_INFO_DTDM("Streams Limit: %u", PXY_INNER_MAX_VBUF_STREAMS);
	LOG_INFO_DTDM("!!!NOT INTENDED TO PERFORM ALL DIRECT3D9 FEATURES!!!");
	LOG_INFO_DTDM("DX9: original display mode width %u height %u", cached_dx9displaymode.Width, cached_dx9displaymode.Height);

	LOG_INFO_DTDM("Redirecting debug messages to P7");
	LOG_INFO_DTDM("Adding vectored exception handler");
	d912pxy_helper::InstallVehHandler();


	if (d912pxy_s(config)->GetValueUI32(PXY_CFG_SDB_ENABLE_PROFILING))
		LOG_INFO_DTDM("Running ps build, expect performance drops");

	UINT64 memKb = 0;

	if (GetPhysicallyInstalledSystemMemory(&memKb))
	{
		LOG_INFO_DTDM("System physical RAM size: %llu Gb", memKb >> 20llu);
	}

	//string includes manufacturer, model and clockspeed
	LOG_INFO_DTDM("CPU: %S", d912pxy_helper::GetCPUBrandString());

	SYSTEM_INFO sysInf = { 0 };
	GetSystemInfo(&sysInf);
	LOG_INFO_DTDM("CPU cores: %u", sysInf.dwNumberOfProcessors);

	LOG_INFO_DTDM("=========================================== Config data");

	for (int i = 0; i != PXY_CFG_CNT; ++i)
	{
		d912pxy_config_value_dsc* entry = d912pxy_s(config)->GetEntryRaw((d912pxy_config_value)i);

		LOG_INFO_DTDM("%s.%s = %s", entry->section, entry->name, entry->value);
	}

	LOG_INFO_DTDM("======================================================");
}

void d912pxy_device::InitDefaultSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	swapchains[0] = new d912pxy_swapchain(
		this,
		0,
		pPresentationParameters
	);

	d912pxy_s(iframe)->SetSwapper(swapchains[0]);
}

ComPtr<ID3D12Device> d912pxy_device::SelectSuitableGPU()
{
	ComPtr<IDXGIFactory4> dxgiFactory;
	UINT createFactoryFlags = 0;

	if (d912pxy_s(config)->GetValueUI32(PXY_CFG_DX_DBG_RUNTIME))
	{
		d912pxy_helper::d3d12_EnableDebugLayer();
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
	}

	LOG_ERR_THROW2(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)), "DXGI factory @ GetAdapter");

	ComPtr<IDXGIAdapter1> dxgiAdapter1;
	ComPtr<IDXGIAdapter3> dxgiAdapter4;
	ComPtr<IDXGIAdapter3> gpu = nullptr;

	SIZE_T maxVidmem = 0;
	D3D_FEATURE_LEVEL usingFeatures = D3D_FEATURE_LEVEL_12_1;

	const D3D_FEATURE_LEVEL featureToCreate[] = {
		D3D_FEATURE_LEVEL_9_1,//megai2: should never happen
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
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
				"FL_11_0 expect problems "
			};

			if (operational)
			{
				operational = SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), nullptr));
				operational |= SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)) << 1;
				operational |= SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_1, __uuidof(ID3D12Device), nullptr)) << 2;
				operational |= SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) << 3;

				switch (operational)
				{
				case 0xF:
					operational = 1;
					break;
				case 0xE:
					operational = 2;
					break;
				case 0xC:
					operational = 3;
					break;
				case 0x8:
					operational = 4;
					break;
				default:
					operational = 0;
					break;
				}
			}

			LOG_INFO_DTDM("%u: VRAM: %06u Mb | FL: %S | %s",
				i,
				(DWORD)(dxgiAdapterDesc2.DedicatedVideoMemory >> 20llu),
				flText[operational],
				dxgiAdapterDesc2.Description
			);

			if (operational && (maxVidmem < dxgiAdapterDesc2.DedicatedVideoMemory))
			{
				maxVidmem = dxgiAdapterDesc2.DedicatedVideoMemory;
				gpu = dxgiAdapter4;

				usingFeatures = featureToCreate[operational];
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
	
	for (int i = 0; i != 128; ++i)
	{		
		GPUNameA[i] = (char)pDesc.Description[i];
	}
	
	DXGI_QUERY_VIDEO_MEMORY_INFO vaMem;
	gpu->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &vaMem);

	LOG_INFO_DTDM("Adapter local memory: BU %u AR %u CR %u CU %u",
		vaMem.Budget >> 20, vaMem.AvailableForReservation >> 20, vaMem.CurrentReservation >> 20, vaMem.CurrentUsage >> 20
	);

	//megai2: create device actually

	ComPtr<ID3D12Device> ret;
	LOG_ERR_THROW2(D3D12CreateDevice(gpu.Get(), usingFeatures, IID_PPV_ARGS(&ret)), "D3D12CreateDevice");

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

		LOG_ERR_THROW2(pInfoQueue->PushStorageFilter(&NewFilter), "D3D12CreateDevice dbg filters");
	}
#endif

	return ret;
}

void d912pxy_device::SetupDevice(ComPtr<ID3D12Device> device)
{
	m_d12evice = device;
	m_d12evice_ptr = m_d12evice.Get();
	d912pxy_s(DXDev) = m_d12evice_ptr;

	D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT vaSizes;
	m_d12evice->CheckFeatureSupport(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, &vaSizes, sizeof(vaSizes));

	LOG_INFO_DTDM("Device virtual address info: BPR %lu BPP %lu",
		1 << (vaSizes.MaxGPUVirtualAddressBitsPerResource - 20), 1 << (vaSizes.MaxGPUVirtualAddressBitsPerProcess - 20)
	);

	LOG_INFO_DTDM("Adapter Nodes: %u", m_d12evice->GetNodeCount());

	LOG_DBG_DTDM("dev %016llX", m_d12evice.Get());
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
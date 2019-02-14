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

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_DEVICE

using namespace Microsoft::WRL;

d912pxy_device::d912pxy_device(IDirect3DDevice9* dev, void* par) : d912pxy_comhandler(L"device")
{
	d912pxy_s(dev) = this;
	initPtr = par;

	CopyOriginalDX9Data(dev, &creationData, &initialPresentParameters);	

	PrintInfoBanner();

#ifdef ENABLE_METRICS
	new d912pxy_metrics(this);
	FRAME_METRIC_PRESENT(1)
#endif

#ifdef PERFORMANCE_GRAPH_WRITE
	perfGraph = new d912pxy_performance_graph(0);
#endif

	LOG_INFO_DTDM2(InitClassFields(),									"Startup step 1/9");
	LOG_INFO_DTDM2(InitVFS(),											"Startup step 2/9");
	LOG_INFO_DTDM2(InitThreadSyncObjects(),								"Startup step 3/9");
	LOG_INFO_DTDM2(SetupDevice(SelectSuitableGPU()),					"Startup step 4/9");
	LOG_INFO_DTDM2(InitDescriptorHeaps(),								"Startup step 5/9");
	LOG_INFO_DTDM2(InitSingletons(),									"Startup step 6/9");
	LOG_INFO_DTDM2(InitNullSRV(),										"Startup step 7/9");
	LOG_INFO_DTDM2(InitDrawUPBuffers(),									"Startup step 8/9");
	LOG_INFO_DTDM2(InitDefaultSwapChain(&initialPresentParameters),		"Startup step 9/9");
	LOG_INFO_DTDM2(d912pxy_s(iframe)->Start(),							"Started first IFrame");	
}

d912pxy_device::~d912pxy_device(void)
{	
	LOG_INFO_DTDM("d912pxy exiting");

	LOG_INFO_DTDM2(d912pxy_s(CMDReplay)->Finish(),   "Replayer finished");
	LOG_INFO_DTDM2(d912pxy_s(iframe)->End(),		 "Last iframe ended");
	LOG_INFO_DTDM2(FreeAdditionalDX9Objects(),		 "Additional DX9 objects freed");
	LOG_INFO_DTDM2(d912pxy_s(GPUque)->Flush(0),      "Last gpu cmd lists flushed");
	LOG_INFO_DTDM2(swapchains[0]->Release(),		 "Swapchain stopped");

	//megai2: we have some tree like deletions of objects, so we must call this multiple times
	for (int i = 0; i != 100; ++i)
		d912pxy_s(GPUque)->Flush(0);

	LOG_INFO_DTDM("Pending GPU cleanups processed");
		
	LOG_INFO_DTDM2(delete d912pxy_s(bufloadThread),		"Final cleanups  1/11");
	LOG_INFO_DTDM2(delete d912pxy_s(iframe),			"Final cleanups  2/12");
	LOG_INFO_DTDM2(delete d912pxy_s(sdb),				"Final cleanups  3/12");
	LOG_INFO_DTDM2(delete d912pxy_s(thread_cleanup),	"Final cleanups  4/12");
	LOG_INFO_DTDM2(delete d912pxy_s(pool_vstream),		"Final cleanups  5/12");
	LOG_INFO_DTDM2(delete d912pxy_s(pool_upload),		"Final cleanups  6/12");
	LOG_INFO_DTDM2(delete d912pxy_s(pool_surface),		"Final cleanups  7/12");
	LOG_INFO_DTDM2(delete d912pxy_s(GPUque),			"Final cleanups  8/12");
	LOG_INFO_DTDM2(delete d912pxy_s(CMDReplay),			"Final cleanups  9/12");
	LOG_INFO_DTDM2(delete d912pxy_s(texloadThread),		"Final cleanups 10/12");
	LOG_INFO_DTDM2(delete d912pxy_s(vfs),				"Final cleanups 11/12");
		
	for (int i = 0; i != PXY_INNER_MAX_DSC_HEAPS; ++i)
		delete m_dheaps[i];

	LOG_INFO_DTDM("Final cleanups 12/12");
	
#ifdef ENABLE_METRICS
	delete d912pxy_s(metrics);
#endif

#ifdef PERFORMANCE_GRAPH_WRITE
	delete perfGraph;
#endif
	
	LOG_INFO_DTDM("d912pxy exited");

#ifdef _DEBUG
	d912pxy_helper::d3d12_ReportLeaks();
#endif
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
		dx9swc->Release();

		if (!origPP->hDeviceWindow)
			origPP->hDeviceWindow = origPars->hFocusWindow;

		if (!origPP->BackBufferHeight)
			origPP->BackBufferHeight = 1;

		if (!origPP->BackBufferWidth)
			origPP->BackBufferWidth = 1;

		LOG_ERR_THROW(dev->GetDeviceCaps(&cached_dx9caps));
		LOG_ERR_THROW(dev->GetDisplayMode(0, &cached_dx9displaymode));

		dev->Release();
	} else {
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
}

void d912pxy_device::InitClassFields()
{
	ZeroMemory(swapchains, sizeof(intptr_t)*PXY_INNER_MAX_SWAP_CHAINS);
}

void d912pxy_device::InitThreadSyncObjects()
{
	for (int i = 0; i != PXY_INNER_THREADID_MAX; ++i)
	{
		InitializeCriticalSection(&threadLockdEvents[i]);
	}
	InitializeCriticalSection(&threadLock);
	InitializeCriticalSection(&cleanupLock);
}

void d912pxy_device::InitSingletons()
{
	new d912pxy_gpu_que(this, 2, PXY_INNER_MAX_CLEANUPS_PER_SYNC, PXY_INNER_MAX_IFRAME_CLEANUPS, 0);
	new d912pxy_replay(this);
	new d912pxy_shader_db(this);

	new d912pxy_iframe(this, m_dheaps);
	d912pxy_s(textureState)->SetStatePointer(&mTextureState);

	new d912pxy_texture_loader(this);
	new d912pxy_buffer_loader(this);
	new d912pxy_upload_pool(this);
	new d912pxy_vstream_pool(this);
	new d912pxy_surface_pool(this);
	new d912pxy_cleanup_thread(this);
}

void d912pxy_device::InitNullSRV()
{
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
}

void d912pxy_device::InitDrawUPBuffers()
{
	UINT32 tmpUPbufSpace = 0xFFFF;

	mDrawUPVbuf = d912pxy_s(pool_vstream)->GetVStreamObject(tmpUPbufSpace, 0, 0)->AsDX9VB();
	mDrawUPIbuf = d912pxy_s(pool_vstream)->GetVStreamObject(tmpUPbufSpace * 2, D3DFMT_INDEX16, 1)->AsDX9IB();

	UINT16* ibufDt;
	mDrawUPIbuf->Lock(0, 0, (void**)&ibufDt, 0);

	for (int i = 0; i != tmpUPbufSpace; ++i)
	{
		ibufDt[i] = i;
	}

	mDrawUPIbuf->Unlock();
	mDrawUPStreamPtr = 0;
}

void d912pxy_device::FreeAdditionalDX9Objects()
{
	mDrawUPIbuf->Release();
	mDrawUPVbuf->Release();
	mNullTexture->Release();
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

#ifdef _DEBUG
	LOG_INFO_DTDM("Redirecting DX12 and DXGI debug messages to P7");	
	d912pxy_helper::InstallVehHandler();
#endif

#ifdef TRACK_SHADER_BUGS_PROFILE
	LOG_INFO_DTDM("Running ps build, expect performance drops");
#endif

	UINT64 memKb = 0;

	if (GetPhysicallyInstalledSystemMemory(&memKb))
	{
		LOG_INFO_DTDM("System physical RAM size: %llu Gb", memKb >> 20llu);
	}
	
	int CPUInfo[4] = { -1 };
	unsigned   nExIds, i = 0;
	char CPUBrandString[0x40];
	// Get the information associated with each extended ID.
	__cpuid(CPUInfo, 0x80000000);
	nExIds = CPUInfo[0];
	for (i = 0x80000000; i <= nExIds; ++i)
	{
		__cpuid(CPUInfo, i);
		// Interpret CPU brand string
		if (i == 0x80000002)
			memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000003)
			memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000004)
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
	}
	//string includes manufacturer, model and clockspeed
	LOG_INFO_DTDM("CPU: %S", CPUBrandString);

	SYSTEM_INFO sysInf = { 0 };
	GetSystemInfo(&sysInf);
	LOG_INFO_DTDM("CPU cores: %u", sysInf.dwNumberOfProcessors);
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
	d912pxy_helper::d3d12_EnableDebugLayer();

	ComPtr<IDXGIFactory4> dxgiFactory;
	UINT createFactoryFlags = 0;
#ifdef _DEBUG
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

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
			const char* flText []= {
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
	
	m_log->P7_INFO(LGC_DEFAULT, TM("GPU name: %s vidmem: %u Mb"), pDesc.Description, gpu_totalVidmemMB);		

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

	m_log->P7_INFO(LGC_DEFAULT, TM("Adapter Nodes: %u"), m_d12evice->GetNodeCount());

	LOG_DBG_DTDM("dev %016llX", m_d12evice.Get());
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
		if (initialPresentParameters.Windowed)
			memcpy(pMode, &cached_dx9displaymode, sizeof(D3DDISPLAYMODE));
		else
		{
			pMode->Width = initialPresentParameters.BackBufferWidth;
			pMode->Height = initialPresentParameters.BackBufferHeight;
			pMode->RefreshRate = initialPresentParameters.FullScreen_RefreshRateInHz;
			pMode->Format = initialPresentParameters.BackBufferFormat;
		}
	} 
	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	LOG_DBG_DTDM(__FUNCTION__);

	if (!pParameters)
		return D3DERR_INVALIDCALL;

	*pParameters = creationData;

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
		
		swapchains[i] = new d912pxy_swapchain(this, i, pPresentationParameters);

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

	d912pxy_s(iframe)->End();
	d912pxy_s(GPUque)->Flush(0);

	HRESULT ret = swapchains[0]->SetPresentParameters(pPresentationParameters);
	
	d912pxy_s(iframe)->Start();

	API_OVERHEAD_TRACK_END(0)
		
	return ret; 
}

HRESULT WINAPI d912pxy_device::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)
	d912pxy_s(iframe)->End();
	API_OVERHEAD_TRACK_END(0)	
	FRAME_METRIC_PRESENT(0)

	LOG_DBG_DTDM2("Present Exec GPU");
	HRESULT ret = d912pxy_s(GPUque)->ExecuteCommands(1);

#ifdef PERFORMANCE_GRAPH_WRITE
	perfGraph->RecordPresent(d912pxy_s(iframe)->GetBatchCount());
#endif

#ifdef ENABLE_METRICS
	d912pxy_s(metrics)->TrackDrawCount(d912pxy_s(iframe)->GetBatchCount());
	d912pxy_s(metrics)->FlushIFrameValues();	
#endif 
	
	FRAME_METRIC_PRESENT(1)
	API_OVERHEAD_TRACK_START(0)
	mDrawUPStreamPtr = 0;
	d912pxy_s(iframe)->Start();
	API_OVERHEAD_TRACK_END(0)	

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

	swapchains[iSwapChain]->SetGammaRamp(Flags, pRamp);

	API_OVERHEAD_TRACK_END(0)
}

void WINAPI d912pxy_device::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	swapchains[iSwapChain]->GetGammaRamp(pRamp);

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

	d912pxy_s(iframe)->BindSurface(1 + RenderTargetIndex, rtSurf);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppRenderTarget = d912pxy_s(iframe)->GetBindedSurface(RenderTargetIndex + 1);
	(*ppRenderTarget)->AddRef();

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{ 
	LOG_DBG_DTDM("depth surface set to %016llX", pNewZStencil);
	
	API_OVERHEAD_TRACK_START(0)

	d912pxy_s(iframe)->BindSurface(0, (d912pxy_surface*)pNewZStencil);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppZStencilSurface = d912pxy_s(iframe)->GetBindedSurface(0);
	(*ppZStencilSurface)->AddRef();	

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

//scissors

HRESULT WINAPI d912pxy_device::SetScissorRect(CONST RECT* pRect) 
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	
	API_OVERHEAD_TRACK_START(0)

	d912pxy_s(iframe)->SetScissors((D3D12_RECT*)pRect);

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

	d912pxy_s(iframe)->SetViewport(&main_viewport);

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

	/*if (State > D3DRS_BLENDOPALPHA)
		return D3DERR_INVALIDCALL;*/

	switch (State)
	{
		case D3DRS_ENABLE_D912PXY_API_HACKS:
			return 343434;
		break;
		case D3DRS_STENCILREF:
			d912pxy_s(CMDReplay)->OMStencilRef(Value);
		break; //57,   /* Reference value used in stencil test */

		case D3DRS_BLENDFACTOR:
		{
			DWORD Color = Value;

			float fvClra[4];

			for (int i = 0; i != 4; ++i)
			{
				fvClra[i] = ((Color >> (i << 3)) & 0xFF) / 255.0f;
			}

			d912pxy_s(CMDReplay)->OMBlendFac(fvClra);
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
	
	switch (State)
	{
	case D3DRS_D912PXY_ENQUEUE_PSO_COMPILE:
		d912pxy_s(psoCache)->UseWithFeedbackPtr((void**)pValue);
		break;
	case D3DRS_D912PXY_SETUP_PSO:
		d912pxy_s(psoCache)->UseCompiled((d912pxy_pso_cache_item*)pValue);
		break;
	default:
		*pValue = d912pxy_s(psoCache)->GetDX9RsValue(State);
		return D3D_OK;
	}

	return D3D_OK;
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
			((Color >> 0) & 0xFF) / 255.0f,
			((Color >> 8) & 0xFF) / 255.0f,
			((Color >> 16) & 0xFF) / 255.0f
		};

		d912pxy_surface* surf = d912pxy_s(iframe)->GetBindedSurface(1);

		if (surf)
			d912pxy_s(CMDReplay)->RTClear(surf, fvColor);
			//iframe->GetBindedSurface(1)->d912_rtv_clear(fvColor, Count, (D3D12_RECT*)pRects);//megai2: rect is 4 uint structure, may comply
	}

	if (Flags & (D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER))
	{
		DWORD cvtCf = ((D3D12_CLEAR_FLAG_DEPTH * ((Flags & D3DCLEAR_ZBUFFER) != 0)) | (D3D12_CLEAR_FLAG_STENCIL * ((Flags & D3DCLEAR_STENCIL) != 0)));

		d912pxy_surface* surf = d912pxy_s(iframe)->GetBindedSurface(0);

		if (surf)
			d912pxy_s(CMDReplay)->DSClear(surf, Z, Stencil & 0xFF, (D3D12_CLEAR_FLAGS)cvtCf);

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

	*ppShader = (IDirect3DVertexShader9*)(new d912pxy_vshader(this, pFunction));

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

	*ppShader = (IDirect3DPixelShader9*)(new d912pxy_pshader(this, pFunction));

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

	d912pxy_s(iframe)->SetVBuf((d912pxy_vbuf*)pStreamData, StreamNumber, OffsetInBytes, Stride);

	API_OVERHEAD_TRACK_END(0)
	
	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::SetStreamSourceFreq(UINT StreamNumber, UINT Divider)
{ 
	API_OVERHEAD_TRACK_START(0)

	LOG_DBG_DTDM("stream %u div %u", StreamNumber, Divider);

	if (StreamNumber >= PXY_INNER_MAX_VBUF_STREAMS)
		return D3DERR_INVALIDCALL;

	d912pxy_s(iframe)->SetStreamFreq(StreamNumber, Divider);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	if (pIndexData)
		d912pxy_s(iframe)->SetIBuf((d912pxy_ibuf*)pIndexData);

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

HRESULT WINAPI d912pxy_device::GetDirect3D(IDirect3D9 ** ppv)
{
	*ppv = (IDirect3D9*)initPtr;
	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	LOG_DBG_DTDM3(__FUNCTION__);

	//megai2: hacky way to fix dxgi backbuffer format change
	switch (Format)
	{
		case D3DFMT_X8R8G8B8:
		case D3DFMT_UNKNOWN:
			Format = D3DFMT_A8R8G8B8;
		break;
	}

	UINT levels = 1;
	d912pxy_surface* ret = new d912pxy_surface(this, Width, Height, Format, D3DUSAGE_D912PXY_FORCE_RT, &levels, 1);

	*ppSurface = (IDirect3DSurface9*)ret;

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) 
{	
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	swapchains[iSwapChain]->GetFrontBufferData(pDestSurface);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) 
{ 	
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	d912pxy_surface* src = (d912pxy_surface*)pRenderTarget;
	d912pxy_surface* dst = (d912pxy_surface*)pDestSurface;
	src->CopyTo(dst, 0, d912pxy_s(GPUcl)->GID(CLG_SEQ).Get());
	
	dst->CopySurfaceDataToCPU();

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
	API_OVERHEAD_TRACK_START(0)

	d912pxy_s(CMDReplay)->StretchRect((d912pxy_surface*)pSourceSurface, (d912pxy_surface*)pDestSurface);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::SetClipPlane(DWORD Index, CONST float* pPlane)
{
	API_OVERHEAD_TRACK_START(0)

	d912pxy_s(batch)->SetShaderConstF(1, PXY_INNER_MAX_SHADER_CONSTS_IDX - 2 - Index, 1, (float*)pPlane);  return D3D_OK;

	API_OVERHEAD_TRACK_END(0)
}



//UNIMPLEMENTED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

HRESULT WINAPI d912pxy_device::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) { LOG_DBG_DTDM(__FUNCTION__); return D3DERR_INVALIDCALL; }
HRESULT WINAPI d912pxy_device::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) { LOG_DBG_DTDM(__FUNCTION__); return D3DERR_INVALIDCALL; }
HRESULT WINAPI d912pxy_device::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) { LOG_DBG_DTDM(__FUNCTION__); return D3DERR_INVALIDCALL; }

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
	
	d912pxy_ibuf* oi = d912pxy_s(iframe)->GetIBuf();
	d912pxy_device_streamsrc oss = d912pxy_s(iframe)->GetStreamSource(0);
	d912pxy_device_streamsrc ossi = d912pxy_s(iframe)->GetStreamSource(1);
	
	d912pxy_s(iframe)->SetIBuf((d912pxy_ibuf*)mDrawUPIbuf);
	d912pxy_s(iframe)->SetVBuf((d912pxy_vbuf*)mDrawUPVbuf, 0, mDrawUPStreamPtr, VertexStreamZeroStride);
	d912pxy_s(iframe)->SetStreamFreq(0, 1);
	d912pxy_s(iframe)->SetStreamFreq(1, 0);

	mDrawUPStreamPtr += PrimitiveCount * 3 * VertexStreamZeroStride;

	DrawIndexedPrimitive(PrimitiveType, 0, 0, 0, 0, PrimitiveCount);
	
	d912pxy_s(iframe)->SetIBuf(oi);
	d912pxy_s(iframe)->SetVBuf(oss.buffer, 0, oss.offset, oss.stride);
	d912pxy_s(iframe)->SetStreamFreq(0, oss.divider);
	d912pxy_s(iframe)->SetStreamFreq(1, ossi.divider);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) { 
	LOG_DBG_DTDM(__FUNCTION__); return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::SetFVF(DWORD FVF) { LOG_DBG_DTDM(__FUNCTION__); return 0; }
HRESULT WINAPI d912pxy_device::GetFVF(DWORD* pFVF) { LOG_DBG_DTDM(__FUNCTION__); return 0; }

////////////////////////////////////////////

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
	d912pxy_s(GPUque)->EnqueueCleanup(obj);
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
	EnterCriticalSection(&threadLockdEvents[thread]);
}

void d912pxy_device::LockAsyncThreads()
{
	FRAME_METRIC_SYNC(1)

	EnterCriticalSection(&threadLock);

	InterlockedIncrement(&threadInterruptState);

	d912pxy_s(texloadThread)->SignalWork();
	d912pxy_s(bufloadThread)->SignalWork();
	d912pxy_s(CMDReplay)->Finish();
	//iframe->PSO()->SignalWork();

	for (int i = 0; i != PXY_INNER_THREADID_MAX; ++i)
	{
		EnterCriticalSection(&threadLockdEvents[i]);			
	}
	
	FRAME_METRIC_SYNC(0)
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

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
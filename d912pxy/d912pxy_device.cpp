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
	PrintInfoBanner();

	initPtr = par;
	CopyOriginalDX9Data(dev, &creationData, &initialPresentParameters);	

#ifdef ENABLE_METRICS
	new d912pxy_metrics(this);
	FRAME_METRIC_PRESENT(1)
#endif

#ifdef PERFORMANCE_GRAPH_WRITE
	perfGraph = new d912pxy_performance_graph(0);
#endif

	LOG_INFO_DTDM2(InitClassFields(),									"Startup step  1/10");
	LOG_INFO_DTDM2(InitVFS(),											"Startup step  2/10");
	LOG_INFO_DTDM2(InitThreadSyncObjects(),								"Startup step  3/10");
	LOG_INFO_DTDM2(SetupDevice(SelectSuitableGPU()),					"Startup step  4/10");
	LOG_INFO_DTDM2(InitDescriptorHeaps(),								"Startup step  5/10");
	LOG_INFO_DTDM2(InitSingletons(),									"Startup step  6/10");
	LOG_INFO_DTDM2(InitComPatches(),									"Startup step  7/10");
	LOG_INFO_DTDM2(InitNullSRV(),										"Startup step  8/10");
	LOG_INFO_DTDM2(InitDrawUPBuffers(),									"Startup step  9/10");
	LOG_INFO_DTDM2(InitDefaultSwapChain(&initialPresentParameters),		"Startup step 10/10");
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

	if (initPtr)
		((IDirect3D9*)initPtr)->Release();
	
	LOG_INFO_DTDM("d912pxy exited");

#ifdef _DEBUG
	d912pxy_helper::d3d12_ReportLeaks();
#endif
}

void d912pxy_device::FreeAdditionalDX9Objects()
{
	mDrawUPIbuf->Release();
	mDrawUPVbuf->Release();
	mNullTexture->Release();
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

UINT WINAPI d912pxy_device::GetAvailableTextureMem(void)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	return gpu_totalVidmemMB; 
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

void WINAPI d912pxy_device::SetCursorPosition(int X, int Y, DWORD Flags)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	SetCursorPos(X, Y);	 
}

HRESULT WINAPI d912pxy_device::ValidateDevice(DWORD* pNumPasses)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	//megai2: pretend we can do anything! YES!
	*pNumPasses = 1;
	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::GetDirect3D(IDirect3D9 ** ppv)
{
	*ppv = (IDirect3D9*)initPtr;
	(*ppv)->AddRef();
	return D3D_OK;
}

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

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
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

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_DEVICE

using namespace Microsoft::WRL;

d912pxy_com_object * d912pxy_device::d912pxy_device_com(void* baseMem, IDirect3DDevice9 * dev, void * par)
{
	size_t objSz = sizeof(d912pxy_device);

	d912pxy_com_object* ret = (d912pxy_com_object*)baseMem;
	ZeroMemory(ret, objSz);

	new (&ret->device)d912pxy_device();
	ret->vtable = d912pxy_com_route_get_vtable(PXY_COM_ROUTE_DEVICE);

	ret->device.Init(dev, par);
	
	return ret;
}

d912pxy_device::d912pxy_device() : d912pxy_comhandler()
{
}

d912pxy_device::~d912pxy_device(void)
{	
}

void d912pxy_device::UnInit()
{
	LOG_INFO_DTDM("Device last reference removal");

	NvGPU_restore();

	LOG_INFO_DTDM("d912pxy exiting");
	isRunning.SetValue(0);

	swapOpLock.Hold();

	LOG_INFO_DTDM2(d912pxy_s.render.iframe.End(), "Last iframe ended");
	LOG_INFO_DTDM2(FreeAdditionalDX9Objects(), "Additional DX9 objects freed");
	LOG_INFO_DTDM2(d912pxy_s.dx12.que.Flush(0), "Last gpu cmd lists flushed");
	LOG_INFO_DTDM2(swapchains[0]->ReleaseSwapChain(), "Swapchain stopped");

	if (d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_ENABLE))
		d912pxy_s.extras.UnInit();

	d912pxy_s.dev_vtable = NULL;

	LOG_INFO_DTDM("Pending GPU cleanups processed");

	LOG_INFO_DTDM2(d912pxy_s.thread.cleanup.UnInit(), "Final cleanups  1/13");
	swapOpLock.Release();

	LOG_INFO_DTDM2(d912pxy_s.thread.bufld.UnInit(), "Final cleanups  2/13");
	LOG_INFO_DTDM2(d912pxy_s.thread.texld.UnInit(), "Final cleanups  3/13");
	LOG_INFO_DTDM2(d912pxy_s.render.db.pso.UnInit(), "Final cleanups  4/13");
	LOG_INFO_DTDM2(d912pxy_s.render.iframe.UnInit(), "Final cleanups  5/13");
	LOG_INFO_DTDM2(d912pxy_s.render.db.shader.UnInit(), "Final cleanups  6/13");
	

	LOG_INFO_DTDM2(d912pxy_s.pool.vstream.UnInit(), "Final cleanups  7/13");
	LOG_INFO_DTDM2(d912pxy_s.pool.upload.UnInit(), "Final cleanups  8/13");
	LOG_INFO_DTDM2(d912pxy_s.pool.surface.UnInit(), "Final cleanups  9/13");
	LOG_INFO_DTDM2(d912pxy_s.pool.rtds.UnInit(), "Final cleanups  10/13");
	LOG_INFO_DTDM2(d912pxy_s.dx12.que.UnInit(), "Final cleanups 11/13");
	LOG_INFO_DTDM2(d912pxy_s.render.replay.Free(), "Final cleanups 12/13");
	LOG_INFO_DTDM2(d912pxy_s.vfs.UnInit(), "Final cleanups 13/13");

	for (int i = 0; i != PXY_INNER_MAX_DSC_HEAPS; ++i)
		delete m_dheaps[i];

	LOG_INFO_DTDM("Final cleanups 13/13");

#ifdef ENABLE_METRICS
	d912pxy_s.log.metrics.UnInit();
#endif

	delete perfGraph;

	if (initPtr)
		((IDirect3D9*)initPtr)->Release();

	d912pxy_s.pool.hostPow2.DeInit();
	d912pxy_s.com.DeInit();

	m_d12evice = nullptr;

	LOG_INFO_DTDM("d912pxy exited");

	d912pxy_comhandler::UnInit();
#ifdef _DEBUG
	d912pxy_helper::d3d12_ReportLeaks();
#endif	
}


ULONG d912pxy_device::ReleaseDevice()
{
	if (GetCOMRefCount() == 1)
	{
		d912pxy_com_object* comObj = comBase;

		UnInit();
		
		d912pxy_final_cleanup();

		return 0;

	}
	else 
		return Release();
}

void d912pxy_device::FreeAdditionalDX9Objects()
{
	if (mFakeOccQuery)
		mFakeOccQuery->Release();

	d912pxy_query_occlusion::DeInitOccQueryEmulation();

	d912pxy_s.render.draw_up.UnInit();
	delete m_emulatedSurfaceOps;

	mNullTexture->Release();
}

UINT d912pxy_device::GetAvailableTextureMem(void)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	return gpu_totalVidmemMB; 
}

HRESULT d912pxy_device::GetDeviceCaps(D3DCAPS9* pCaps)
{
	LOG_DBG_DTDM(__FUNCTION__);
	if (!pCaps)
		return D3DERR_INVALIDCALL;
	memcpy(pCaps, &cached_dx9caps, sizeof(D3DCAPS9));
	return D3D_OK; 
}

HRESULT d912pxy_device::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode)
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

HRESULT d912pxy_device::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	LOG_DBG_DTDM(__FUNCTION__);

	if (!pParameters)
		return D3DERR_INVALIDCALL;

	*pParameters = creationData;

	return D3D_OK;
}

void d912pxy_device::SetCursorPosition(int X, int Y, DWORD Flags)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	SetCursorPos(X, Y);	 
}

HRESULT d912pxy_device::ValidateDevice(DWORD* pNumPasses)
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	//megai2: pretend we can do anything! YES!
	*pNumPasses = 1;
	return D3D_OK; 
}

HRESULT d912pxy_device::GetDirect3D(IDirect3D9 ** ppv)
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
	cleanupLock.Hold();
	d912pxy_s.dx12.que.EnqueueCleanup(obj);
	cleanupLock.Release();
}

char * d912pxy_device::GetCurrentGPUName()
{
	return GPUNameA;
}

void d912pxy_device::ExternalFlush()
{
	if ((!isRunning.GetValue()) || (swapchains[0]->GetCurrentState() != SWCS_FOCUS_PENDING))
		return;

	swapOpLock.Hold();

	d912pxy_s.render.iframe.End();
	d912pxy_s.dx12.que.Flush(0);
	d912pxy_s.render.iframe.Start();

	swapOpLock.Release();
}

d912pxy_swapchain * d912pxy_device::GetPrimarySwapChain()
{
	return swapchains[0];
}


#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
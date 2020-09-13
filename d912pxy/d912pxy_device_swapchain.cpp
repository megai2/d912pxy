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

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_DEVICE_SWAPCHAIN

HRESULT d912pxy_device::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain)
{ 	
	//zero is always present
	for (int i = 1; i != PXY_INNER_MAX_SWAP_CHAINS; ++i)
	{
		if (swapchains[i])
			continue;
		
		swapchains[i] = &d912pxy_swapchain::d912pxy_swapchain_com(i, pPresentationParameters)->swapchain;

		*pSwapChain = PXY_COM_CAST_(IDirect3DSwapChain9, swapchains[i]);

		return D3D_OK;
	}
	return D3DERR_OUTOFVIDEOMEMORY;
}

HRESULT d912pxy_device::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain)
{ 	
	if (iSwapChain >= PXY_INNER_MAX_SWAP_CHAINS)
		return D3DERR_INVALIDCALL;

	*pSwapChain = PXY_COM_CAST_(IDirect3DSwapChain9, swapchains[iSwapChain]);
	(*pSwapChain)->AddRef();

	return D3D_OK; 
}

UINT d912pxy_device::GetNumberOfSwapChains(void)
{ 	
	//This method returns the number of swap chains created by CreateDevice.
	return 1; 
}

HRESULT d912pxy_device::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{ 		
	swapOpLock.Hold();
	
	d912pxy_s.render.iframe.End();
	d912pxy_s.dx12.que.Flush(0);

	cached_dx9displaymode.Width = pPresentationParameters->BackBufferWidth;
	cached_dx9displaymode.Height = pPresentationParameters->BackBufferHeight;


	HRESULT ret = swapchains[0]->SetPresentParameters(pPresentationParameters);
	
	d912pxy_s.render.iframe.Start();

	swapOpLock.Release();

	d912pxy_s.thread.cleanup.OnReset();
		
	return ret; 
}

HRESULT d912pxy_device::InnerPresentExecute()
{
	swapOpLock.Hold();

	d912pxy_s.render.iframe.End();
	
	FRAME_METRIC_PRESENT(0)

	LOG_DBG_DTDM2("Present Exec GPU");
	return d912pxy_s.dx12.que.ExecuteCommands(1);
}

void d912pxy_device::InnerPresentFinish()
{
	FRAME_METRIC_PRESENT(1)
	
	d912pxy_s.render.iframe.Start();

	swapOpLock.Release();
	
	if (swapchains[0])
		swapchains[0]->WaitForNewFrame();

	LOG_DBG_DTDM("Present finished");	
}

HRESULT d912pxy_device::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{ 

#ifdef ENABLE_METRICS
	d912pxy_s.log.metrics.TrackDrawCount(d912pxy_s.render.batch.GetBatchCount());
#endif 

	HRESULT ret = InnerPresentExecute();

#ifdef ENABLE_METRICS	
	d912pxy_s.log.metrics.FlushIFrameValues();	
#endif 
	
	InnerPresentFinish();

	return ret;
}

HRESULT d912pxy_device::Present_PG(const RECT * pSourceRect, const RECT * pDestRect, HWND hDestWindowOverride, const RGNDATA * pDirtyRegion)
{
#ifdef ENABLE_METRICS
	d912pxy_s.log.metrics.TrackDrawCount(d912pxy_s.render.batch.GetBatchCount());
#endif 

	perfGraph->RecordPresent(d912pxy_s.render.batch.GetBatchCount());

	HRESULT ret = InnerPresentExecute();

#ifdef ENABLE_METRICS
	d912pxy_s.log.metrics.FlushIFrameValues();
#endif

	InnerPresentFinish();

	return ret;
}

HRESULT d912pxy_device::Present_Extra(const RECT * pSourceRect, const RECT * pDestRect, HWND hDestWindowOverride, const RGNDATA * pDirtyRegion)
{
#ifdef ENABLE_METRICS
	d912pxy_s.log.metrics.TrackDrawCount(d912pxy_s.render.batch.GetBatchCount());
#endif 

	d912pxy_s.extras.OnPresent();

	perfGraph->RecordPresent(d912pxy_s.render.batch.GetBatchCount());

	HRESULT ret = InnerPresentExecute();

#ifdef ENABLE_METRICS
	d912pxy_s.log.metrics.FlushIFrameValues();
#endif

	InnerPresentFinish();

	d912pxy_s.extras.WaitForTargetFrameTime();

	return ret;
}

HRESULT d912pxy_device::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
	HRESULT ret = swapchains[iSwapChain]->GetBackBuffer(iBackBuffer, Type, ppBackBuffer);

	return ret;
}

HRESULT d912pxy_device::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
{
	return PXY_COM_CAST_(IDirect3DSwapChain9, swapchains[iSwapChain])->GetRasterStatus(pRasterStatus);
}

void d912pxy_device::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{ 
	swapchains[iSwapChain]->SetGammaRamp(Flags, pRamp);
}

void d912pxy_device::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp)
{ 
	swapchains[iSwapChain]->GetGammaRamp(pRamp);
}

HRESULT d912pxy_device::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) 
{	
	swapchains[iSwapChain]->GetFrontBufferData(pDestSurface);
	return D3D_OK;
}

HRESULT d912pxy_device::TestCooperativeLevel(void)
{
	swapOpLock.Hold();

	HRESULT ret = swapchains[0]->TestCoopLevel();

	swapOpLock.Release();

	return ret;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
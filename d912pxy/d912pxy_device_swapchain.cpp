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

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_DEVICE_SWAPCHAIN

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

	swapOpLock.Hold();

	d912pxy_s(iframe)->End();
	d912pxy_s(GPUque)->Flush(0);

	HRESULT ret = swapchains[0]->SetPresentParameters(pPresentationParameters);
	
	d912pxy_s(iframe)->Start();

	swapOpLock.Release();

	API_OVERHEAD_TRACK_END(0)
		
	return ret; 
}

HRESULT WINAPI d912pxy_device::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	swapOpLock.Hold();
	
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

	swapOpLock.Release();

	API_OVERHEAD_TRACK_END(0)	

	return ret;
}

HRESULT WINAPI d912pxy_device::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	HRESULT ret = swapchains[iSwapChain]->GetBackBuffer(iBackBuffer, Type, ppBackBuffer);

	API_OVERHEAD_TRACK_END(0)

	return ret;
}

HRESULT WINAPI d912pxy_device::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
{
	LOG_DBG_DTDM(__FUNCTION__);

	return swapchains[iSwapChain]->GetRasterStatus(pRasterStatus);
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

HRESULT WINAPI d912pxy_device::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) 
{	
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	swapchains[iSwapChain]->GetFrontBufferData(pDestSurface);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::TestCooperativeLevel(void)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	swapOpLock.Hold();

	HRESULT ret = swapchains[0]->TestCoopLevel();

	swapOpLock.Release();

	API_OVERHEAD_TRACK_END(0)

	return ret;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
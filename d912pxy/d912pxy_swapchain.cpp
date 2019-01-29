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
#include "d912pxy_swapchain.h"

#define LOG_DBG_DEV(fmt, ...) m_log->P7_DEBUG(LGC_SWPCHA, TM(fmt), __VA_ARGS__)

d912pxy_swapchain::d912pxy_swapchain(d912pxy_device* dev, int index, HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount_div2, BOOL Fullscreen, UINT i_vSync):
	d912pxy_comhandler(L"swap chain")
{
	m_dev = dev;
	m_idx = index;
	m_hwnd = hWnd;

	vSync = i_vSync;

	markedAsLostOnFullscreen = 0;

	m_width = width;
	m_height = height;

	UINT bufferCount = bufferCount_div2 * 2;
	
	if (bufferCount == 0)
		bufferCount = 2;

	totalBackBuffers = bufferCount;

	m_d12swp = d912pxy_helper::CreateSwapChain(hWnd, commandQueue, m_width, m_height, totalBackBuffers, Fullscreen);

	m_log->P7_INFO(LGC_DEFAULT, TM("wnd %llX w %u h %u c %u"), m_hwnd, m_width, m_height, totalBackBuffers);

	if (m_d12swp)
	{
		for (int i = 0; i != bufferCount; ++i)
		{
			ComPtr<ID3D12Resource> bbRes;

			LOG_ERR_THROW2(m_d12swp->GetBuffer(i, IID_PPV_ARGS(&bbRes)), "swapchain getbuffer fail");

			dxgiBackBuffer[i] = new d912pxy_surface(dev, bbRes, D3D12_RESOURCE_STATE_COMMON, this);
		}
	}
	else {
				
		m_log->P7_ERROR(LGC_DEFAULT, TM("swapchain for wnd %llX w %u h %u c %u failed, but faking that's this is ok"), hWnd, width, height, bufferCount);

		for (int i = 0; i != bufferCount; ++i)
		{
			UINT lret = 0;
			dxgiBackBuffer[i] = new d912pxy_surface(dev, width, height, D3DFMT_A8R8G8B8, D3DUSAGE_RENDERTARGET, &lret, 1);
		}
	}

	if (!Fullscreen)
		m_flags = d912pxy_helper::CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
	else
		m_flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	
	backBufferSurface = new d912pxy_surface(dev, width, height, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, 0, 0);	
	depthStencilSurface = new d912pxy_surface(dev, width, height, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, 0, 1);

	dev->SetRenderTarget(0, backBufferSurface);
	dev->SetDepthStencilSurface(depthStencilSurface);

	SetFullscreen(Fullscreen);
}

d912pxy_swapchain::~d912pxy_swapchain()
{
	m_log->P7_INFO(LGC_DEFAULT, TM("Stopping swapchain"));

	m_d12swp->SetFullscreenState(0, NULL);

	for (int i = 0; i != totalBackBuffers; ++i)
		dxgiBackBuffer[i]->Release();

	backBufferSurface->Release();
	depthStencilSurface->Release();
}

HRESULT d912pxy_swapchain::QueryInterface(REFIID riid, void ** ppvObj)
{
	return d912pxy_comhandler::QueryInterface(riid, ppvObj);
}
ULONG d912pxy_swapchain::AddRef(void)
{
	return d912pxy_comhandler::AddRef();
}

ULONG d912pxy_swapchain::Release(void)
{
	return d912pxy_comhandler::Release();
}

HRESULT d912pxy_swapchain::Present(CONST RECT * pSourceRect, CONST RECT * pDestRect, HWND hDestWindowOverride, CONST RGNDATA * pDirtyRegion, DWORD dwFlags)
{
	//ignore parameters for now, just cause...	
	//TODO

	return D3D_OK;
}

HRESULT d912pxy_swapchain::GetFrontBufferData(IDirect3DSurface9 * pDestSurface)
{
	LOG_DBG_DTDM(__FUNCTION__);

	return E_NOTIMPL;
}

HRESULT d912pxy_swapchain::GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9 ** ppBackBuffer)
{
	LOG_DBG_DTDM(__FUNCTION__);
	
	//iBackBuffer represents a index of pair buffers?

	//megai2: DX9 uses the same surface interface for various buffers, but we - not, so assming that app saving backbuffer for SetRenderTarget, we need to fake up acces to real back buffer

	*ppBackBuffer = (IDirect3DSurface9 *)backBufferSurface;

	backBufferSurface->AddRef();

	return D3D_OK;
}

HRESULT d912pxy_swapchain::GetRasterStatus(D3DRASTER_STATUS * pRasterStatus)
{
	LOG_DBG_DTDM(__FUNCTION__);

	return E_NOTIMPL;
}

HRESULT d912pxy_swapchain::GetDisplayMode(D3DDISPLAYMODE * pMode)
{
	LOG_DBG_DTDM(__FUNCTION__);

	return E_NOTIMPL;
}

HRESULT d912pxy_swapchain::GetDevice(IDirect3DDevice9 ** ppDevice)
{
	*ppDevice = (IDirect3DDevice9 *)m_dev;

	return D3D_OK;
}

HRESULT d912pxy_swapchain::GetPresentParameters(D3DPRESENT_PARAMETERS * pPresentationParameters)
{
	LOG_DBG_DTDM(__FUNCTION__);

	return E_NOTIMPL;
}

void d912pxy_swapchain::SetFullscreen(UINT fullscreen)
{
	if (fullscreen)
	{

		DXGI_MODE_DESC mdsc;
		mdsc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		mdsc.Height = m_height;
		mdsc.Width = m_width;
		mdsc.RefreshRate.Denominator = 1;
		mdsc.RefreshRate.Numerator = 60;
		mdsc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		mdsc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		m_d12swp->ResizeTarget(&mdsc);
	}

	isFullscreen = fullscreen;

	m_d12swp->SetFullscreenState(isFullscreen, NULL);
}

ComPtr<IDXGISwapChain4> d912pxy_swapchain::GetD12swpc()
{
	return m_d12swp;
}

void d912pxy_swapchain::StartFrame()
{
	backBufferSurface->IFrameBarrierTrans(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_RENDER_TARGET, CLG_TOP);

	m_dev->SetRenderTarget(0, backBufferSurface);
	m_dev->SetDepthStencilSurface(depthStencilSurface);	
}

void d912pxy_swapchain::EndFrame()
{
	backBufferSurface->IFrameBarrierTrans(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_PRESENT, CLG_SEQ);
}

d912pxy_surface * d912pxy_swapchain::GetRTBackBuffer()
{
	return backBufferSurface;
}

void d912pxy_swapchain::Resize(UINT width, UINT height, UINT fullscreen, UINT newVSync)
{	
	m_log->P7_INFO(LGC_DEFAULT, TM("swapchain resize width %u height %u"), width, height);

	vSync = newVSync;

	depthStencilSurface->Release();
	backBufferSurface->Release();

	for (int i = 0; i != totalBackBuffers; ++i)
	{
		dxgiBackBuffer[i]->Release();
	}

	d912pxy_s(iframe)->ClearBindedSurfaces();

	//manual cleanup cuz buffers woulbe be referenced by this frame, but we need them d.e.d.
	d912pxy_s(GPUcl)->CleanupAllReferenced();

	/*m_d12swp = nullptr;

	m_d12swp = d912pxy_helper::CreateSwapChain(m_hwnd, d912pxy_s(GPUque)->GetDXQue(), m_width, m_height, totalBackBuffers, fullscreen);

	m_log->P7_INFO(LGC_DEFAULT, TM("wnd %llX w %u h %u c %u"), m_hwnd, m_width, m_height, totalBackBuffers);

	if (m_d12swp)
	{
		currentBackBuffer = m_d12swp->GetCurrentBackBufferIndex();

		for (int i = 0; i != totalBackBuffers; ++i)
		{
			ComPtr<ID3D12Resource> bbRes;

			LOG_ERR_THROW2(m_d12swp->GetBuffer(i, IID_PPV_ARGS(&bbRes)), "swapchain getbuffer fail");

			backBufferSurfaces[i] = new d912pxy_surface(m_dev, bbRes, D3D12_RESOURCE_STATE_COMMON, this);
		}
	}*/
	
	SetFullscreen(fullscreen && !markedAsLostOnFullscreen);

	m_d12swp->ResizeBuffers(totalBackBuffers, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, m_flags);

	for (int i = 0; i != totalBackBuffers; ++i)
	{
		ComPtr<ID3D12Resource> bbRes;

		LOG_ERR_THROW2(m_d12swp->GetBuffer(i, IID_PPV_ARGS(&bbRes)), "swapchain getbuffer fail");

		dxgiBackBuffer[i] = new d912pxy_surface(m_dev, bbRes, D3D12_RESOURCE_STATE_COMMON, this);
	}

	backBufferSurface = new d912pxy_surface(m_dev, width, height, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, 0, 0);
	depthStencilSurface = new d912pxy_surface(m_dev, width, height, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, 0, 1);

/*	m_dev->SetRenderTarget(0, backBufferSurfaces[0]);
	m_dev->SetDepthStencilSurface(depthStencilSurface);*/

	m_width = width;
	m_height = height;	
}

HRESULT d912pxy_swapchain::TestCoopLevel()
{
	if (markedAsLostOnFullscreen)
	{
		if (!isFullscreen)
		{
			markedAsLostOnFullscreen = 0;
			return D3DERR_DEVICENOTRESET;
		}

		UINT isActiveWindow = (GetActiveWindow() == m_hwnd) && (markedAsLostOnFullscreen > 10);

		if (!isActiveWindow)
		{
			ShowWindow(m_hwnd, SW_FORCEMINIMIZE);
			++markedAsLostOnFullscreen;
		}

		if (isActiveWindow)
			return D3DERR_DEVICENOTRESET;
		else 
			return D3DERR_DEVICELOST;
		/*
		UINT isActiveWindow = (GetActiveWindow() == m_hwnd) && (markedAsLostOnFullscreen > 10);
		
		if (isActiveWindow)
		{
			ShowWindow(m_hwnd, SW_RESTORE | SW_SHOW);

			MSG msg;
			if (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
			{
				if (::GetMessage(&msg, NULL, 0, 0))
				{
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}
			}
		}

		HRESULT ret = m_d12swp->Present(0, DXGI_PRESENT_TEST);

		if (ret == DXGI_STATUS_OCCLUDED)
		{
			if (!isActiveWindow)
			{
				ShowWindow(m_hwnd, SW_FORCEMINIMIZE);
				++markedAsLostOnFullscreen;
			}
			//m_d12swp->SetFullscreenState(0, NULL);

			//LOG_ERR_THROW2(ret, L"d912pxy_swapchain::TestCoopLevel() got occluded");

			return D3DERR_DEVICELOST;
		}
		else {
			LOG_ERR_THROW2(ret, L"d912pxy_swapchain::TestCoopLevel()");

			markedAsLostOnFullscreen = 0;
			
			return D3DERR_DEVICENOTRESET;
		}*/
	}
	else
		return D3D_OK;

}

HRESULT d912pxy_swapchain::Swap()
{
	//do a swap with saved data from Present
	if (m_d12swp)
	{
		HRESULT pret = S_OK;

		if (markedAsLostOnFullscreen)
		{
			return D3DERR_DEVICELOST;
		}

		if (isFullscreen)
		{
			pret = m_d12swp->Present(0, DXGI_PRESENT_TEST);

			if (pret == S_OK)
				pret = m_d12swp->Present(vSync, 0);

			if (pret == DXGI_STATUS_OCCLUDED)
			{
				while (!d912pxy_s(psoCache)->IsCompileQueueFree())
					Sleep(0);

				pret = D3DERR_DEVICELOST;
				markedAsLostOnFullscreen = 1;
			}
			else {
				LOG_ERR_THROW2(pret, "d912pxy_swapchain::Swap");
				pret = D3D_OK;
			}
		}
		else {
			pret = m_d12swp->Present(0, 0);
			LOG_ERR_THROW2(pret, "d912pxy_swapchain::Swap");			
		}

		return pret;
	}
	else
		return D3D_OK;
}

HRESULT d912pxy_swapchain::AsyncSwapNote()
{
	if (m_d12swp)
	{
		HRESULT pret = S_OK;
		
		if (markedAsLostOnFullscreen)
		{
			return D3DERR_DEVICELOST;
		}

		if (isFullscreen)
			pret = m_d12swp->Present(0, DXGI_PRESENT_TEST);

		if (pret == DXGI_STATUS_OCCLUDED)
		{
			while (!d912pxy_s(psoCache)->IsCompileQueueFree())
				Sleep(0);

			pret = D3DERR_DEVICELOST;
			markedAsLostOnFullscreen = 1;
		}
		else {
			LOG_ERR_THROW2(pret, "d912pxy_swapchain::Swap");			
		}

		return pret;
	}
	else
		return 0;
}

HRESULT d912pxy_swapchain::AsyncSwapExec()
{
	HRESULT pret = Swap();

	return pret;
}

void d912pxy_swapchain::CopyToDXGI(ID3D12GraphicsCommandList * cl)
{
	UINT dxgiIdx = m_d12swp->GetCurrentBackBufferIndex();

	d912pxy_surface* dxgiBuf = dxgiBackBuffer[dxgiIdx];
	
	dxgiBuf->IFrameBarrierTrans2(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT, cl);
	backBufferSurface->IFrameBarrierTrans2(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PRESENT, cl);

	backBufferSurface->CopyTo2(dxgiBuf, cl);

	backBufferSurface->IFrameBarrierTrans2(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_SOURCE, cl);
	dxgiBuf->IFrameBarrierTrans2(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST, cl);
}

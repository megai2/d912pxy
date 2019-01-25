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
#pragma once

#include "stdafx.h"

class d912pxy_swapchain : public d912pxy_comhandler, public IDirect3DSwapChain9
{
public:
	d912pxy_swapchain(d912pxy_device* dev, int index, HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount, BOOL Fullscreen, UINT i_vSync);
	virtual ~d912pxy_swapchain();

	HRESULT WINAPI QueryInterface(REFIID riid, void** ppvObj);
	ULONG WINAPI AddRef(void);
	ULONG WINAPI Release(void);

	/*** IDirect3DSwapChain9 methods ***/
	HRESULT WINAPI Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags);
	HRESULT WINAPI GetFrontBufferData(IDirect3DSurface9* pDestSurface);
	HRESULT WINAPI GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer);
	HRESULT WINAPI GetRasterStatus(D3DRASTER_STATUS* pRasterStatus);
	HRESULT WINAPI GetDisplayMode(D3DDISPLAYMODE* pMode);
	HRESULT WINAPI GetDevice(IDirect3DDevice9** ppDevice);
	HRESULT WINAPI GetPresentParameters(D3DPRESENT_PARAMETERS* pPresentationParameters);

	///

	void SetFullscreen(UINT fullscreen);

	ComPtr<IDXGISwapChain4> GetD12swpc();

	void StartFrame();
	void EndFrame();

	d912pxy_surface* GetRTBackBuffer();

	void Resize(UINT width, UINT height, UINT fullscreen, UINT newVSync);


	HRESULT TestCoopLevel();

	HRESULT Swap();

	HRESULT AsyncSwapNote();
	HRESULT AsyncSwapExec();

private:
	DWORD m_idx;
	HWND m_hwnd;

	UINT vSync;

	d912pxy_device* m_dev;

	d912pxy_surface* backBufferSurfaces[256];
	d912pxy_surface* depthStencilSurface;

	UINT currentBackBuffer;
	UINT totalBackBuffers;

	UINT isFullscreen;
	UINT m_width;
	UINT m_height;
	UINT m_flags;
	
	UINT markedAsLostOnFullscreen;

	ComPtr<IDXGISwapChain4> m_d12swp;
	ComPtr<ID3D12Fence> fence;

	HANDLE fenceEvent;
};


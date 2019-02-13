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

typedef enum d912pxy_swapchain_state {
	SWCS_SETUP = 0,
	SWCS_INIT_ERROR,
	SWCS_SWAPPABLE,
	SWCS_SWAPPABLE_EXCLUSIVE,
	SWCS_RECONFIGURE,
	SWCS_SWAP_ERROR,
	SWCS_FOCUS_LOST_SWITCH,
	SWCS_FOCUS_LOST,
	SWCS_FOCUS_PENDING,
	SWCS_RESETUP,
	SWCS_SHUTDOWN,
	SWCS_COUNT
} d912pxy_swapchain_state;

static const char* d912pxy_swapchain_state_names[] = {
	"SWCS_SETUP",
	"SWCS_INIT_ERROR",
	"SWCS_SWAPPABLE",
	"SWCS_SWAPPABLE_EXCLUSIVE",
	"SWCS_RECONFIGURE",
	"SWCS_SWAP_ERROR",
	"SWCS_FOCUS_LOST_SWITCH",
	"SWCS_FOCUS_LOST",
	"SWCS_FOCUS_PENDING",
	"SWCS_RESETUP",
	"SWCS_SHUTDOWN",
	"SWCS_COUNT"
};

class d912pxy_swapchain : public d912pxy_comhandler, public IDirect3DSwapChain9
{
public:
	d912pxy_swapchain(d912pxy_device* dev, int index, D3DPRESENT_PARAMETERS* in_pp);
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

	HRESULT SetPresentParameters(D3DPRESENT_PARAMETERS* pp);

	void StartFrame();
	void EndFrame();

	HRESULT TestCoopLevel();
	HRESULT Swap();
	HRESULT SwapCheck();

	void CopyFrameToDXGI(ID3D12GraphicsCommandList* cl);

	void SetGammaRamp(DWORD Flags, CONST D3DGAMMARAMP* pRamp);
	void GetGammaRamp(D3DGAMMARAMP* pRamp);

private:	
	void ChangeState(d912pxy_swapchain_state newState);

	void ThrowCritialError(HRESULT ret, const char* msg);
	
	void ResetFrameTargets();
	void FreeFrameTargets();
	void FixPresentParameters();

	D3DPRESENT_PARAMETERS currentPP;
	D3DPRESENT_PARAMETERS oldPP;

	d912pxy_surface* backBufferSurface;
	d912pxy_surface* depthStencilSurface;

	d912pxy_swapchain_state state;	
	HRESULT swapCheckValue;
	UINT errorCount;
	HRESULT (d912pxy_swapchain::*swapHandlers[SWCS_COUNT])();

	HRESULT SwapHandle_Default();	
	HRESULT SwapHandle_Setup();
	HRESULT SwapHandle_Init_Error();
	HRESULT SwapHandle_Swappable();
	HRESULT SwapHandle_Swappable_Exclusive();
	HRESULT SwapHandle_Reconfigure();
	HRESULT SwapHandle_Swap_Error();
	HRESULT SwapHandle_Focus_Lost();
	HRESULT SwapHandle_ReSetup();
	HRESULT SwapHandle_Focus_Pending();
	HRESULT SwapHandle_Focus_Lost_Switch();
	
	//DXGI related
	HRESULT InitDXGISwapChain();
	void FreeDXGISwapChain();
	HRESULT GetDXGIBuffers();
	void FreeDXGISwapChainReferences();
	HRESULT ChangeDXGISwapChain();
	HRESULT SetDXGIFullscreen();
	DXGI_FORMAT GetDXGIFormatForBackBuffer(D3DFORMAT fmt);

	void CacheDXGITearingSupport();

	ComPtr<IDXGISwapChain4> dxgiSwapchain;
	ComPtr<ID3D12Resource> dxgiBackBuffer[4];	
	UINT dxgiResizeFlags;
	UINT dxgiNoWaitFlag;
	UINT dxgiPresentFlags;
	UINT dxgiTearingSupported;
	UINT dxgiBuffersCount;
};


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
	SWCS_SWAP_TEST,
	SWCS_SETUP_W7,
	SWCS_RECONFIGURE_W7,
	SWCS_SWAPPABLE_W7,
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
	"SWCS_SWAP_TEST",
	"SWCS_SETUP_W7",
	"SWCS_RECONFIGURE_W7",
	"SWCS_SWAPPABLE_W7",
	"SWCS_COUNT"
};

class d912pxy_swapchain : public d912pxy_vtable, public d912pxy_comhandler
{
public:	
	static d912pxy_com_object* d912pxy_swapchain_com(int index, D3DPRESENT_PARAMETERS* in_pp);
	~d912pxy_swapchain();

	D912PXY_METHOD(Present)(PXY_THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags);
	D912PXY_METHOD(GetFrontBufferData)(PXY_THIS_ IDirect3DSurface9* pDestSurface);
	D912PXY_METHOD(GetBackBuffer)(PXY_THIS_ UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer);
	D912PXY_METHOD(GetRasterStatus)(PXY_THIS_ D3DRASTER_STATUS* pRasterStatus);
	D912PXY_METHOD(GetDisplayMode)(PXY_THIS_ D3DDISPLAYMODE* pMode);	
	D912PXY_METHOD(GetPresentParameters)(PXY_THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);
	D912PXY_METHOD_(ULONG, ReleaseSwapChain)(PXY_THIS);

	HRESULT Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags);
	HRESULT GetFrontBufferData(IDirect3DSurface9* pDestSurface);
	HRESULT GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer);
	HRESULT GetPresentParameters(D3DPRESENT_PARAMETERS* pPresentationParameters);
	D912PXY_METHOD_NC_(ULONG, ReleaseSwapChain)(THIS);

	///

	HRESULT SetPresentParameters(D3DPRESENT_PARAMETERS* pp);

	void StartFrame();
	void EndFrame();

	HRESULT TestCoopLevel();
	HRESULT Swap();
	HRESULT SwapCheck();

	void WaitForNewFrame();

	d912pxy_swapchain_state GetCurrentState() { return state; };

	void CopyFrameToDXGI(ID3D12GraphicsCommandList* cl);

	void SetGammaRamp(DWORD Flags, CONST D3DGAMMARAMP* pRamp);
	void GetGammaRamp(D3DGAMMARAMP* pRamp);

	LRESULT DXGIWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT DXGIWndProc_Extras(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void ReanimateDXGI();

	HWND GetTargetWindow();
	d912pxy_surface* GetRenderBuffer();

private:	
	d912pxy_swapchain(int index, D3DPRESENT_PARAMETERS* in_pp);

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
	HRESULT SwapHandle_Swap_Test();

	HRESULT SwapHandle_Setup_W7();
	HRESULT SwapHandle_Reconfigure_W7();
	HRESULT SwapHandle_Swappable_W7();
	
	//DXGI related
	HRESULT InitDXGISwapChain();
	void FreeDXGISwapChain();
	HRESULT GetDXGIBuffers();
	void FreeDXGISwapChainReferences();
	HRESULT ChangeDXGISwapChain();
	HRESULT SetDXGIFullscreen();
	DXGI_FORMAT GetDXGIFormatForBackBuffer(D3DFORMAT fmt);
	UINT DXGIFullscreenInterrupt(UINT inactive);
	void CacheDXGITearingSupport();

	void OverrideWndProc();
	
	ComPtr<IDXGISwapChain4> dxgiSwapchain;
	ComPtr<ID3D12Resource> dxgiBackBuffer[4];	
	UINT dxgiResizeFlags;
	UINT dxgiNoWaitFlag;
	UINT dxgiPresentFlags;
	UINT dxgiTearingSupported;
	UINT dxgiBuffersCount;
	UINT dxgiMaxFrameLatency;
	HANDLE dxgiFrameLatencyWaitObj;

	d912pxy_thread_lock fullscreenIterrupt;
	WNDPROC dxgiOWndProc;

	//w7 port
	ComPtr<ID3D12CommandQueueDownlevel> w7_cq;
	ComPtr<ID3D12GraphicsCommandList> w7_cls[2];
	ComPtr<ID3D12CommandAllocator> w7_cla[2];
	UINT32 w7_cl_idx;
};


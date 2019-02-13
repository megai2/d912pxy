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

#define DXGI_SOFT_THROW(a, msg, ...) { HRESULT eret = a; if (FAILED(eret)) { LOG_ERR_DTDM(msg, __VA_ARGS__); LOG_ERR_DTDM("HR: %08lX", eret); state = SWCS_INIT_ERROR; return D3D_OK; } }

d912pxy_swapchain::d912pxy_swapchain(d912pxy_device * dev, int index, D3DPRESENT_PARAMETERS * in_pp) : d912pxy_comhandler(dev, L"swap chain")
{
	state = SWCS_SETUP;
	depthStencilSurface = NULL;
	backBufferSurface = NULL;
	swapCheckValue = D3D_OK;
	dxgiNoWaitFlag = DXGI_PRESENT_DO_NOT_WAIT;
	errorCount = 0;

	currentPP = *in_pp;

	swapHandlers[SWCS_SETUP] = &d912pxy_swapchain::SwapHandle_Setup;
	swapHandlers[SWCS_INIT_ERROR] = &d912pxy_swapchain::SwapHandle_Init_Error;
	swapHandlers[SWCS_SWAPPABLE] = &d912pxy_swapchain::SwapHandle_Swappable;
	swapHandlers[SWCS_SWAPPABLE_EXCLUSIVE] = &d912pxy_swapchain::SwapHandle_Swappable_Exclusive;
	swapHandlers[SWCS_RECONFIGURE] = &d912pxy_swapchain::SwapHandle_Reconfigure;
	swapHandlers[SWCS_SWAP_ERROR] = &d912pxy_swapchain::SwapHandle_Swap_Error;
	swapHandlers[SWCS_FOCUS_LOST] = &d912pxy_swapchain::SwapHandle_Focus_Lost;
	swapHandlers[SWCS_FOCUS_LOST_SWITCH] = &d912pxy_swapchain::SwapHandle_Focus_Lost_Switch;
	swapHandlers[SWCS_RESETUP] = &d912pxy_swapchain::SwapHandle_ReSetup;
	swapHandlers[SWCS_FOCUS_PENDING] = &d912pxy_swapchain::SwapHandle_Focus_Pending;
	swapHandlers[SWCS_SHUTDOWN] = &d912pxy_swapchain::SwapHandle_Default;

	CacheDXGITearingSupport();
	ResetFrameTargets();
}

d912pxy_swapchain::~d912pxy_swapchain()
{
	m_log->P7_INFO(LGC_DEFAULT, TM("Stopping swapchain"));
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
	ULONG ret = d912pxy_comhandler::Release();

	if (!ret)
	{
		ChangeState(SWCS_SHUTDOWN);
		FreeFrameTargets();
		FreeDXGISwapChain();

		d912pxy_s(psoCache)->LockCompileQue(0);
		swapCheckValue = D3DERR_DEVICELOST;		
	}

	return ret;
}

HRESULT d912pxy_swapchain::Present(CONST RECT * pSourceRect, CONST RECT * pDestRect, HWND hDestWindowOverride, CONST RGNDATA * pDirtyRegion, DWORD dwFlags)
{
	//ignore parameters for now, just cause...	
	//TODO

	return D3D_OK;
}

HRESULT d912pxy_swapchain::GetFrontBufferData(IDirect3DSurface9 * pDestSurface)
{	
	//megai2: not actual front buffer data, but should work 

	d912pxy_surface * dst = (d912pxy_surface*)pDestSurface;
	backBufferSurface->CopyTo(dst, 0, d912pxy_s(GPUcl)->GID(CLG_SEQ).Get());

	dst->CopySurfaceDataToCPU();

	return D3D_OK;
}

HRESULT d912pxy_swapchain::GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9 ** ppBackBuffer)
{
	LOG_DBG_DTDM(__FUNCTION__);
	
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

	*pPresentationParameters = currentPP;

	return D3D_OK;
}

HRESULT d912pxy_swapchain::SetPresentParameters(D3DPRESENT_PARAMETERS * pp)
{
	//megai2: be shure this is called when all pending operations are done

	//megai2: skip reconfiguration if parameters are same
	if (!memcmp(pp, &currentPP, sizeof(D3DPRESENT_PARAMETERS)))
		return D3D_OK;
	
	//megai2: error out if system is not ready for one more reset
	while ((state != SWCS_SWAPPABLE) && ((state != SWCS_SWAPPABLE_EXCLUSIVE)))
	{
		Swap();		
		if (state == SWCS_FOCUS_LOST)
			return D3DERR_DEVICELOST;
	}
	
	oldPP = currentPP;
	currentPP = *pp;
	
	ResetFrameTargets();	

	ChangeState(SWCS_RECONFIGURE);

	//megai2: force state processing due to window message lockup
	while ((state != SWCS_SWAPPABLE) && ((state != SWCS_SWAPPABLE_EXCLUSIVE)))
	{
		Swap();
		if (state == SWCS_FOCUS_LOST)
			return D3DERR_DEVICELOST;
	}

	return D3D_OK;
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

HRESULT d912pxy_swapchain::TestCoopLevel()
{	 
	if ((state == SWCS_FOCUS_LOST) || (state == SWCS_FOCUS_PENDING) || (state == SWCS_FOCUS_LOST_SWITCH))
	{
		Swap();
		return D3DERR_DEVICELOST;
	}
	else 
		return swapCheckValue;
}

HRESULT d912pxy_swapchain::Swap()
{
	return (this->*swapHandlers[state])();
}

HRESULT d912pxy_swapchain::SwapCheck()
{
	return swapCheckValue;
}

void d912pxy_swapchain::CopyFrameToDXGI(ID3D12GraphicsCommandList * cl)
{
	if ((state != SWCS_SWAPPABLE) && ((state != SWCS_SWAPPABLE_EXCLUSIVE)))
		return;

	UINT dxgiIdx = dxgiSwapchain->GetCurrentBackBufferIndex();

	ComPtr<ID3D12Resource> dxgiBuf = dxgiBackBuffer[dxgiIdx];

	d912pxy_resource::IFrameBarrierTrans3(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT, cl, dxgiBuf);
	backBufferSurface->IFrameBarrierTrans2(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PRESENT, cl);

	backBufferSurface->CopyTo3(dxgiBuf, cl);

	backBufferSurface->IFrameBarrierTrans2(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_SOURCE, cl);
	d912pxy_resource::IFrameBarrierTrans3(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST, cl, dxgiBuf);
}

void d912pxy_swapchain::SetGammaRamp(DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{
	//get output info to convert ramp formats
	ComPtr<IDXGISwapChain4> d12sw = dxgiSwapchain;
	ComPtr<IDXGIOutput> odata;

	if (FAILED(d12sw->GetContainingOutput(&odata)))
		return;

	DXGI_GAMMA_CONTROL_CAPABILITIES gammaCaps;
	if (FAILED(odata->GetGammaControlCapabilities(&gammaCaps)))
		return;

	//0 = 0
	//255 = 1.0f

	DXGI_GAMMA_CONTROL newRamp;

	newRamp.Scale.Red = 1.0f;
	newRamp.Scale.Green = 1.0f;
	newRamp.Scale.Blue = 1.0f;
	newRamp.Offset.Red = 0.0f;
	newRamp.Offset.Green = 0.0f;
	newRamp.Offset.Blue = 0.0f;

	float dlt = (gammaCaps.MaxConvertedValue - gammaCaps.MinConvertedValue) / gammaCaps.NumGammaControlPoints;
	float base = gammaCaps.MinConvertedValue;

	for (int i = 0; i != gammaCaps.NumGammaControlPoints; ++i)
	{
		float dxgiFI = base + dlt * i;
		for (int j = 0; j != 256; ++j)
		{
			float d9FI = j / 255.0f;
			float d9FIn = (j + 1) / 255.0f;

			if ((dxgiFI >= d9FI) && (dxgiFI < d9FIn))
			{
				newRamp.GammaCurve[i].Red = pRamp->red[j] / 65535.0f;
				newRamp.GammaCurve[i].Green = pRamp->green[j] / 65535.0f;
				newRamp.GammaCurve[i].Blue = pRamp->blue[j] / 65535.0f;
				break;
			}
		}

		if (dxgiFI < 0)
		{
			newRamp.GammaCurve[i].Red = pRamp->red[0] / 65535.0f;
			newRamp.GammaCurve[i].Green = pRamp->green[0] / 65535.0f;
			newRamp.GammaCurve[i].Blue = pRamp->blue[0] / 65535.0f;
		}
		else if (dxgiFI > 1.0f) {
			newRamp.GammaCurve[i].Red = pRamp->red[255] / 65535.0f;
			newRamp.GammaCurve[i].Green = pRamp->green[255] / 65535.0f;
			newRamp.GammaCurve[i].Blue = pRamp->blue[255] / 65535.0f;
		}


	}

	odata->SetGammaControl(&newRamp);
}

void d912pxy_swapchain::GetGammaRamp(D3DGAMMARAMP* pRamp)
{
	//get output info to convert ramp formats
	ComPtr<IDXGISwapChain4> d12sw = dxgiSwapchain;
	ComPtr<IDXGIOutput> odata;

	if (FAILED(d12sw->GetContainingOutput(&odata)))
		return;

	DXGI_GAMMA_CONTROL_CAPABILITIES gammaCaps;
	if (FAILED(odata->GetGammaControlCapabilities(&gammaCaps)))
		return;

	DXGI_GAMMA_CONTROL curRamp;
	if (FAILED(odata->GetGammaControl(&curRamp)))
		return;

	float dlt = (gammaCaps.MaxConvertedValue - gammaCaps.MinConvertedValue) / gammaCaps.NumGammaControlPoints;
	float base = gammaCaps.MinConvertedValue;

	for (int j = 0; j != 256; ++j)
	{
		float d9FI = j / 255.0f;

		for (int i = 0; i != gammaCaps.NumGammaControlPoints; ++i)
		{
			float dxgiFI = base + dlt * i;

			if ((dxgiFI >= d9FI) && ((dxgiFI + dlt) < d9FI))
			{
				pRamp->red[j] = (WORD)(curRamp.GammaCurve[i].Red * 65535);
				pRamp->green[j] = (WORD)(curRamp.GammaCurve[i].Green * 65535);
				pRamp->blue[j] = (WORD)(curRamp.GammaCurve[i].Blue * 65535);
				break;
			}

		}
	}
}

void d912pxy_swapchain::ChangeState(d912pxy_swapchain_state newState)
{
	LOG_DBG_DTDM3("swapchain state %S=>%S", d912pxy_swapchain_state_names[state], d912pxy_swapchain_state_names[newState]);
	state = newState;

	if (!currentPP.Windowed)
	{
		if (state == SWCS_SWAPPABLE_EXCLUSIVE)		
			d912pxy_s(psoCache)->LockCompileQue(0);		
		else
			d912pxy_s(psoCache)->LockCompileQue(1);
	}
}

void d912pxy_swapchain::ThrowCritialError(HRESULT ret, const char * msg)
{
	if (FAILED(ret))
	{
		LOG_ERR_DTDM("swapchain criterror: %S | state: %u ", msg, state);
		LOG_ERR_THROW2(-1, msg);
	}
}

void d912pxy_swapchain::ResetFrameTargets()
{	
	FixPresentParameters();

	FreeFrameTargets();	

	backBufferSurface = new d912pxy_surface(
		m_dev, 
		currentPP.BackBufferWidth,
		currentPP.BackBufferHeight,
		currentPP.BackBufferFormat, 
		D3DMULTISAMPLE_NONE, 
		0, 
		0, 
		0
	);

	d912pxy_s(iframe)->BindSurface(1, backBufferSurface);

	if (currentPP.EnableAutoDepthStencil)
	{
		depthStencilSurface = new d912pxy_surface(
			m_dev,
			currentPP.BackBufferWidth,
			currentPP.BackBufferHeight,
			currentPP.AutoDepthStencilFormat,
			D3DMULTISAMPLE_NONE,
			0,
			0,
			1
		);

		d912pxy_s(iframe)->BindSurface(0, depthStencilSurface);
	}	
}

void d912pxy_swapchain::FreeFrameTargets()
{
	if (backBufferSurface)
	{
		backBufferSurface->Release();
		backBufferSurface = NULL;
	}

	if (depthStencilSurface)
	{
		depthStencilSurface->Release();
		depthStencilSurface = NULL;
	}
}

HRESULT d912pxy_swapchain::SwapHandle_Default()
{
	ThrowCritialError(-1, "default handle called in swapchain operation");
	return D3DERR_INVALIDCALL;
}

HRESULT d912pxy_swapchain::SwapHandle_Setup()
{	
	DXGI_SOFT_THROW(InitDXGISwapChain(), "Init");
	DXGI_SOFT_THROW(SetDXGIFullscreen(), "Fullscreen");
	DXGI_SOFT_THROW(ChangeDXGISwapChain(), "Resize");
	DXGI_SOFT_THROW(GetDXGIBuffers(), "Buffers");

	errorCount = 0;
	swapCheckValue = D3D_OK;

	if (currentPP.Windowed)
		ChangeState(SWCS_SWAPPABLE);
	else 
		ChangeState(SWCS_SWAPPABLE_EXCLUSIVE);

	return D3D_OK;
}

HRESULT d912pxy_swapchain::SwapHandle_Init_Error()
{
	++errorCount;

	LOG_ERR_DTDM("Init error, %u in sequence", errorCount);

	if (errorCount > 9)
	{
		LOG_ERR_THROW2(-1, "swap chain 10 init errors, check display settings");		
	}

	ChangeState(SWCS_SETUP);

	return D3D_OK;
}

HRESULT d912pxy_swapchain::SwapHandle_Swappable()
{
	HRESULT ret = dxgiSwapchain->Present(0, dxgiPresentFlags);

	if (!((ret == DXGI_ERROR_WAS_STILL_DRAWING) || (ret == S_OK)))
	{
		//megai2: disable no wait flag and drop back to blocking call if we catch an error, 
		//should fix vrr monitors behaivour on FPS > maxRefreshRate
		dxgiNoWaitFlag = 0;
		LOG_ERR_DTDM("error: %llX", ret);
		ChangeState(SWCS_SWAP_ERROR);
	}
		
	return D3D_OK;
}

HRESULT d912pxy_swapchain::SwapHandle_Swappable_Exclusive()
{
	HRESULT ret = dxgiSwapchain->Present(0, DXGI_PRESENT_TEST);

	if (ret == S_OK)
		ret = dxgiSwapchain->Present(currentPP.PresentationInterval != D3DPRESENT_INTERVAL_IMMEDIATE, dxgiPresentFlags);

	if (ret == DXGI_STATUS_OCCLUDED)
	{
		swapCheckValue = D3DERR_DEVICELOST;		
		ChangeState(SWCS_FOCUS_LOST_SWITCH);
	} else if (FAILED(ret))
	{
		LOG_ERR_DTDM("error: %llX", ret);
		ChangeState(SWCS_SWAP_ERROR);
	}	

	return D3D_OK;
}

HRESULT d912pxy_swapchain::SwapHandle_Reconfigure()
{
	//megai2: if we change fullscreen state: need to recreate whole swapchain
	if (oldPP.Windowed != currentPP.Windowed)
	{
		ChangeState(SWCS_RESETUP);
	}
	else //megai2: TODO add more conditions to RESETUP 
	{		
		FreeDXGISwapChainReferences();
		DXGI_SOFT_THROW(SetDXGIFullscreen(), "Fullscreen");
		DXGI_SOFT_THROW(ChangeDXGISwapChain(), "Resize");
		DXGI_SOFT_THROW(GetDXGIBuffers(), "Buffers");

		errorCount = 0;
		swapCheckValue = D3D_OK;

		if (currentPP.Windowed)
			ChangeState(SWCS_SWAPPABLE);
		else
			ChangeState(SWCS_SWAPPABLE_EXCLUSIVE);
	}

	return D3D_OK;
}

HRESULT d912pxy_swapchain::SwapHandle_Swap_Error()
{
	++errorCount;

	LOG_ERR_DTDM("Swap error, %u in sequence", errorCount);

	if (errorCount > 9)
	{
		LOG_ERR_THROW2(-1, "swap chain 10 swap errors, something go wrong");
	}

	ChangeState(SWCS_RESETUP);

	return D3D_OK;
}

HRESULT d912pxy_swapchain::SwapHandle_Focus_Lost()
{
	ShowWindow(currentPP.hDeviceWindow, SW_FORCEMINIMIZE);

	if (GetForegroundWindow() != currentPP.hDeviceWindow)
	{
		ChangeState(SWCS_FOCUS_PENDING);
	}

	return D3D_OK;
}

HRESULT d912pxy_swapchain::SwapHandle_ReSetup()
{
	dxgiSwapchain->SetFullscreenState(0, NULL);
	FreeDXGISwapChain();
	ChangeState(SWCS_SETUP);
	return D3D_OK;
}

HRESULT d912pxy_swapchain::SwapHandle_Focus_Pending()
{
	if (GetForegroundWindow() == currentPP.hDeviceWindow)
	{
		swapCheckValue = D3DERR_DEVICENOTRESET;
		ChangeState(SWCS_RECONFIGURE);
	}

	return D3D_OK;
}

HRESULT d912pxy_swapchain::SwapHandle_Focus_Lost_Switch()
{
	dxgiSwapchain->SetFullscreenState(0, NULL);
	ChangeState(SWCS_FOCUS_LOST);

	return D3D_OK;
}

HRESULT d912pxy_swapchain::InitDXGISwapChain()
{
	ComPtr<IDXGIFactory4> dxgiFactory4;
	UINT createFactoryFlags = 0;
#ifdef _DEBUG
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	ThrowCritialError(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)), "DXGI factory @ InitDXGISwapChain");

	dxgiBuffersCount = currentPP.BackBufferCount < 2 ? 2 : currentPP.BackBufferCount;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = currentPP.BackBufferWidth;
	swapChainDesc.Height = currentPP.BackBufferHeight;
	swapChainDesc.Format = GetDXGIFormatForBackBuffer(currentPP.BackBufferFormat);
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc = { 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	
	swapChainDesc.BufferCount = dxgiBuffersCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	dxgiResizeFlags = 0;
	dxgiPresentFlags = 0;
	
	if (currentPP.Windowed)
	{
		dxgiResizeFlags |= dxgiTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		dxgiPresentFlags |= dxgiTearingSupported ? DXGI_PRESENT_ALLOW_TEARING : 0;
		dxgiPresentFlags |= dxgiNoWaitFlag;
	}
	else {
		dxgiResizeFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	}

	swapChainDesc.Flags = dxgiResizeFlags;	

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDsc = {};
	swapChainFullscreenDsc.RefreshRate.Numerator = currentPP.FullScreen_RefreshRateInHz;
	swapChainFullscreenDsc.RefreshRate.Denominator = 1;
	swapChainFullscreenDsc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainFullscreenDsc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainFullscreenDsc.Windowed = currentPP.Windowed;
	
	ComPtr<IDXGISwapChain1> swapChain1;

	LOG_INFO_DTDM("Window: %lX Width: %u Height: %u Fullscreen: %u Buffers: %u Format: %u", 
		currentPP.hDeviceWindow, 
		currentPP.BackBufferWidth, 
		currentPP.BackBufferHeight, 
		!currentPP.Windowed,
		dxgiBuffersCount,
		currentPP.BackBufferFormat
	);

	HRESULT swapRet = dxgiFactory4->CreateSwapChainForHwnd(
		d912pxy_s(GPUque)->GetDXQue().Get(),
		currentPP.hDeviceWindow,
		&swapChainDesc,
		nullptr,//currentPP.Windowed ? nullptr : &swapChainFullscreenDsc,
		nullptr,
		&swapChain1
	);
	
	if (FAILED(swapRet))
	{		
		return swapRet;
	}
	else {
		// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen will be handled manually.
		ThrowCritialError(dxgiFactory4->MakeWindowAssociation(currentPP.hDeviceWindow, DXGI_MWA_NO_ALT_ENTER), "DXGI window assoc @ InitDXGISwapChain");
		ThrowCritialError(swapChain1.As(&dxgiSwapchain), "DXGI swap chain 1->4 @ InitDXGISwapChain");
		return swapRet;
	}	
}

void d912pxy_swapchain::FreeDXGISwapChain()
{
	FreeDXGISwapChainReferences();

	if (!currentPP.Windowed && dxgiSwapchain)
	{
		dxgiSwapchain->SetFullscreenState(0, NULL);
	}

	dxgiSwapchain = nullptr;
}

HRESULT d912pxy_swapchain::GetDXGIBuffers()
{
	ComPtr<ID3D12Resource> bbRes[4];

	for (int i = 0; i != dxgiBuffersCount; ++i)
	{
		HRESULT cr = dxgiSwapchain->GetBuffer(i, IID_PPV_ARGS(&bbRes[i]));
		if (FAILED(cr))
			return cr;
	}

	for (int i = 0; i != dxgiBuffersCount; ++i)
	{
		dxgiBackBuffer[i] = bbRes[i];
	}

	return S_OK;
}

void d912pxy_swapchain::FreeDXGISwapChainReferences()
{
	for (int i = 0; i != dxgiBuffersCount; ++i)
	{
		dxgiBackBuffer[i] = nullptr;
	}
}

HRESULT d912pxy_swapchain::ChangeDXGISwapChain()
{
	return dxgiSwapchain->ResizeBuffers(dxgiBuffersCount, currentPP.BackBufferWidth, currentPP.BackBufferHeight, GetDXGIFormatForBackBuffer(currentPP.BackBufferFormat), dxgiResizeFlags);
}

HRESULT d912pxy_swapchain::SetDXGIFullscreen()
{
	if (!currentPP.Windowed)
	{
		DXGI_MODE_DESC mdsc;
		mdsc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		mdsc.Height = currentPP.BackBufferHeight;
		mdsc.Width = currentPP.BackBufferWidth;
		mdsc.RefreshRate.Denominator = 1;
		mdsc.RefreshRate.Numerator = currentPP.FullScreen_RefreshRateInHz;
		mdsc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		mdsc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		HRESULT cr = dxgiSwapchain->ResizeTarget(&mdsc);

		if (FAILED(cr))
			return cr;
	}

	return dxgiSwapchain->SetFullscreenState(!currentPP.Windowed, NULL);
}

void d912pxy_swapchain::FixPresentParameters()
{
	switch (currentPP.BackBufferFormat)
	{
		case D3DFMT_X8R8G8B8:
		case D3DFMT_UNKNOWN:
			currentPP.BackBufferFormat = D3DFMT_A8R8G8B8;
			break;
	}
}

DXGI_FORMAT d912pxy_swapchain::GetDXGIFormatForBackBuffer(D3DFORMAT fmt)
{
	DXGI_FORMAT ret = d912pxy_helper::DXGIFormatFromDX9FMT(currentPP.BackBufferFormat);
	return ret;
}

void d912pxy_swapchain::CacheDXGITearingSupport()
{
	dxgiTearingSupported = FALSE;

	ComPtr<IDXGIFactory4> factory4;
	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
	{
		ComPtr<IDXGIFactory5> factory5;
		if (SUCCEEDED(factory4.As(&factory5)))
		{
			if (FAILED(factory5->CheckFeatureSupport(
				DXGI_FEATURE_PRESENT_ALLOW_TEARING,
				&dxgiTearingSupported, sizeof(dxgiTearingSupported))))
			{
				dxgiTearingSupported = FALSE;
			}
		}
	}
}

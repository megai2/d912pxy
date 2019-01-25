#include "IDirect3D9Proxy.h"
#include "IDirect3DDevice9Proxy.h"

IDirect3D9Proxy::IDirect3D9Proxy(IDirect3D9 *pOriginal){
	origIDirect3D9 = pOriginal;
}

IDirect3D9Proxy::~IDirect3D9Proxy(void){
}

HRESULT WINAPI IDirect3D9Proxy::QueryInterface(REFIID riid, void** ppvObj){
	*ppvObj = NULL;
	// call this to increase AddRef at original object
	// and to check if such an interface is there
	HRESULT hRes = origIDirect3D9->QueryInterface(riid, ppvObj);
	// if OK, send our "fake" address
	if (hRes == NOERROR)
		*ppvObj = this;
	return hRes;
}

ULONG WINAPI IDirect3D9Proxy::AddRef(void){
	return(origIDirect3D9->AddRef());
}

ULONG WINAPI IDirect3D9Proxy::Release(void){
	// call original routine
	ULONG count = origIDirect3D9->Release();
	// in case no further Ref is there, the Original Object has deleted itself
	// so do we here
	if (count == 0){
		delete(this); 
	}
	return(count);
}

HRESULT WINAPI IDirect3D9Proxy::RegisterSoftwareDevice(void* pInitializeFunction){
	return(origIDirect3D9->RegisterSoftwareDevice(pInitializeFunction));
}

UINT WINAPI IDirect3D9Proxy::GetAdapterCount(void){
	return(origIDirect3D9->GetAdapterCount());
}

HRESULT WINAPI IDirect3D9Proxy::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier){
	return(origIDirect3D9->GetAdapterIdentifier(Adapter, Flags, pIdentifier));
}

UINT WINAPI IDirect3D9Proxy::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format){
	return(origIDirect3D9->GetAdapterModeCount(Adapter, Format));
}

HRESULT WINAPI IDirect3D9Proxy::EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode){
	return(origIDirect3D9->EnumAdapterModes(Adapter, Format, Mode, pMode));
}

HRESULT WINAPI IDirect3D9Proxy::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode){
	return(origIDirect3D9->GetAdapterDisplayMode(Adapter, pMode));
}

HRESULT WINAPI IDirect3D9Proxy::CheckDeviceType(UINT iAdapter, D3DDEVTYPE DevType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed){
	return(origIDirect3D9->CheckDeviceType(iAdapter, DevType, DisplayFormat, BackBufferFormat, bWindowed));
}

HRESULT WINAPI IDirect3D9Proxy::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType,D3DFORMAT CheckFormat){
	return(origIDirect3D9->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat));
}

HRESULT WINAPI IDirect3D9Proxy::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels){
	return(origIDirect3D9->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels));
}

HRESULT WINAPI IDirect3D9Proxy::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat){
	return(origIDirect3D9->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat));
}

HRESULT WINAPI IDirect3D9Proxy::CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat){
	return(origIDirect3D9->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat));
}

HRESULT WINAPI IDirect3D9Proxy::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps){
	return(origIDirect3D9->GetDeviceCaps(Adapter, DeviceType, pCaps));
}

HMONITOR WINAPI IDirect3D9Proxy::GetAdapterMonitor(UINT Adapter){
	return(origIDirect3D9->GetAdapterMonitor(Adapter));
}

HRESULT WINAPI IDirect3D9Proxy::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface){
	IDirect3DDevice9Proxy* tmp;
	//Recall original function
	HRESULT hres = origIDirect3D9->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	//Create fake for the device, using original device.
	tmp = new IDirect3DDevice9Proxy(*ppReturnedDeviceInterface);

	//Replace returned device with our fake class
	*ppReturnedDeviceInterface = tmp;
	return hres; 
}
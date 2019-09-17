/* 

Original work: https://github.com/iorlas/D3D9Proxy

*/
//FIXME: correct this to be right, as i'm not shure
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

#pragma pack(1)
HINSTANCE hlThis = 0;
HINSTANCE hlD3D9 = 0;

d3d9ProxyCB_OnDevCreate pxCb_devCreate = NULL;
d3d9ProxyCB_OnDevDestroy pxCb_devDestroy = NULL;

//megai2: added for win10 update compat

typedef void(*d3d9_ordinal_fn)();

#ifdef _DEBUG
#define d3d9_ordinal_export_passthru(a) d3d9_ordinal_fn d3d9_ordinal_ptr_##a; void d3d9_Ordinal##a() { return; }
#else
#define d3d9_ordinal_export_passthru(a) d3d9_ordinal_fn d3d9_ordinal_ptr_##a; void d3d9_Ordinal##a() { d3d9_ordinal_ptr_##a(); return; }
#endif

#define d3d9_ordinal_get_orig_adr(a) d3d9_ordinal_ptr_##a = (d3d9_ordinal_fn)GetProcAddress(hlD3D9, MAKEINTRESOURCEA(a))

#define d3d9_ordinal_gen_macro(a) a(16); a(17); a(18); a(19); a(20); a(21); a(22); a(23);

d3d9_ordinal_gen_macro(d3d9_ordinal_export_passthru);

//

bool loadD3D9dll()
{
	//Get path to the original d3d9.dll
	wchar_t infoBuf[4096];
	GetSystemDirectory(infoBuf, 4096);
	lstrcatW(infoBuf, L"\\d3d9.dll");

	//And load it, but before we do that, load d3d12 dll to fix wrong hooking behaivour on various tools
	d912pxy_s.imports.Init();

	hlD3D9 = LoadLibrary(infoBuf);
	if (!hlD3D9) {
		MessageBox(NULL, L"Cannot find original d3d9.dll in the system directory!", L"D3D9 Proxy DLL error", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	else {
		d3d9_ordinal_gen_macro(d3d9_ordinal_get_orig_adr);		
		return TRUE;
	}
}

bool d3d9_proxy_dll_main(HINSTANCE hInst, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH) {
		hlThis = hInst;
	}
	else if (reason == DLL_PROCESS_DETACH) {
		if (hlD3D9)
			FreeLibrary(hlD3D9);
	}
	return TRUE;
}

void D3D9ProxyCb_set_OnDevCreate(d3d9ProxyCB_OnDevCreate pxFun) {
	pxCb_devCreate = pxFun;
}

d3d9ProxyCB_OnDevCreate D3D9ProxyCb_get_OnDevCreate() {
	return pxCb_devCreate;
}

//Direct3DCreate9
extern "C" IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion) {

	//Recall original function
	typedef IDirect3D9* (WINAPI* Direct3DCreate9Func)(UINT sdkver);

	if (!hlD3D9)
		loadD3D9dll();

	Direct3DCreate9Func origDirect3DCreate9 = (Direct3DCreate9Func)GetProcAddress(hlD3D9, "Direct3DCreate9");
	IDirect3D9* res = origDirect3DCreate9(SDKVersion);

	return new IDirect3D9Proxy(res);
}

extern "C" HRESULT WINAPI Direct3DCreate9Ex(
	_In_  UINT         SDKVersion,
	_Out_ IDirect3D9Ex **ppD3D
)
{
	typedef HRESULT (WINAPI* Direct3DCreate9Ex_t)(UINT SDKVersion, IDirect3D9Ex ** ppD3D);

	static Direct3DCreate9Ex_t _imp_Direct3DCreate9Ex = NULL;

	if (!hlD3D9)
		loadD3D9dll();

	if (hlD3D9 && !_imp_Direct3DCreate9Ex) {
		_imp_Direct3DCreate9Ex = (Direct3DCreate9Ex_t)GetProcAddress(hlD3D9, "Direct3DCreate9Ex");
	}

	if (_imp_Direct3DCreate9Ex)
		return _imp_Direct3DCreate9Ex(SDKVersion, ppD3D);

	return D3DERR_NOTAVAILABLE;
}


extern "C" int WINAPI D3DPERF_BeginEvent(D3DCOLOR col, LPCWSTR wszName)
{

	typedef int (WINAPI* BeginEvent_t)(D3DCOLOR, LPCWSTR);
	static BeginEvent_t _imp_BeginEvent = NULL;

	if (!hlD3D9)
		loadD3D9dll();

	if (hlD3D9 && !_imp_BeginEvent)
		_imp_BeginEvent = (BeginEvent_t)GetProcAddress(hlD3D9, "D3DPERF_BeginEvent");

	if (_imp_BeginEvent)
		return _imp_BeginEvent(col, wszName);

	return D3DERR_NOTAVAILABLE;
}

extern "C" int WINAPI D3DPERF_EndEvent(void)
{

	typedef int (WINAPI* EndEvent_t)(void);
	static EndEvent_t _imp_EndEvent = NULL;

	if (!hlD3D9)
		loadD3D9dll();

	if (hlD3D9 && !_imp_EndEvent)
		_imp_EndEvent = (EndEvent_t)GetProcAddress(hlD3D9, "D3DPERF_EndEvent");

	if (_imp_EndEvent)
		return _imp_EndEvent();

	return D3DERR_NOTAVAILABLE;
}

extern "C" void WINAPI D3DPERF_SetMarker(D3DCOLOR col, LPCWSTR wszName)
{

	typedef VOID(WINAPI* Direct3DSet_t)(D3DCOLOR, LPCWSTR);
	static Direct3DSet_t _imp_SetMarker = NULL;
	
	if (!hlD3D9)
		loadD3D9dll();

	if (hlD3D9 && !_imp_SetMarker)
		_imp_SetMarker = (Direct3DSet_t)GetProcAddress(hlD3D9, "D3DPERF_SetMarker");

	if (_imp_SetMarker)
		_imp_SetMarker(col, wszName);
}

extern "C" void WINAPI D3DPERF_SetRegion(D3DCOLOR col, LPCWSTR wszName)
{

	typedef VOID(WINAPI* Direct3DSet_t)(D3DCOLOR, LPCWSTR);
	static Direct3DSet_t _imp_SetRegion = NULL;
	
	if (!hlD3D9)
		loadD3D9dll();

	if (hlD3D9 && !_imp_SetRegion)
		_imp_SetRegion = (Direct3DSet_t)GetProcAddress(hlD3D9, "D3DPERF_SetRegion");

	if (_imp_SetRegion)
		_imp_SetRegion(col, wszName);
}

extern "C" BOOL WINAPI D3DPERF_QueryRepeatFrame(void)
{

	typedef BOOL(WINAPI* QueryRepeatFrame_t)(void);
	static QueryRepeatFrame_t _imp_QueryRepeatFrame = NULL;
	
	if (!hlD3D9)
		loadD3D9dll();

	if (hlD3D9 && !_imp_QueryRepeatFrame)
		_imp_QueryRepeatFrame = (QueryRepeatFrame_t)GetProcAddress(hlD3D9, "D3DPERF_QueryRepeatFrame");

	if (_imp_QueryRepeatFrame)
		return _imp_QueryRepeatFrame();

	return FALSE;
}

extern "C" void WINAPI D3DPERF_SetOptions(DWORD dwOptions)
{
	typedef void (WINAPI* SetOptions_t)(DWORD);
	static SetOptions_t _imp_SetOptions = NULL;
	
	if (!hlD3D9)
		loadD3D9dll();

	if (hlD3D9 && !_imp_SetOptions)
		_imp_SetOptions = (SetOptions_t)GetProcAddress(hlD3D9, "D3DPERF_SetOptions");

	if (_imp_SetOptions)
		_imp_SetOptions(dwOptions);
}

extern "C" DWORD WINAPI D3DPERF_GetStatus(void)
{
	typedef DWORD(WINAPI* GetStatus_t)(void);
	static GetStatus_t _imp_GetStatus = NULL;
	
	if (!hlD3D9)
		loadD3D9dll();

	if (hlD3D9 && !_imp_GetStatus)
		_imp_GetStatus = (GetStatus_t)GetProcAddress(hlD3D9, "D3DPERF_GetStatus");

	if (_imp_GetStatus)
		return _imp_GetStatus();

	return 0;
}

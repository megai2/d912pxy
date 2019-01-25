#include "defines.h"
#include "IDirect3D9Proxy.h"

#pragma pack(1)
HINSTANCE hlThis = 0;
HINSTANCE hlD3D9 = 0;
FARPROC origProc[15] = {0};

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID){
	if (reason == DLL_PROCESS_ATTACH){
		hlThis = hInst;

		//Get path to the original d3d9.dll
		char infoBuf[MAX_PATH];
		GetSystemDirectory(infoBuf, MAX_PATH);
		strcat_s(infoBuf, MAX_PATH, "\\d3d9.dll");

		//And load it...
		hlD3D9 = LoadLibrary(infoBuf);
		if (!hlD3D9){
			MessageBox(NULL, "D3D9 Proxy DLL error", "Cannot find original d3d9.dll in the system directory!", MB_OK | MB_ICONERROR);
			return FALSE;
		}

		//Load original functions
		origProc[0] = GetProcAddress(hlD3D9, "D3DPERF_BeginEvent");
		origProc[1] = GetProcAddress(hlD3D9, "D3DPERF_EndEvent");
		origProc[2] = GetProcAddress(hlD3D9, "D3DPERF_GetStatus");
		origProc[3] = GetProcAddress(hlD3D9, "D3DPERF_QueryRepeatFrame");
		origProc[4] = GetProcAddress(hlD3D9, "D3DPERF_SetMarker");
		origProc[5] = GetProcAddress(hlD3D9, "D3DPERF_SetOptions");
		origProc[6] = GetProcAddress(hlD3D9, "D3DPERF_SetRegion");
		origProc[7] = GetProcAddress(hlD3D9, "DebugSetLevel");
		origProc[8] = GetProcAddress(hlD3D9, "DebugSetMute");
		origProc[9] = GetProcAddress(hlD3D9, "Direct3DCreate9");
		origProc[10] = GetProcAddress(hlD3D9, "Direct3DCreate9Ex");
		origProc[11] = GetProcAddress(hlD3D9, "Direct3DShaderValidatorCreate9");
		origProc[12] = GetProcAddress(hlD3D9, "PSGPError");
		origProc[13] = GetProcAddress(hlD3D9, "PSGPSampleTexture");
	}else if (reason == DLL_PROCESS_DETACH){
		FreeLibrary(hlD3D9);
	}
	return TRUE;
}

//Direct3DCreate9
extern "C" IDirect3D9* WINAPI __ProxyFunc9(UINT SDKVersion){

	//Recall original function
	typedef IDirect3D9* (WINAPI* Direct3DCreate9Func)(UINT sdkver);
	Direct3DCreate9Func origDirect3DCreate9 = (Direct3DCreate9Func)GetProcAddress(hlD3D9, "Direct3DCreate9");
	IDirect3D9* res = origDirect3DCreate9(SDKVersion);
	return new IDirect3D9Proxy(res);
}
//Direct3DCreate9Ex
extern "C" __declspec(naked) void __stdcall __ProxyFunc10(){
	__asm{
		jmp origProc[10*4];
	}
}

extern "C" __declspec(naked) void __stdcall __ProxyFunc0(){
	__asm{
		jmp origProc[0*4];
	}
}
extern "C" __declspec(naked) void __stdcall __ProxyFunc1(){
	__asm{
		jmp origProc[1*4];
	}
}
extern "C" __declspec(naked) void __stdcall __ProxyFunc2(){
	__asm{
		jmp origProc[2*4];
	}
}
extern "C" __declspec(naked) void __stdcall __ProxyFunc3(){
	__asm{
		jmp origProc[3*4];
	}
}
extern "C" __declspec(naked) void __stdcall __ProxyFunc4(){
	__asm{
		jmp origProc[4*4];
	}
}
extern "C" __declspec(naked) void __stdcall __ProxyFunc5(){
	__asm{
		jmp origProc[5*4];
	}
}
extern "C" __declspec(naked) void __stdcall __ProxyFunc6(){
	__asm{
		jmp origProc[6*4];
	}
}
extern "C" __declspec(naked) void __stdcall __ProxyFunc7(){
	__asm{
		jmp origProc[7*4];
	}
}
extern "C" __declspec(naked) void __stdcall __ProxyFunc8(){
	__asm{
		jmp origProc[8*4];
	}
}
extern "C" __declspec(naked) void __stdcall __ProxyFunc11(){
	__asm{
		jmp origProc[11*4];
	}
}
extern "C" __declspec(naked) void __stdcall __ProxyFunc12(){
	__asm{
		jmp origProc[12*4];
	}
}
extern "C" __declspec(naked) void __stdcall __ProxyFunc13(){
	__asm{
		jmp origProc[13*4];
	}
}
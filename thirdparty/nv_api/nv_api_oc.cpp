/*
			DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
					Version 2, December 2004

 Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

			DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.
*/
//https://1vwjbxf1wko0yhnr.wordpress.com/2015/08/10/overclocking-tools-for-nvidia-gpus-suck-i-made-my-own/
#include "stdafx.h"

nvapi_fptrs gNvapi;

nvapi_fptrs* init_nv_api_oc()
{		
	HMODULE nvApiDll = LoadLibrary(L"nvapi64.dll");

	if (!nvApiDll)
		return 0;

	gNvapi.ptrs[0] = GetProcAddress(nvApiDll, "nvapi_QueryInterface");

	if (!gNvapi.ptrs[0])
		return 0;

	gNvapi.ptrs[1] = gNvapi.QueryInterface(0x0150E828);
	gNvapi.ptrs[2] = gNvapi.QueryInterface(0xD22BDD7E);
	gNvapi.ptrs[3] = gNvapi.QueryInterface(0xE5AC921F);
	gNvapi.ptrs[4] = gNvapi.QueryInterface(0xBAAABFCC);
	gNvapi.ptrs[5] = gNvapi.QueryInterface(0xCEEE8E9F);
	gNvapi.ptrs[6] = gNvapi.QueryInterface(0x46FBEB03);
	gNvapi.ptrs[7] = gNvapi.QueryInterface(0x57F7CAAC);
	gNvapi.ptrs[8] = gNvapi.QueryInterface(0xA561FD7D);
	gNvapi.ptrs[9] = gNvapi.QueryInterface(0xDCB616C3);
	gNvapi.ptrs[10] = gNvapi.QueryInterface(0x6FF81213);
	gNvapi.ptrs[11] = gNvapi.QueryInterface(0x0F4DAE6B);
	gNvapi.ptrs[12] = gNvapi.QueryInterface(0x2926AAAD);    
	gNvapi.ptrs[13] = gNvapi.QueryInterface(0x0FA579A0F);
	gNvapi.ptrs[14] = gNvapi.QueryInterface(0x60DED2ED);

	for (int i = 1; i != 14; ++i)
		if (!gNvapi.ptrs[i])
			return 0;

	return &gNvapi;
}
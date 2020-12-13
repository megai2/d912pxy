#include "stdafx.h"
#include "../d912pxy/build_version.h"

//gw2addon_get_description
//gw2addon_load
//gw2addon_unload

gw2al_addon_dsc gAddonDeps[] = {
	{
		L"loader_core",
		L"whatever",
		0,
		1,
		1,
		0
	},
	{
		L"d3d9_wrapper",
		L"whatever",
		1,
		0,
		1,
		0
	},
	{0,0,0,0,0,0}
};

gw2al_addon_dsc gAddonDsc = {
	L"d912pxy",
	L"DirectX9 to DirectX12 API proxy, designed for performance improvements",
	2,
	3,
	BUILD_VERSION_REV,
	gAddonDeps
};

HMODULE custom_d3d9_module;

gw2al_core_vtable* instance::api = NULL;

#include "../thirdparty/cpu_arch_test.inc"

wchar_t* GetD3D9CustomLib()
{
	cpu_arch arch = GetCPUArch();

	if (arch.AVX2)
		return (wchar_t*)L"./addons/d912pxy/dll/release_avx2/d3d9.dll";
	else if (arch.AVX)
		return (wchar_t*)L"./addons/d912pxy/dll/release_avx/d3d9.dll";
	else if (arch.SSE)
		return (wchar_t*)L"./addons/d912pxy/dll/release/d3d9.dll";
	else 
	{
		gAPI->log_text(LL_ERR, (wchar_t*)L"d912pxy", (wchar_t*)L"can't load due to not supported CPU");
		return (wchar_t*)L"d3d9.dll";
	}
}

gw2al_addon_dsc* gw2addon_get_description()
{
	return &gAddonDsc;
}

gw2al_api_ret gw2addon_load(gw2al_core_vtable* core_api)
{
	gAPI = core_api;

	gAPI->register_function(&GetD3D9CustomLib, gAPI->hash_name((wchar_t*)L"D3D_wrapper_custom_d3d9_lib_query"));
		
	return GW2AL_OK;
}

gw2al_api_ret gw2addon_unload(int gameExiting)
{
	//TODO cleanup


	return GW2AL_OK;
}
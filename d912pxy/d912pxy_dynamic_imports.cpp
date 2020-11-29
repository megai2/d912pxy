/*
MIT License

Copyright(c) 2019 megai2

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

d912pxy_dynamic_imports::d912pxy_dynamic_imports()
{
}

d912pxy_dynamic_imports::~d912pxy_dynamic_imports()
{
}

void d912pxy_dynamic_imports::Init()
{
	NonCom_Init(L"dyn_imports");

	LOG_INFO_DTDM("Loading dynamic libs");

	//megai2: try to load rdoc first if specified
	if (d912pxy_s.config.GetValueUI32(PXY_CFG_LOG_LOAD_RDOC))
	{	
		HMODULE mod = LoadLibraryA("renderdoc.dll");
		LOG_INFO_DTDM("RDoc loaded at %016llX", mod);
	}


	UINT dynLibSet = LoadDynLib(DYNIMP_DX12, DYNIMP_LIB_SET_WIN7) ? DYNIMP_LIB_SET_WIN7 : DYNIMP_LIB_SET_WIN10;
	bool isWin7 = (dynLibSet == DYNIMP_LIB_SET_WIN7);

	if (isWin7)
		LOG_INFO_DTDM("Pepe: Is this windows 7? windows 7 right.");
	else if (d912pxy_s.config.GetValueUI32(PXY_CFG_COMPAT_EXPLICIT_D3DCOMPILER) > 0)
	{
#if _WIN64
		dynLibSet = DYNIMP_LIB_SET_WIN10_EXPLICIT_COMPILER_X64;
#else
		dynLibSet = DYNIMP_LIB_SET_WIN10_EXPLICIT_COMPILER_X86;
#endif
	}

	for (int i = isWin7 ? DYNIMP_D3DCOMPILER : DYNIMP_DX12; i != DYNIMP_COUNT; ++i)
	{
		if (!LoadDynLib((d912pxy_dynamic_import_lib)i, dynLibSet))
		{
			LOG_ERR_DTDM("Dyn lib %s %u failed to load", d912pxy_dynamic_import_libs[dynLibSet].dllName[i], dynLibSet);
			LOG_ERR_THROW2(-1, "Dyn lib failed to load");
		}
	}

	UINT funIdx = 0;

	while (d912pxy_dynamic_import_funcs[funIdx].fn)
	{
		d912pxy_dynamic_import_lib libId = d912pxy_dynamic_import_funcs[funIdx].libId;

		import_fun_ptrs[funIdx] = GetProcAddress(libHandles[libId], d912pxy_dynamic_import_funcs[funIdx].fn);

		if (!import_fun_ptrs[funIdx])
		{
			LOG_ERR_DTDM("Failed to import function %s from %s(%u)", d912pxy_dynamic_import_funcs[funIdx].fn, d912pxy_dynamic_import_libs[dynLibSet].dllName[libId], dynLibSet);
			LOG_ERR_THROW2(-1, "Dynlib function import fail");
		}
		++funIdx;
	}

	LOG_INFO_DTDM("All dynamic imports are fine");
}

UINT d912pxy_dynamic_imports::LoadDynLib(d912pxy_dynamic_import_lib lib, UINT set)
{
	if (d912pxy_dynamic_import_libs[set].dllName[lib])
	{
		wchar_t buf[1024];
		wsprintf(buf, L"%s%s", d912pxy_helper::GetFilePath(d912pxy_dynamic_import_libs[set].pathPrefix[lib])->w, d912pxy_dynamic_import_libs[set].dllName[lib]);

		libHandles[lib] = LoadLibrary(buf);

		if (libHandles[lib])
			LOG_INFO_DTDM("set %u lib %u loaded from %s at %016llX", set, lib, buf, libHandles[lib]);

		return libHandles[lib] != 0;
	}
	else
		libHandles[lib] = NULL;

	return 1;
}

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
#include "stdafx.h"
#include "../thirdparty/renderdoc_app.h"

d912pxy_instance d912pxy_global::instance;

IDirect3DDevice9* app_cb_D3D9Dev_create(IDirect3DDevice9Proxy* dev, IDirect3D9* obj)
{
	d912pxy_first_init();
	
	if (d912pxy_s.config.GetValueUI32(PXY_CFG_DX_ROUTE_TO_DX9))
	{
		dev->InitPerfGraph();
		return dev;
	} else 
		if (dev->GetOrigD3D9Call()->DeviceType != D3DDEVTYPE_HAL)
			return dev;		
		
	d912pxy_s.mem.StartTrackingBlocks();

	d912pxy_com_object* d912translator = d912pxy_device::d912pxy_device_com(&d912pxy_s.dev, dev, obj);
	return (IDirect3DDevice9*)d912translator;
}

void DetectFilePaths()
{
	//megai2: check if we are installed as gw2 addon
	if (d912pxy_helper::IsFileExist("./addons/d912pxy/dll/release/d3d9.dll"))
		d912pxy_helper::SwitchFilePaths((d912pxy_file_path*)d912pxy_file_paths_addon);
	else
	{
		//megai2: check if we running specific game
		static const int maxPathLen = 4069;

		char binaryPath[maxPathLen];
		if (GetModuleFileNameA(GetModuleHandleA(nullptr), binaryPath, maxPathLen))
		{
			//  Astellia
			//working dir is not same as exe folder & anticheats/whatever blocking that dir
			//use 2 way failsafe
			//  replace relative path to absolute
			//  search d912pxy installation up in path so user can install d912pxy out of game blocked folders
			if (strstr(binaryPath, "Astellia.exe") != nullptr)
			{
				//navigate up until we find d912pxy installation
				bool foundInstallation = false;
				while (d912pxy_helper::StrCutLastElementInPath(binaryPath))
				{
					char testFile[maxPathLen];
					sprintf_s(testFile, "%s%s", binaryPath, "d912pxy/dll/release/d3d9.dll");

					if (d912pxy_helper::IsFileExist(testFile))
					{
						foundInstallation = true;
						break;
					}
				}

				if (!foundInstallation)
				{
					MessageBoxA(
						0,
						"d912pxy installation is not found, make sure there is d912pxy folder in one of folders on route to Astellia installation and it is writable&accesible",
						"d912pxy fatal",
						MB_ICONERROR
					);
					abort();
				}

				d912pxy_mem_block::allocZero(&d912pxy_s.dynamicFilePaths, FP_NO_PATH + 1);
				for (int i = 0; i != FP_NO_PATH; ++i)
				{
					d912pxy_mem_block::allocZero(&d912pxy_s.dynamicFilePaths[i].ds, maxPathLen);
					d912pxy_mem_block::allocZero(&d912pxy_s.dynamicFilePaths[i].dw, maxPathLen);

					sprintf(d912pxy_s.dynamicFilePaths[i].ds, "%s%s", binaryPath, d912pxy_file_paths_abs_rh[i].s);
					wsprintf(d912pxy_s.dynamicFilePaths[i].dw, L"%S%s", binaryPath, d912pxy_file_paths_abs_rh[i].w);
				}

				d912pxy_mem_block::allocZero(&d912pxy_s.dynamicFilePaths[FP_NO_PATH].ds, maxPathLen);
				d912pxy_mem_block::allocZero(&d912pxy_s.dynamicFilePaths[FP_NO_PATH].dw, maxPathLen);
				d912pxy_helper::SwitchFilePaths((d912pxy_file_path*)d912pxy_s.dynamicFilePaths);
			}
		}

	}
}

void d912pxy_first_init()
{
	//megai2: load config at dll load
	//also do dirty tricks with memmgr
	if (d912pxy_s.running)
		return;

	d912pxy_s.running = 1;
	d912pxy_s.dynamicFilePaths = nullptr;
	
	DetectFilePaths();

	d912pxy_s.dev_vtable = NULL;
	d912pxy_s.mem.Init();	
	d912pxy_s.config.Init();
	d912pxy_s.log.text.Init();	   		
	d912pxy_s.mem.PostInit();
	d912pxy_s.imports.Init();

	D3D9ProxyCb_set_OnDevCreate(&app_cb_D3D9Dev_create);
}

void d912pxy_final_cleanup()
{
	if (!d912pxy_s.running)
		return;
	
	d912pxy_s.imports.UnInit();	
	d912pxy_s.pool.hostPow2.UnInit();
	d912pxy_s.mem.UnInit();
	d912pxy_s.log.text.UnInit();

	if (d912pxy_s.dynamicFilePaths)
	{
		for (int i = 0; i != FP_NO_PATH + 1; ++i)
		{
			PXY_FREE(d912pxy_s.dynamicFilePaths[i].ds);
			PXY_FREE(d912pxy_s.dynamicFilePaths[i].dw);
		}

		PXY_FREE(d912pxy_s.dynamicFilePaths);
	}
	
	d912pxy_s.running = 0;
}

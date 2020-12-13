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
#pragma once
#include "stdafx.h"

typedef enum d912pxy_dynamic_import_lib {
	DYNIMP_DX12,
	DYNIMP_D3DCOMPILER,
	DYNIMP_DXILCONV7,
	DYNIMP_COUNT
} d912pxy_dynamic_import_lib;

typedef struct d912pxy_dynamic_import_lib_array {
	const wchar_t* dllName[DYNIMP_COUNT];
	d912pxy_file_path_id pathPrefix[DYNIMP_COUNT];
} d912pxy_dynamic_import_lib_array;

#define DYNIMP_LIB_SET_WIN10 0
#define DYNIMP_LIB_SET_WIN7 1
#define DYNIMP_LIB_SET_WIN10_EXPLICIT_COMPILER_X64 2
#define DYNIMP_LIB_SET_WIN10_EXPLICIT_COMPILER_X86 3

static const d912pxy_dynamic_import_lib_array d912pxy_dynamic_import_libs[] = {
	{
		{
			L"d3d12.dll",
			L"d3dcompiler_47.dll",
			NULL
		},
		{
			FP_NO_PATH,
			FP_NO_PATH,
			FP_NO_PATH
		}
	}, 
	{
		{
			L"d3d12.dll",
			L"d3dcompiler_47_v10.dll",
			L"dxilconv7.dll"
		},
		{
			FP_W7_12ON7,
			FP_W7_12ON7,
			FP_W7_12ON7
		}
	},
	{
		{
			L"d3d12.dll",
			L"d3dcompiler_47_v10_x64.dll",
			NULL
		},
		{
			FP_NO_PATH,
			FP_PROXY_DATA_PATH,
			FP_NO_PATH
		}
	},
	{
		{
			L"d3d12.dll",
			L"d3dcompiler_47_v10_x86.dll",
			NULL
		},
		{
			FP_NO_PATH,
			FP_PROXY_DATA_PATH,
			FP_NO_PATH
		}
	}
};

typedef struct d912pxy_dynamic_import_entry {
	const char* fn;
	d912pxy_dynamic_import_lib libId;
} d912pxy_dynamic_import_entry;

static const d912pxy_dynamic_import_entry d912pxy_dynamic_import_funcs[] = {
	{ "D3D12CreateDevice", DYNIMP_DX12 },
	{ "D3D12SerializeRootSignature", DYNIMP_DX12 },
	{ "D3DCompile", DYNIMP_D3DCOMPILER },
	{ "D3DCompileFromFile", DYNIMP_D3DCOMPILER },
	{ NULL, DYNIMP_DX12 }
};

typedef HRESULT(WINAPI *pD3DCompile_47)
(LPCVOID                         pSrcData,
	SIZE_T                          SrcDataSize,
	LPCSTR                          pFileName,
	CONST D3D_SHADER_MACRO*         pDefines,
	ID3DInclude*                    pInclude,
	LPCSTR                          pEntrypoint,
	LPCSTR                          pTarget,
	UINT                            Flags1,
	UINT                            Flags2,
	ID3DBlob**                      ppCode,
	ID3DBlob**                      ppErrorMsgs);

typedef HRESULT(WINAPI *pD3DCompileFromFile_47)(
	LPCWSTR pFileName,
	CONST D3D_SHADER_MACRO* pDefines,
	ID3DInclude* pInclude,
	LPCSTR pEntrypoint,
	LPCSTR pTarget,
	UINT Flags1,
	UINT Flags2,
	ID3DBlob** ppCode,
	ID3DBlob** ppErrorMsgs
	);

class d912pxy_dynamic_imports : public d912pxy_noncom 
{
public:
	d912pxy_dynamic_imports();
	~d912pxy_dynamic_imports();

	void Init();
	
	union {
		struct {
			struct
			{
				PFN_D3D12_CREATE_DEVICE CreateDevice;
				PFN_D3D12_SERIALIZE_ROOT_SIGNATURE SerializeRootSignature;
			} dx12;

			struct {
				pD3DCompile_47 Compile;
				pD3DCompileFromFile_47 CompileFromFile;
			} d3d_compiler;
		};
		void* import_fun_ptrs[4];
	};

private:

	UINT LoadDynLib(d912pxy_dynamic_import_lib lib, UINT set);
	HMODULE libHandles[DYNIMP_COUNT];

};
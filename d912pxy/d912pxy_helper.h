/*
MIT License

Copyright(c) 2018 Jeremiah van Oosten
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
#include <sys/stat.h>

#define D3DFMT_ATI2 0x32495441
#define D3DFMT_INTZ 0x5A544E49
#define D3DFMT_RAWZ 0x5a574152
#define D3DFMT_NULL 0x4C4C554E

using namespace Microsoft::WRL;

typedef struct fv4Color {
	float val[4];
} fv4Color;

namespace d912pxy_helper {

	LONG NTAPI VexHandler(PEXCEPTION_POINTERS ExceptionInfo);
	LONG NTAPI VexDbgHandler(PEXCEPTION_POINTERS ExceptionInfo);

	void InitLogModule();
	void InstallVehHandler();

	int IsFileExist(const char *name);
	void ThrowIfFailed(HRESULT hr, const char* reason);

	void d3d12_EnableDebugLayer();
	void d3d12_ReportLeaks();

	ComPtr<ID3D12CommandQueue> CreateCommandQueue(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type);
	
	DXGI_FORMAT DXGIFormatFromDX9FMT(D3DFORMAT fmt);
	UINT8 BitsPerPixel(DXGI_FORMAT fmt);

	BOOL GetTrueWindowsVersion(OSVERSIONINFOEX* pOSversion);
	char* GetCPUBrandString();

	char* StrGetCurrentLineStart(char* buffer);
	char* StrNextLine(char* buffer);
	bool StrCutLastElementInPath(char* fn);

	UINT64 GetClosestPow2(UINT64 size);

	UINT64 AlignValueByPow2(UINT64 val, UINT64 pow2val);

	d912pxy_file_path* GetFilePath(d912pxy_file_path_id fpId);	

	void SwitchFilePaths(d912pxy_file_path* newFpArray);

	bool IsKeyDown(int vkcode);

	INT64 SafeDiv(INT64 a, INT64 b);

	wchar_t* strdupw(const wchar_t* s);
}

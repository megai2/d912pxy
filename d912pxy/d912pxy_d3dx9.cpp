/*
MIT License

Copyright(c) 2020 megai2

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

HRESULT d912pxy_d3dx9_DisassembleShader(const DWORD* tokens, ID3DXBuffer** ppDisassembly)
{
	static BOOL firsttime = TRUE;

	/*
	* TODO: Consider using d3dcompile_xx.dll per
	* http://msdn.microsoft.com/en-us/library/windows/desktop/ee663275.aspx
	*/

	static HMODULE hD3DXModule = NULL;
	static PD3DXDISASSEMBLESHADER pfnD3DXDisassembleShader = NULL;

	if (firsttime) {
		if (!hD3DXModule) {
			unsigned release;
			int version;
			for (release = 0; release <= 1; ++release) {
				/* Version 41 corresponds to Mar 2009 version of DirectX Runtime / SDK */
				for (version = 41; version >= 0; --version) {
					char filename[256];
					_snprintf(filename, sizeof(filename),
						"d3dx9%s%s%u.dll", release ? "" : "d", version ? "_" : "", version);
					hD3DXModule = LoadLibraryA(filename);
					if (hD3DXModule)
						goto found;
				}
			}
		found:
			;
		}

		if (hD3DXModule) {
			if (!pfnD3DXDisassembleShader) {
				pfnD3DXDisassembleShader = (PD3DXDISASSEMBLESHADER)GetProcAddress(hD3DXModule, "D3DXDisassembleShader");
			}
		}

		firsttime = FALSE;
	}

	HRESULT hr = E_FAIL;
	if (pfnD3DXDisassembleShader) {
		hr = pfnD3DXDisassembleShader(tokens, FALSE, NULL,
			reinterpret_cast<ID3DXBuffer**>(ppDisassembly));
	}
	return hr;
}

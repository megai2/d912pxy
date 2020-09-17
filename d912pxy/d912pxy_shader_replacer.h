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
#pragma once
#include "stdafx.h"

struct ID3DXBuffer;

typedef struct d912pxy_shader_code {
	void* code;
	size_t sz;

	ComPtr<ID3DBlob> blob;
} d912pxy_shader_code;

class d912pxy_shader_replacer : public d912pxy_noncom
{
public:
	d912pxy_shader_replacer(DWORD* fun, UINT len, d912pxy_shader_uid UID, UINT isVs);
	~d912pxy_shader_replacer();
	
	d912pxy_shader_code CompileFromHLSL(const wchar_t* bfolder, UINT keepSource);
	d912pxy_shader_code CompileFromHLSL_MEM(const wchar_t* bfolder, void* imem, UINT size, UINT saveSource);
	d912pxy_shader_code CompileFromHLSL_CS(const wchar_t* bfolder);
	d912pxy_shader_code LoadFromCSO(const char* bfolder);
	void SaveCSO(d912pxy_shader_code code, const char* bfolder);

	d912pxy_hlsl_generator_memout* GenerateHLSL(const wchar_t * bfolder);

	d912pxy_mem_block GetHLSL();
	d912pxy_shader_code GetCode();
	d912pxy_shader_code GetCodeCS();

	UINT GetMaxVars();

private:
	UINT CheckTypeSignature();


	d912pxy_shader_uid mUID;
	DWORD* oCode;
	UINT oLen;

	UINT vsSig;
};


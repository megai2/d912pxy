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

#define PXY_INNER_MAX_SHADER_LISTING_LEN 1024 * 1024

class d912pxy_shader : public d912pxy_comhandler
{
public:
	d912pxy_shader(d912pxy_device* dev, const wchar_t * shtName, const DWORD* fun);
	~d912pxy_shader();

	/*** IDirect3DVertexShader9 methods ***/
	D912PXY_METHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice);
	D912PXY_METHOD(GetFunction)(THIS_ void* arg, UINT* pSizeOfData);

	D3D12_SHADER_BYTECODE* GetCode();

	DWORD* GetOCode() { return oCode; };
	UINT GetOLen() { return oLen; };

	void SetMaxVars(UINT v);

	UINT GetMaxVars() { return maxVars; };

	d912pxy_shader_uid GetID() { return mUID; };

	void NotePairUsage(UINT32 pairHash);
	d912pxy_ringbuffer<UINT32>* GetPairs() { return pairs; };

	UINT FinalReleaseCB();

private:
	D3D12_SHADER_BYTECODE dx12code;

	d912pxy_ringbuffer<UINT32>* pairs;

	d912pxy_shader_code bytecode;
	d912pxy_shader_uid mUID;

	UINT oLen;
	UINT maxVars;
	DWORD* oCode;
};


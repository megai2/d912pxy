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

#define PXY_INNER_MAX_SHADER_LISTING_LEN (1024 * 1024)

#define PXY_SHADER_TYPE_PS 0
#define PXY_SHADER_TYPE_VS 1

class d912pxy_shader : public d912pxy_vtable, public d912pxy_comhandler
{
public:
	static d912pxy_shader* d912pxy_shader_com(PXY_INSTANCE_PAR UINT isVs, const DWORD* origCode, d912pxy_shader_uid uid);

	d912pxy_shader(const wchar_t * shtName, const DWORD* fun, d912pxy_shader_uid uid, UINT isVS);	
	~d912pxy_shader();
	d912pxy_shader(const d912pxy_shader&) = delete;

	D912PXY_METHOD(GetFunction)(PXY_THIS_ void* arg, UINT* pSizeOfData);

	D912PXY_METHOD_(ULONG, ReleaseWithPairRemoval)(PXY_THIS);

	D912PXY_METHOD_NC_(ULONG, ReleaseWithPairRemoval)(THIS);
	
	D3D12_SHADER_BYTECODE* GetCode();
	d912pxy_mem_block GetHLSLSource();

	d912pxy_shader_uid GetID();

	void NotePairUsage(d912pxy_shader_pair_hash_type pairHash);
	
	UINT FinalReleaseCB();

	void RemovePairs();

private:
	d912pxy_ringbuffer<d912pxy_shader_pair_hash_type>* pairs;

	d912pxy_shader_code bytecode;
	d912pxy_shader_uid mUID;

	UINT shaderType;

	UINT oLen;	
	DWORD* oCode;

};


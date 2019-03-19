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
#include "stdafx.h"

d912pxy_shader::d912pxy_shader(d912pxy_device * dev, const wchar_t * shtName, const DWORD * fun) : d912pxy_comhandler(shtName)
{
	m_dev = dev;
	
	mUID = d912pxy_s(sdb)->GetUID((DWORD*)fun, &oLen);

	oCode = (DWORD*)malloc(oLen*4);
	memcpy(oCode, fun, oLen * 4);

	bytecode.code = 0;
	bytecode.blob = nullptr;

	pairs = new d912pxy_ringbuffer<d912pxy_shader_pair_hash_type>(0x10, 2);
}

d912pxy_shader::d912pxy_shader(d912pxy_device * dev, const wchar_t * shtName, d912pxy_shader_uid uid) : d912pxy_comhandler(shtName)
{
	m_dev = dev;
	mUID = uid;

	oCode = NULL;

	d912pxy_shader_replacer* replacer = new d912pxy_shader_replacer(oCode, oLen, mUID);

	bytecode = replacer->GetCode();

	delete replacer;

	pairs = new d912pxy_ringbuffer<d912pxy_shader_pair_hash_type>(0x10, 2);
}

d912pxy_shader::~d912pxy_shader()
{
	delete pairs;

	if (oCode)
		free(oCode);

	if (bytecode.blob)
		bytecode.blob->Release();

	if ((!bytecode.blob) && (bytecode.code))
		free(bytecode.code);
}

D3D12_SHADER_BYTECODE * d912pxy_shader::GetCode()
{
	if (!bytecode.code)
	{	
		d912pxy_shader_replacer* replacer = new d912pxy_shader_replacer(oCode, oLen, mUID);

		bytecode = replacer->GetCode();

		delete replacer;

		free(oCode);		

		oCode = NULL;
	}

	return (D3D12_SHADER_BYTECODE*)&bytecode;
}

d912pxy_shader_uid d912pxy_shader::GetID()
{
	return mUID;
}

void d912pxy_shader::NotePairUsage(d912pxy_shader_pair_hash_type pairHash)
{
	pairs->WriteElement(pairHash);
}

UINT d912pxy_shader::FinalReleaseCB()
{
	return 1;
}

void d912pxy_shader::RemovePairs()
{
	while (pairs->HaveElements())
	{
		d912pxy_shader_pair_hash_type ha = pairs->GetElement();

		d912pxy_s(sdb)->DeletePair(ha);

		pairs->Next();
	}
}

#define D912PXY_METHOD_IMPL_CN d912pxy_shader

/*** IDirect3DVertexShader9 methods ***/
D912PXY_METHOD_IMPL(GetDevice)(THIS_ IDirect3DDevice9** ppDevice)
{
	*ppDevice = (IDirect3DDevice9*)m_dev;

	return D3D_OK;
}

D912PXY_METHOD_IMPL(GetFunction)(THIS_ void* arg, UINT* pSizeOfData)
{
	//IT WILL NOT WORK!
	//LOG_ERR_THROW(-1, "Get shader function is not meant to work");

	return D3D_OK;
}

D912PXY_METHOD_IMPL_(ULONG, ReleaseWithPairRemoval)(THIS)
{
	ULONG ret = ((d912pxy_comhandler*)this)->Release();

	if (!ret)
	{
		RemovePairs();
	}

	return ret;
}

#undef D912PXY_METHOD_IMPL_CN

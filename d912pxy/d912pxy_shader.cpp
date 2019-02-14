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

	maxVars = 4096;
	
	mUID = d912pxy_s(sdb)->GetUID((DWORD*)fun, &oLen);
	oCode = (DWORD*)malloc(oLen*4);
	memcpy(oCode, fun, oLen * 4);

	bytecode.code = 0;
	bytecode.blob = nullptr;

//	GetCode();

	pairs = new d912pxy_ringbuffer<UINT32>(0x10, 2);
}

d912pxy_shader::~d912pxy_shader()
{
	delete pairs;

	if ((!bytecode.blob) && (bytecode.code))
		free(bytecode.code);
}

D3D12_SHADER_BYTECODE * d912pxy_shader::GetCode()
{
	if (!bytecode.code)
	{
		bytecode = d912pxy_s(sdb)->GetCode(mUID, this);

		free(oCode);

		dx12code.pShaderBytecode = bytecode.code;
		dx12code.BytecodeLength = bytecode.sz;
	}

	return &dx12code;
}

void d912pxy_shader::SetMaxVars(UINT v)
{
	maxVars = v;
}

void d912pxy_shader::NotePairUsage(UINT32 pairHash)
{
	pairs->WriteElement(pairHash);
}

UINT d912pxy_shader::FinalReleaseCB()
{
	d912pxy_s(psoCache)->QueueShaderCleanup(this);
	d912pxy_s(psoCache)->SignalWork();
	return 0;
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

#undef D912PXY_METHOD_IMPL_CN

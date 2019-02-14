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

d912pxy_pshader::d912pxy_pshader(d912pxy_device * dev, const DWORD * fun) : d912pxy_shader(dev, L"pshader", fun)
{

}

d912pxy_pshader::~d912pxy_pshader()
{
}

#define D912PXY_METHOD_IMPL_CN d912pxy_pshader

D912PXY_IUNK_IMPL

/*** IDirect3DVertexShader9 methods ***/
D912PXY_METHOD_IMPL(GetDevice)(THIS_ IDirect3DDevice9** ppDevice)
{
	return d912pxy_shader::GetDevice(ppDevice);
}

D912PXY_METHOD_IMPL(GetFunction)(THIS_ void* arg, UINT* pSizeOfData)
{
	return d912pxy_shader::GetFunction(arg, pSizeOfData);
}

#undef D912PXY_METHOD_IMPL_CN
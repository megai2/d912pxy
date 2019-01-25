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

#ifdef _DEBUG
#define DEBUG_LEAKOBJ
#endif

class d912pxy_noncom
{
public:
	d912pxy_noncom(d912pxy_device* dev, const wchar_t* logModule);
	~d912pxy_noncom();

	void ThrowErrorDbg(HRESULT hr, const char* msg);

	HRESULT WINAPI GetDevice(IDirect3DDevice9** ppDevice);

protected:

	d912pxy_device* m_dev;
	IP7_Trace * m_log;
	IP7_Trace::hModule LGC_DEFAULT;

#ifdef DEBUG_LEAKOBJ
	UINT lkObjTrace;
#endif

};


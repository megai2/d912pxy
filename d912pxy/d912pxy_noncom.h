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

class d912pxy_noncom 
{
public:
	d912pxy_noncom(const wchar_t* logModule);
	d912pxy_noncom();
	~d912pxy_noncom();

	//virtual void Init();

	void UnInit();

	void ThrowErrorDbg(HRESULT hr, const char* msg);

	static HRESULT WINAPI com_GetDevice(d912pxy_com_object* obj, IDirect3DDevice9** ppDevice);

	void NonCom_Init(const wchar_t* logModule);

	void TrackCall(const char* function);

	void ImplStubCall(const char* function, UINT line);

protected:
	d912pxy_log_module LGC_DEFAULT;

#ifdef _DEBUG
	UINT lkObjTrace;
#endif

};


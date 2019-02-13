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

//megai2: why nuke exist? cuz it do his job

#define D912PXY_METHOD(meth) HRESULT WINAPI meth
#define D912PXY_METHOD_(a,b) a WINAPI b

#define D912PXY_METHOD_IMPL_(a,b) a D912PXY_METHOD_IMPL_CN :: b
#define D912PXY_METHOD_IMPL(a) HRESULT D912PXY_METHOD_IMPL_CN :: a

#define D912PXY_IUNK_IMPL \
/*** IUnknown methods ***/ \
D912PXY_METHOD_IMPL(QueryInterface)(THIS_ REFIID riid, void** ppvObj) \
{ \
	return d912pxy_comhandler::QueryInterface(riid, ppvObj); \
} \
 \
D912PXY_METHOD_IMPL_(ULONG, AddRef)(THIS) \
{ \
	return d912pxy_comhandler::AddRef(); \
} \
 \
D912PXY_METHOD_IMPL_(ULONG, Release)(THIS) \
{ \
	return d912pxy_comhandler::Release(); \
} 

class d912pxy_comhandler : public d912pxy_noncom
{
public:
	d912pxy_comhandler(const wchar_t* moduleText);
	d912pxy_comhandler(d912pxy_device* dev, const wchar_t* moduleText);
	virtual ~d912pxy_comhandler();

	HRESULT WINAPI QueryInterface(REFIID riid, void** ppvObj);
	ULONG WINAPI AddRef();
	ULONG WINAPI Release();

	UINT FinalReleaseTest();

	virtual UINT FinalRelease();

	virtual UINT FinalReleaseCB();

	void ThreadRef(INT ic);

	void NoteDeletion(UINT32 time);
	UINT CheckExpired(UINT32 nt);
	virtual UINT32 PooledAction(UINT32 use);

private:
	LONG thrdRefc;
	LONG refc;
	UINT32 timestamp;
};


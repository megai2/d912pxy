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

#define PXY_COM_LOOKUP(var, obj) &(PXY_COM_CAST(d912pxy_com_object, var)->obj)
#define PXY_COM_LOOKUP_(var, obj) PXY_COM_CAST(d912pxy_com_object, var)->obj
//#define PXY_COM_LOOKUP(var, obj) var != 0 ? (&(PXY_COM_CAST(d912pxy_com_object, var)->obj)) : NULL
#define PXY_COM_CAST(type, var) ((type*)var)
#define PXY_COM_CAST_(type, var) ((type*)((void*)((intptr_t)var)))
#define PXY_THIS d912pxy_com_object* obj
#define PXY_THIS_ d912pxy_com_object* obj,

#define D912PXY_METHOD(meth) static HRESULT WINAPI com_##meth
#define D912PXY_METHOD_(a,b) static a WINAPI com_##b

#define D912PXY_METHOD_IMPL_(a,b) a D912PXY_METHOD_IMPL_CN :: com_##b
#define D912PXY_METHOD_IMPL(a) HRESULT D912PXY_METHOD_IMPL_CN :: com_##a

#define D912PXY_METHOD_NC(meth) HRESULT meth
#define D912PXY_METHOD_NC_(a,b) a b

#define D912PXY_METHOD_IMPL_NC_(a,b) a D912PXY_METHOD_IMPL_CN :: b
#define D912PXY_METHOD_IMPL_NC(a) HRESULT D912PXY_METHOD_IMPL_CN :: a

#define D912PXY_IUNK_IMPL 

class d912pxy_vtable {
public:
	virtual void __topmost_fake_vfptr() { ; };
};

class d912pxy_comhandler : public d912pxy_noncom
{
public:
	d912pxy_comhandler(d912pxy_com_obj_typeid tid, const wchar_t* moduleText);
	d912pxy_comhandler();	
	~d912pxy_comhandler();

	void Init(d912pxy_com_obj_typeid tid, const wchar_t* moduleText);

	D912PXY_METHOD(QueryInterface)(PXY_THIS_ REFIID riid, void** ppvObj);
	D912PXY_METHOD_(ULONG, AddRef)(PXY_THIS);
	D912PXY_METHOD_(ULONG, Release)(PXY_THIS);

	HRESULT QueryInterface(REFIID riid, void** ppvObj);
	ULONG AddRef();
	ULONG Release();

	UINT FinalReleaseTest();

	virtual UINT FinalRelease();
	virtual UINT FinalReleaseCB();
	virtual UINT32 PooledAction(UINT32 use);

	void ThreadRef(INT ic);

	void NoteDeletion(UINT32 time);
	UINT CheckExpired(UINT32 nt, UINT32 lifetime);	
	void PooledActionExit();

	int Watching(LONG v);

	LONG GetCOMRefCount() { return refc; };

	LONG GetCurrentPoolSyncValue() { return poolSync.GetValue(); };

	void DeAllocateBase();

	UINT IsPersistentlyPooled() { return persistentlyPooled; };
	void PoolPersistently() { persistentlyPooled = 1; };

protected:
	d912pxy_com_object* comBase;

private:
	std::atomic<LONG> thrdRefc;
	std::atomic<LONG> thrdRefcFlag;
	std::atomic<LONG> beingWatched;
	std::atomic<ULONG> refc;

	UINT32 timestamp;		
	UINT persistentlyPooled;
	d912pxy_com_obj_typeid objType;

	d912pxy_thread_lock poolSync;	
};

class d912pxy_comhandler_non_derived : public d912pxy_vtable, public d912pxy_comhandler
{

};

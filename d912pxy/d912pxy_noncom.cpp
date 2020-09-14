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

#ifdef _DEBUG
std::atomic<LONG> g_ObjectsCounter { 0 };

#include <map>

std::map<UINT, const wchar_t*>*  gLeakTracker;
HANDLE gLeakMapLock;

#endif

d912pxy_noncom::d912pxy_noncom(const wchar_t * logModule)
{
	NonCom_Init(logModule);
}

d912pxy_noncom::d912pxy_noncom()
{
}

d912pxy_noncom::~d912pxy_noncom()
{
#ifdef _DEBUG	
	if (lkObjTrace != 0)
	{
		UnInit();
	}
#endif
}

void d912pxy_noncom::UnInit()
{
#ifdef _DEBUG	
	LOG_DBG_DTDM("Objs last = %u", g_ObjectsCounter.load());

	WaitForSingleObject(gLeakMapLock, INFINITE);
	gLeakTracker->erase(lkObjTrace);
	ReleaseMutex(gLeakMapLock);

	if (lkObjTrace == 1)
	{
		CloseHandle(gLeakMapLock);
		for (std::map<UINT, const wchar_t*>::iterator it = gLeakTracker->begin(); it != gLeakTracker->end(); ++it)
		{
			LOG_DBG_DTDM3("obj %u = %s is leaked", it->first, it->second);
		}
	}

	if (gLeakTracker->empty())
	{
		LOG_DBG_DTDM3("all d912pxy objects are freed. Freedom!");

		delete gLeakTracker;
	}

	lkObjTrace = 0;
#endif
}

void d912pxy_noncom::ThrowErrorDbg(HRESULT hr, const char * msg)
{
	if (!FAILED(hr))
		return;

	LOG_ERR_DTDM("throw on %S with HR = 0x%lX", msg, hr);
			
	d912pxy_helper::ThrowIfFailed(hr, msg);
}

HRESULT d912pxy_noncom::com_GetDevice(d912pxy_com_object* obj, IDirect3DDevice9 ** ppDevice)
{
	*ppDevice = PXY_COM_CAST_(IDirect3DDevice9, &d912pxy_s.dev);

	(*ppDevice)->AddRef();

	return D3D_OK;
}

void d912pxy_noncom::NonCom_Init(const wchar_t * logModule)
{
	d912pxy_s.log.text.RegisterModule(logModule, &LGC_DEFAULT);

	LOG_DBG_DTDM("new %s", logModule);

#ifdef _DEBUG
	LONG ouid = ++g_ObjectsCounter;

	LOG_DBG_DTDM("obj %u is %s", ouid, logModule);

	if (ouid == 1)
	{
		gLeakMapLock = CreateMutex(0, 0, 0);
		gLeakTracker = new std::map<UINT, const wchar_t*>();
		gLeakTracker->clear();
	}
	lkObjTrace = ouid;
	WaitForSingleObject(gLeakMapLock, INFINITE);
	gLeakTracker[0][lkObjTrace] = logModule;
	ReleaseMutex(gLeakMapLock);
#endif
}

void d912pxy_noncom::TrackCall(const char * function)
{
#ifndef DISABLE_P7LIB
	d912pxy_s.log.text.GetP7TrackTrace()->P7_DEBUG(NULL, TM("%016llX %S"), this, function);
#endif
}

void d912pxy_noncom::ImplStubCall(const char * function, UINT line)
{
	LOG_DBG_DTDM("%S %u : stub", function, line);
}

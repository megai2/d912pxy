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
UINT g_ObjectsCounter = 0;

#ifdef DEBUG_LEAKOBJ

#include <map>

std::map<UINT, const wchar_t*>  gLeakTracker;
HANDLE gLeakMapLock;

#endif

#endif

d912pxy_noncom::d912pxy_noncom(d912pxy_device * dev, const wchar_t * logModule)
{
	m_log = d912pxy_helper::GetLogger();

	m_log->Register_Module(logModule, &LGC_DEFAULT);

	LOG_DBG_DTDM("new %s", logModule);

#ifdef _DEBUG
	++g_ObjectsCounter;
	LOG_DBG_DTDM("obj %u is %s", g_ObjectsCounter, logModule);

#ifdef DEBUG_LEAKOBJ
	if (g_ObjectsCounter == 1)
		gLeakMapLock = CreateMutex(0, 0, 0);
	lkObjTrace = g_ObjectsCounter;
	WaitForSingleObject(gLeakMapLock, INFINITE);
	gLeakTracker[lkObjTrace] = logModule;
	ReleaseMutex(gLeakMapLock);
#endif 

#endif

	m_dev = dev;
}

d912pxy_noncom::~d912pxy_noncom()
{
#ifdef _DEBUG	
	--g_ObjectsCounter;

#ifdef DEBUG_LEAKOBJ
	LOG_DBG_DTDM("Objs last = %u", g_ObjectsCounter);

	WaitForSingleObject(gLeakMapLock, INFINITE);
	gLeakTracker.erase(lkObjTrace);
	ReleaseMutex(gLeakMapLock);

	if (lkObjTrace == 1)
	{
		CloseHandle(gLeakMapLock);
		for (std::map<UINT, const wchar_t*>::iterator it = gLeakTracker.begin(); it != gLeakTracker.end(); ++it)
		{
			LOG_DBG_DTDM3("obj %s is leaked", it->second);
		}		
	}

#endif
	if (g_ObjectsCounter == 0)
	{
		LOG_DBG_DTDM3("all d912pxy objects are freed. Freedom!");
	}
#endif
}

void d912pxy_noncom::ThrowErrorDbg(HRESULT hr, const char * msg)
{
	if (!FAILED(hr))
		return;

	m_log->P7_ERROR(LGC_DEFAULT, TM("throw on %S with HR = 0x%lX"), msg, hr);

	//P7_Exceptional_Flush();
			
	d912pxy_helper::ThrowIfFailed(hr, msg);
}

HRESULT d912pxy_noncom::GetDevice(IDirect3DDevice9 ** ppDevice)
{
	LOG_DBG_DTDM(__FUNCTION__);
	*ppDevice = m_dev;
	return D3D_OK;
}

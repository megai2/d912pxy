/*
MIT License

Copyright(c) 2019 megai2

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

#ifdef DISABLE_P7LIB
	#define d912pxy_log_module wchar_t*

	#define _PXY_LOG_INFO(m, t, ...) WriteLogLine(m, t, L"info",  __VA_ARGS__)
	#define _PXY_LOG_ERROR(m, t, ...) WriteLogLine(m, t, L"error",  __VA_ARGS__)
	#define _PXY_LOG_WARNING(m, t, ...) WriteLogLine(m, t, L"warning",  __VA_ARGS__)
	#define _PXY_LOG_DEBUG(m, t, ...) WriteLogLine(m, t, L"debug",  __VA_ARGS__)

#else 
	#define d912pxy_log_module IP7_Trace::hModule

	#define _PXY_LOG_INFO(m, t, ...) GetP7Trace()->P7_INFO(m, t, __VA_ARGS__)
	#define _PXY_LOG_ERROR(m, t, ...) GetP7Trace()->P7_ERROR(m, t, __VA_ARGS__)
	#define _PXY_LOG_WARNING(m, t, ...) GetP7Trace()->P7_WARNING(m, t, __VA_ARGS__)
	#define _PXY_LOG_DEBUG(m, t, ...) GetP7Trace()->P7_DEBUG(m, t, __VA_ARGS__)

#endif

class d912pxy_log 
{
public:
	d912pxy_log();
	~d912pxy_log();

	void Init();
	void UnInit();

#ifndef DISABLE_P7LIB
	IP7_Trace* GetP7Trace();
	IP7_Trace* GetP7TrackTrace();
#endif

	void RegisterModule(const wchar_t * mdn, d912pxy_log_module* ret);
	void SyncCrashWrite(UINT lock);
	void WriteCrashLogLine(wchar_t* buf);

	void WriteLogLine(d912pxy_log_module module, const wchar_t* fmt, const wchar_t* cat, ...);

	void RegisterThread(const char* name);

private:
#ifndef DISABLE_P7LIB
	IP7_Trace* m_log;
	IP7_Trace* m_trackLog;

	IP7_Client* p7cli;

	wchar_t threadNames[255][10];
	UINT32 threadNameId;
#else
	FILE* logfile;
	d912pxy_thread_lock logLock;
#endif

	FILE* crashLog=nullptr;
	UINT32 crashLogLine;
	d912pxy_thread_lock crashLock;
};


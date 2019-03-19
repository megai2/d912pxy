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

#ifndef DISABLE_P7LIB
	IP7_Trace* GetP7Trace();
#endif
		
	void RegisterModule(const wchar_t * mdn, d912pxy_log_module* ret);
	void SyncCrashWrite(UINT lock);
	void WriteCrashLogLine(wchar_t* buf);

	void WriteLogLine(d912pxy_log_module module, const wchar_t* fmt, const wchar_t* cat, ...);

private:
#ifndef DISABLE_P7LIB
	IP7_Trace* m_log;
	IP7_Client* p7cli;
#else

	FILE* logfile;
	d912pxy_thread_lock logLock;
#endif

	FILE* crashLog;
	UINT32 crashLogLine;
	d912pxy_thread_lock crashLock;
};


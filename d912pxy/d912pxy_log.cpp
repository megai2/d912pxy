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
#include "stdafx.h"

d912pxy_log::d912pxy_log()
{
}


d912pxy_log::~d912pxy_log()
{
	
}

void d912pxy_log::Init()
{	
	crashLog = NULL;
	crashLogLine = 0;

#ifndef DISABLE_P7LIB
	//create P7 client object		
	p7cli = P7_Create_Client(d912pxy_s.config.GetValueRaw(PXY_CFG_LOG_P7CONFIG));

	if (!p7cli)
	{
		MessageBox(0, L"P7 init error", L"d912pxy", MB_ICONERROR);
		throw std::exception();
	}

	p7cli->Share(TM("logger"));

	m_log = P7_Create_Trace(p7cli, TM("d912pxy"));
	m_log->Register_Thread(TM("main thread"), 0);

	m_trackLog = P7_Create_Trace(p7cli, TM("d912pxy api tracker"));
	m_trackLog->Register_Thread(TM("main thread"), 0);

	threadNameId = 0;
#else	
	if (d912pxy_helper::IsFileExist(d912pxy_helper::GetFilePath(FP_LOG)->s))
		CopyFile(d912pxy_helper::GetFilePath(FP_LOG)->w, d912pxy_helper::GetFilePath(FP_LOG_OLD)->w, 0);

	logfile = fopen(d912pxy_helper::GetFilePath(FP_LOG)->s, "w");

	if (!logfile)
	{
		MessageBox(0, L"Can't create log file! Check your installation folder file premissions!", L"d912pxy", MB_ICONERROR);
		exit(-1);
	}
	else {
		WriteLogLine((wchar_t*)L"log", L"d912pxy dll loaded", L"info");
	}
#endif
}

void d912pxy_log::UnInit()
{
#ifndef DISABLE_P7LIB
	//megai2: wait a bit for final datas to be processed
	Sleep(2000);

	m_log->Release();
	m_trackLog->Release();

	p7cli->Release();
#else
	if (logfile)
		fclose(logfile);
#endif

	if (crashLog)
		fclose(crashLog);
}

#ifndef DISABLE_P7LIB
IP7_Trace * d912pxy_log::GetP7Trace()
{
	return m_log;
}
IP7_Trace * d912pxy_log::GetP7TrackTrace()
{
	return m_trackLog;
}
#endif

void d912pxy_log::RegisterModule(const wchar_t * mdn, d912pxy_log_module * ret)
{
#ifndef DISABLE_P7LIB
	m_log->Register_Module(mdn, ret);
#else
	*ret = (wchar_t*)mdn;
#endif
}

void d912pxy_log::SyncCrashWrite(UINT lock)
{
	if (lock)
		crashLock.Hold();
	else
		crashLock.Release();
}

void d912pxy_log::WriteCrashLogLine(wchar_t * buf)
{
#ifdef DISABLE_P7LIB
	WriteLogLine((wchar_t*)L"note", L"%u | %s", L"note", crashLogLine, buf);
#else
	char fn[255];

	sprintf(fn, "%s.txt", d912pxy_helper::GetFilePath(FP_CRASH_LOG)->s);

	if (crashLog == NULL)
	{
		int i = 0;
		while (d912pxy_helper::IsFileExist(fn))
		{
			++i;
			sprintf(fn, "%s%u.txt", d912pxy_helper::GetFilePath(FP_CRASH_LOG)->s, i);

			if (i >= 10)
				break;
		}

		crashLog = fopen(fn, "w");

		if (!crashLog)
		{
			//megai2: i think this will be enough
			MessageBoxW(0, buf, L"d912pxy crash", MB_ICONERROR);
			TerminateProcess(GetCurrentProcess(), -1);			
		}
	}

	fwprintf(crashLog, L"%u | %s \r\n", crashLogLine, buf);	
	fflush(crashLog);
#endif
	++crashLogLine;
}

void d912pxy_log::WriteLogLine(d912pxy_log_module module, const wchar_t * fmt, const wchar_t * cat, ...)
{
#ifdef DISABLE_P7LIB	
	logLock.Hold();

	fwprintf(logfile, L"%lX [ %s ] %s | ", GetTickCount(), cat, module);

	va_list arg;
	va_start(arg, cat);
	vfwprintf(logfile, fmt, arg);
	va_end(arg);

	fwprintf(logfile, L" \r\n");
	
	fflush(logfile);

	logLock.Release();
#endif
}

void d912pxy_log::RegisterThread(const char * name)
{	
#ifndef DISABLE_P7LIB
	wsprintf(threadNames[threadNameId], L"%S", name);
	m_log->Register_Thread(threadNames[threadNameId], GetCurrentThreadId());
	++threadNameId;
#endif
}

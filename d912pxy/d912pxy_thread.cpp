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

const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(DWORD dwThreadID, char* threadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

d912pxy_thread::d912pxy_thread()
{

}


d912pxy_thread::~d912pxy_thread()
{	
}

void d912pxy_thread::InitThread(const char * threadName, UINT suspend)
{
	isRunning = 1;
	workEvent = CreateEvent(NULL, 0, 0, NULL);
	workCompleteEvent = CreateEvent(NULL, 0, 0, NULL);

	thrdHandle = CreateThread(
		NULL,
		0,
		d912pxy_thread_main,
		this,
		CREATE_SUSPENDED * suspend,
		0
	);

	SetThreadName(GetThreadId(thrdHandle), (char*)threadName);

	PXY_MALLOC(name, strlen(threadName) + 1, char*);
	strcpy(name, threadName);
}

void d912pxy_thread::Stop()
{	
	isRunning = 0;
	SignalWork();
	WaitForSingleObject(thrdHandle, INFINITE);
	CloseHandle(thrdHandle);
	CloseHandle(workEvent);	
	CloseHandle(workCompleteEvent);
	PXY_FREE(name);
}

void d912pxy_thread::ThreadProc()
{
	d912pxy_s.log.text.RegisterThread(name);
	ThreadInitProc();

	WaitForJob();
	while (isRunning)
	{		
		ThreadJob();				
		WaitForJob();
	}
}

void d912pxy_thread::ThreadJob()
{
	return;//do nothing 
}

void d912pxy_thread::SignalWork()
{	
	if (!workEventSync.TryHold())
	{
		if (!workEventSync.GetValue())			
			SetEvent(workEvent);
	}
	else {
		workEventSync.SetValue(1);
		workEventSync.Release();
	}
}

void d912pxy_thread::WaitForJob()
{
	workEventSync.Hold();

	if (!workEventSync.GetValue())
	{
		WaitForSingleObject(workEvent, INFINITE);
	}

	workEventSync.SetValue(0);

	workEventSync.Release();
}

void d912pxy_thread::IgnoreJob()
{
	ResetEvent(workEvent);
	workEventSync.SetValue(0);
}

void d912pxy_thread::SignalWorkCompleted()
{
	if (workIssued.TryHold())
	{
		workIssued.Add(-1);
		workIssued.Release();
	}
	else {	
		SetEvent(workCompleteEvent);		
	}
}

void d912pxy_thread::IssueWork()
{
	workIssued.Add(1);
	SignalWork();
}

UINT d912pxy_thread::IsWorkCompleted()
{
	return workIssued.GetValue();
}

void d912pxy_thread::WaitForIssuedWorkCompletion()
{
	workIssued.Wait(0);	
}

UINT d912pxy_thread::WaitForIssuedWorkCompletionTimeout(DWORD timeout)
{
	if (workIssued.SpinOnce(0))
		return 1;

	workIssued.Hold();

	if (workIssued.GetValue())
	{		
		if (WaitForSingleObject(workCompleteEvent, timeout) != WAIT_OBJECT_0)
		{
			workIssued.Release();
			return 0;
		}		

		workIssued.SetValue(0);
	}

	workIssued.Release();

	return 1;
}

void d912pxy_thread::ThreadInitProc()
{

}

void d912pxy_thread::Resume()
{
	ResumeThread(thrdHandle);
}

void d912pxy_thread::RestartThread()
{
	TerminateThread(thrdHandle, 0);

	thrdHandle = CreateThread(
		NULL,
		0,
		d912pxy_thread_main,
		this,
		0,
		0
	);	
}


DWORD WINAPI d912pxy_thread_main(void * arg)
{
	d912pxy_thread* obj = (d912pxy_thread*)arg;

	obj->ThreadProc();

	return 0;
}

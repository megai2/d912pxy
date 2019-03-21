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

d912pxy_thread::d912pxy_thread(const char* threadName)
{
	isRunning = 1;
	workEvent = CreateEvent(NULL, 0, 0, NULL);
	completionEvent = CreateEvent(NULL, 1, 0, NULL);
	workIssued = 0;

	thrdHandle = CreateThread(
		NULL,
		0,
		d912pxy_thread_main,
		this,
		0,
		0
	);	

	SetThreadName(GetThreadId(thrdHandle), (char*)threadName);
}


d912pxy_thread::~d912pxy_thread()
{
}

void d912pxy_thread::Stop()
{	
	isRunning = 0;
	SignalWork();
	WaitForSingleObject(thrdHandle, INFINITE);
	CloseHandle(thrdHandle);
	CloseHandle(workEvent);
	CloseHandle(completionEvent);
}

void d912pxy_thread::ThreadProc()
{
	ThreadInitProc();
	WaitForSingleObject(workEvent, INFINITE);
	while (isRunning)
	{		
		ThreadJob();		
		WaitForSingleObject(workEvent, INFINITE);
	}
}

void d912pxy_thread::ThreadJob()
{
	return;//do nothing 
}

void d912pxy_thread::SignalWork()
{
	SetEvent(workEvent);
}

void d912pxy_thread::WaitForJob()
{
	WaitForSingleObject(workEvent, INFINITE);
}

void d912pxy_thread::IgnoreJob()
{
	ResetEvent(workEvent);
}

void d912pxy_thread::SignalWorkCompleted()
{
	if (!InterlockedDecrement(&workIssued))
	{
		SetEvent(completionEvent);
	}
}

void d912pxy_thread::IssueWork()
{
	InterlockedIncrement(&workIssued);
	SignalWork();
}

UINT d912pxy_thread::IsWorkCompleted()
{
	return InterlockedAdd(&workIssued, 0) == 0;
}

void d912pxy_thread::WaitForIssuedWorkCompletion()
{
	if (IsWorkCompleted())
	{
		ResetEvent(completionEvent);
		return;
	}

	WaitForSingleObject(completionEvent, INFINITE);
	ResetEvent(completionEvent);
}

UINT d912pxy_thread::WaitForIssuedWorkCompletionTimeout(DWORD timeout)
{
	if (IsWorkCompleted())
	{
		ResetEvent(completionEvent);
		return 1;
	}

	if (WaitForSingleObject(completionEvent, timeout) != WAIT_TIMEOUT)
	{
		ResetEvent(completionEvent);
		return 1;
	}
	else
		return 0;
}

void d912pxy_thread::ThreadInitProc()
{

}


DWORD WINAPI d912pxy_thread_main(void * arg)
{
	d912pxy_thread* obj = (d912pxy_thread*)arg;

	obj->ThreadProc();

	return 0;
}

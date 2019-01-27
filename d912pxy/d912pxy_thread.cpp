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

d912pxy_thread::d912pxy_thread()
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
}

void d912pxy_thread::ThreadProc()
{
	while (isRunning)
	{
		WaitForSingleObject(workEvent, INFINITE);
		ThreadJob();		
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


DWORD WINAPI d912pxy_thread_main(void * arg)
{
	d912pxy_thread* obj = (d912pxy_thread*)arg;

	obj->ThreadProc();

	return 0;
}

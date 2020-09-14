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

d912pxy_thread_lock::d912pxy_thread_lock()
	: spinLock(0)
{
	Init();
}

d912pxy_thread_lock::~d912pxy_thread_lock()
{
	DeleteCriticalSection(&cs);
}

UINT32 d912pxy_thread_lock::TryHold()
{
	return TryEnterCriticalSection(&cs);
}

void d912pxy_thread_lock::Hold()
{
	EnterCriticalSection(&cs);
}

void d912pxy_thread_lock::Release()
{
	LeaveCriticalSection(&cs);
}

void d912pxy_thread_lock::Init()
{
	InitializeCriticalSection(&cs);
}

LONG d912pxy_thread_lock::LockedAdd(LONG val)
{
	Hold();
	LONG ret = Add(val);
	Release();

	return ret;
}

void d912pxy_thread_lock::LockedSet(LONG val)
{
	Hold();
	SetValue(val);
	Release();
}

void d912pxy_thread_lock::WaitHold(LONG cond)
{
	Wait(cond);
	Hold();
}

void d912pxy_thread_lock::HoldWait(LONG cond)
{
	Hold();
	Wait(cond);
}

LONG d912pxy_thread_lock::SpinOnce(LONG cond)
{
	//megai2: simple spin wait
	UINT spin = 0;

	while (spinLock != cond)
	{
		if (spin > 64)
			return 0;
		else if (spin > 32)
			Sleep(0);
		else if (spin > 12)
			SwitchToThread();
		else
		{
			int loops = (1 << spin); //1..12 ==> 2..4096
			while (loops > 0)
				loops -= 1;
		}

		++spin;
	}

	return 1;
}

LONG d912pxy_thread_lock::Add(LONG val)
{
	spinLock += val;
	return spinLock;
}

void d912pxy_thread_lock::Wait(LONG cond)
{
	//megai2: simple spin wait
	UINT spin = 0;

	while (spinLock != cond)
	{
		if (spin > 32)
			Sleep(0);
		else if (spin > 12)
			SwitchToThread();
		else
		{
			int loops = (1 << spin); //1..12 ==> 2..4096
			while (loops > 0)
				loops -= 1;
		}

		++spin;
	}
}

UINT d912pxy_thread_lock::WaitTimeout(LONG cond, DWORD ms)
{
	DWORD stime = GetTickCount();

	//megai2: simple spin wait
	UINT spin = 0;

	while (spinLock != cond)
	{
		if (spin > 32)
		{
			if ((GetTickCount() - stime) > ms)
				return 0;
			Sleep(0);
		}
		else if (spin > 12)
			SwitchToThread();
		else
		{
			int loops = (1 << spin); //1..12 ==> 2..4096
			while (loops > 0)
				loops -= 1;
		}

		++spin;
	}

	return 1;
}

LONG d912pxy_thread_lock::GetValue()
{
	return spinLock;
}

void d912pxy_thread_lock::SetValue(LONG val)
{
	spinLock = val;
}

void d912pxy_thread_lock::SetValueAsync(LONG val)
{
	//FIXME: obsolete ?
	spinLock = val;
}

void d912pxy_thread_lock::ResetLock()
{
	DeleteCriticalSection(&cs);
	InitializeCriticalSection(&cs);
}

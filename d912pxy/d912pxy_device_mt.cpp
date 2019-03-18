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

void d912pxy_device::LockThread(UINT thread)
{
	LOG_DBG_DTDM("thread %u locked", thread);
	threadLockdEvents[thread].Release();
	
	threadLock.Hold();	
	threadLock.Release();	
		
	threadLockdEvents[thread].Hold();	

	InterlockedDecrement(&threadLockCounter);
}

void d912pxy_device::InitLockThread(UINT thread)
{
	threadLockdEvents[thread].Hold();	
}

void d912pxy_device::LockAsyncThreads()
{
	FRAME_METRIC_SYNC(1)
	
	//megai2: simple spin wait
	UINT spin = 0;

	while (InterlockedAdd(&threadLockCounter, 0) != 0)
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

	threadLock.Hold();

	InterlockedIncrement(&threadInterruptState);

	d912pxy_s(texloadThread)->SignalWork();
	d912pxy_s(bufloadThread)->SignalWork();
	d912pxy_s(CMDReplay)->Finish();

	InterlockedAdd(&threadLockCounter, activeThreadCount);
	
	for (int i = 0; i != PXY_INNER_THREADID_MAX; ++i)
	{
		threadLockdEvents[i].Hold();
	}

	threadLock.Release();
	
	FRAME_METRIC_SYNC(0)
}

void d912pxy_device::UnLockAsyncThreads()
{
	InterlockedDecrement(&threadInterruptState);
	for (int i = 0; i != PXY_INNER_THREADID_MAX; ++i)
	{
		threadLockdEvents[i].Release();
	}	
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
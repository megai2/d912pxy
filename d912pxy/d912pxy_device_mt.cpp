#include "stdafx.h"

void d912pxy_device::LockThread(UINT thread)
{
	LOG_DBG_DTDM("thread %u locked", thread);
	LeaveCriticalSection(&threadLockdEvents[thread]);

	EnterCriticalSection(&threadLock);
	LeaveCriticalSection(&threadLock);

	EnterCriticalSection(&threadLockdEvents[thread]);
}

void d912pxy_device::InitLockThread(UINT thread)
{
	EnterCriticalSection(&threadLockdEvents[thread]);
}

void d912pxy_device::LockAsyncThreads()
{
	FRAME_METRIC_SYNC(1)

	EnterCriticalSection(&threadLock);

	InterlockedIncrement(&threadInterruptState);

	d912pxy_s(texloadThread)->SignalWork();
	d912pxy_s(bufloadThread)->SignalWork();
	d912pxy_s(CMDReplay)->Finish();
	//iframe->PSO()->SignalWork();

	for (int i = 0; i != PXY_INNER_THREADID_MAX; ++i)
	{
		EnterCriticalSection(&threadLockdEvents[i]);			
	}
	
	FRAME_METRIC_SYNC(0)
}

void d912pxy_device::UnLockAsyncThreads()
{
	for (int i = 0; i != PXY_INNER_THREADID_MAX; ++i)
	{
		LeaveCriticalSection(&threadLockdEvents[i]);
	}

 	InterlockedDecrement(&threadInterruptState);
	LeaveCriticalSection(&threadLock);
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
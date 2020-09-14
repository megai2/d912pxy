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

	FRAME_METRIC_THREAD(0, thread);
	
	threadLock.Hold();	
	threadLock.Release();	
	
	threadLockdEvents[thread].Hold();	
	
	threadLock.Add(-1);	
}

void d912pxy_device::InitLockThread(UINT thread)
{
	threadLockdEvents[thread].Hold();		
}

void d912pxy_device::LockAsyncThreads()
{	
	FRAME_METRIC_SYNC_WAKE(1)

	threadLock.WaitHold(0);

	FRAME_METRIC_SYNC_WAKE(0);

#ifdef ENABLE_METRICS
	for (int i = 0; i != activeThreadCount; ++i)
		FRAME_METRIC_THREAD(1, i);
#endif

	FRAME_METRIC_SYNC(1)

	++threadInterruptState;
	
	d912pxy_s.thread.texld.SignalWork();
	d912pxy_s.thread.bufld.SignalWork();
	d912pxy_s.render.replay.Finish();
	
	threadLock.Add(activeThreadCount);	
	
	for (int i = 0; i != activeThreadCount; ++i)
	{		
		threadLockdEvents[i].Hold();
	}

#ifdef ENABLE_METRICS
	d912pxy_s.log.metrics.TrackUploadMemUsage();
#endif

	threadLock.Release();

	//megai2: sync batch here due to GPUW replay
	d912pxy_s.render.batch.FrameEnd();

	d912pxy_s.render.replay.Start();
	
	FRAME_METRIC_SYNC(0)
}

void d912pxy_device::UnLockAsyncThreads()
{
	--threadInterruptState;
	for (int i = 0; i != activeThreadCount; ++i)
	{
		threadLockdEvents[i].Release();
	}	
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
/*
MIT License

Copyright(c) 2018-2020 megai2

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

d912pxy_cleanup_thread::d912pxy_cleanup_thread() : d912pxy_noncom(), d912pxy_thread()
{

}


d912pxy_cleanup_thread::~d912pxy_cleanup_thread()
{
	
}

void d912pxy_cleanup_thread::Init()
{
	NonCom_Init(L"delayed cleanup thread");

	iterationPeriod = (UINT)d912pxy_s.config.GetValueUI64(PXY_CFG_CLEANUP_PERIOD);
	iterationSubsleep = (UINT)d912pxy_s.config.GetValueUI64(PXY_CFG_CLEANUP_SUBSLEEP);
	lifetime = (UINT)d912pxy_s.config.GetValueUI64(PXY_CFG_POOLING_LIFETIME);
	afterResetMaidPasses = d912pxy_s.config.GetValueUI32(PXY_CFG_CLEANUP_AFTER_RESET_MAID);
	softLimit = d912pxy_s.config.GetValueUI32(PXY_CFG_CLEANUP_SOFT_LIMIT);
	hardLimit = d912pxy_s.config.GetValueUI32(PXY_CFG_CLEANUP_HARD_LIMIT);
	watchCount = 0;
	forcedCleanup.SetValue(0);

	buffer = new d912pxy_linked_list<d912pxy_comhandler*>();
	InitThread("d912pxy pool gc", 0);
	SignalWork();	

}

void d912pxy_cleanup_thread::UnInit()
{
	Stop();
	delete buffer;
	d912pxy_noncom::UnInit();
}

void d912pxy_cleanup_thread::ThreadJob()
{
	UINT32 time = GetTickCount();

	buffer->IterStart();

	bool isForced = forcedCleanup.GetValue() > 0; 
	bool noSubsleep = (afterResetMaidTriggered > 0) | (watchCount > softLimit) | isForced;
	bool ignoreLifetime = (watchCount > hardLimit) | (afterResetMaidTriggered > 0) | isForced;

	if (afterResetMaidTriggered)
		--afterResetMaidTriggered;
	
	while (buffer->Iterating())
	{
		d912pxy_comhandler* obj = buffer->Value();
		
		if (obj->CheckExpired(GetTickCount(), lifetime) || ignoreLifetime)
		{
			if (obj->PooledAction(0) && IsThreadRunning() && !noSubsleep)
				Sleep(iterationSubsleep);

			buffer->IterRemove();
			--watchCount;
			obj->Watching(-1);
			
		}
		else {
			buffer->IterNext();
		}

		if (IsThreadRunning())
		{
			if (!noSubsleep)
				noSubsleep = (afterResetMaidTriggered > 0) | (watchCount > softLimit);

			if (!ignoreLifetime)
				ignoreLifetime = (watchCount > hardLimit) | (afterResetMaidTriggered > 0);

			if (!noSubsleep)
				Sleep(0);
		}
		else
			return;
	}

	//megai2: do external flush on device every wake cycle
	d912pxy_s.dev.ExternalFlush();

	if (isForced)
		forcedCleanup.Add(-1);

	UINT32 etime = GetTickCount();

	if (etime > time)
	{
		if ((etime - time) < iterationPeriod)
		{
			INT32 sleepTime = iterationPeriod - (etime - time);

			//Sleep(sleepTime);

			while ((sleepTime > 0) && (IsThreadRunning()) && !forcedCleanup.GetValue())
			{
				Sleep(1000);
				sleepTime -= 1000;
			}
		}
	}

	SignalWork();
}

void d912pxy_cleanup_thread::Watch(d912pxy_comhandler * obj)
{	
	if (obj->Watching(0) < 2)
	{
		++watchCount;
		buffer->Insert(obj);
		obj->Watching(1);
	}
}

void d912pxy_cleanup_thread::ForceCleanup()
{
	forcedCleanup.Add(1);
	forcedCleanup.Wait(0);
}

void d912pxy_cleanup_thread::OnReset()
{
	afterResetMaidTriggered += afterResetMaidPasses;
}

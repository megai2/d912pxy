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

d912pxy_cleanup_thread::d912pxy_cleanup_thread(d912pxy_device* dev) : d912pxy_noncom( L"delayed cleanup thread"), d912pxy_thread("d912pxy pool gc", 0)
{
	d912pxy_s(thread_cleanup) = this;

	iterationPeriod = (UINT)d912pxy_s(config)->GetValueUI64(PXY_CFG_CLEANUP_PERIOD);
	iterationSubsleep = (UINT)d912pxy_s(config)->GetValueUI64(PXY_CFG_CLEANUP_SUBSLEEP);
	lifetime = (UINT)d912pxy_s(config)->GetValueUI64(PXY_CFG_POOLING_LIFETIME);

	buffer = new d912pxy_linked_list<d912pxy_comhandler*>();
	SignalWork();
}


d912pxy_cleanup_thread::~d912pxy_cleanup_thread()
{
	Stop();
	delete buffer;
}

void d912pxy_cleanup_thread::ThreadJob()
{
	UINT32 time = GetTickCount();

	buffer->IterStart();
	
	while (buffer->Iterating())
	{
		d912pxy_comhandler* obj = buffer->Value();
		
		if (obj->CheckExpired(GetTickCount(), lifetime))
		{
			if (obj->PooledAction(0) && IsThreadRunning())
				Sleep(iterationSubsleep);

			buffer->IterRemove();
			obj->Watching(-1);
			
		}
		else {
			buffer->IterNext();
		}

		if (IsThreadRunning())
			Sleep(0);
		else
			return;
	}

	//megai2: do external flush on device every wake cycle
	d912pxy_s(dev)->ExternalFlush();

	UINT32 etime = GetTickCount();

	if (etime > time)
	{
		if ((etime - time) < iterationPeriod)
		{
			INT32 sleepTime = iterationPeriod - (etime - time);

			//Sleep(sleepTime);

			while ((sleepTime > 0) && (IsThreadRunning()))
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
		buffer->Insert(obj);
		obj->Watching(1);
	}
}
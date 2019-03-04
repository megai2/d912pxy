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

d912pxy_cleanup_thread::d912pxy_cleanup_thread(d912pxy_device* dev) : d912pxy_noncom(dev, L"delayed cleanup thread")
{
	d912pxy_s(thread_cleanup) = this;

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
		
		if (obj->CheckExpired(GetTickCount()))
		{
			if (obj->PooledAction(0))
				Sleep(250);

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

//	m_dev->GetShaderDB()->CleanUnusedPairs();

	UINT32 etime = GetTickCount();

	if (etime > time)
	{
		if ((etime - time) < 10000)
		{
			INT32 sleepTime = 10000 - (etime - time);

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
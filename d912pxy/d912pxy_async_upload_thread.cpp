/*
MIT License

Copyright(c) 2019 megai2

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

template<class QueItemType, class ProcImpl>
d912pxy_async_upload_thread<QueItemType, ProcImpl>::d912pxy_async_upload_thread(d912pxy_device * dev, UINT queueSize, UINT syncId, UINT throttleFactor, const wchar_t* objN, const char* thrdName) : d912pxy_noncom(dev, objN), d912pxy_thread(thrdName)
{
	buffer = new d912pxy_ringbuffer<QueItemType>(queueSize, 2);
	threadSyncId = syncId;

	uploadCount = 0;
	uploadTrigger = throttleFactor;

	finishList = new d912pxy_ringbuffer<void*>(64, 2);

	dev->AddActiveThreads(1);
}

template<class QueItemType, class ProcImpl>
d912pxy_async_upload_thread<QueItemType, ProcImpl>::~d912pxy_async_upload_thread()
{
	delete buffer;
	delete finishList;
}

template<class QueItemType, class ProcImpl>
void d912pxy_async_upload_thread<QueItemType, ProcImpl>::QueueItem(QueItemType it)
{
	writeLock.Hold();
	buffer->WriteElement(it);
	writeLock.Release();

	++uploadCount;

	if ((uploadCount % uploadTrigger) == 0)
		SignalWork();
}

template<class QueItemType, class ProcImpl>
void d912pxy_async_upload_thread<QueItemType, ProcImpl>::ThreadJob()
{
	static_cast<ProcImpl>(this)->ThreadWake();

	while (buffer->HaveElements())
	{
		QueItemType it = buffer->PopElementMTG();

		static_cast<ProcImpl>(this)->UploadItem(&it);		
	}

	CheckInterrupt();
}

template<class QueItemType, class ProcImpl>
void d912pxy_async_upload_thread<QueItemType, ProcImpl>::ThreadInitProc()
{
	d912pxy_s(dev)->InitLockThread(threadSyncId);
}

template<class QueItemType, class ProcImpl>
void d912pxy_async_upload_thread<QueItemType, ProcImpl>::AddToFinishList(void * ptr)
{
	finishList->WriteElement(ptr);
}

template<class QueItemType, class ProcImpl>
void d912pxy_async_upload_thread<QueItemType, ProcImpl>::CheckInterrupt()
{
	if (m_dev->InterruptThreads())
	{
		static_cast<ProcImpl>(this)->OnThreadInterrupt();
		m_dev->LockThread(threadSyncId);
	}
}

template class d912pxy_async_upload_thread<d912pxy_texture_load_item, d912pxy_texture_loader*>;
template class d912pxy_async_upload_thread<d912pxy_vstream_lock_data, d912pxy_buffer_loader*>;

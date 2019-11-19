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
d912pxy_async_upload_thread<QueItemType, ProcImpl>::d912pxy_async_upload_thread() : d912pxy_noncom(), d912pxy_thread()
{

}

template<class QueItemType, class ProcImpl>
d912pxy_async_upload_thread<QueItemType, ProcImpl>::~d912pxy_async_upload_thread()
{

}

template<class QueItemType, class ProcImpl>
void d912pxy_async_upload_thread<QueItemType, ProcImpl>::Init(UINT queueSize, UINT syncId, UINT throttleFactor, const wchar_t* objN, const char* thrdName)
{
	NonCom_Init(objN);
	InitThread(thrdName, 1);

	ulMem = NULL;
	buffer = new d912pxy_ringbuffer<QueItemType>(queueSize, 2);
	threadSyncId = syncId;

	uploadCount = 0;
	uploadTrigger = throttleFactor;

	finishList = new d912pxy_ringbuffer<void*>(64, 2);

	d912pxy_s.dev.AddActiveThreads(1);
}

template<class QueItemType, class ProcImpl>
void d912pxy_async_upload_thread<QueItemType, ProcImpl>::UnInit()
{
	Stop();
	delete buffer;
	delete finishList;

	d912pxy_noncom::UnInit();
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
	d912pxy_s.dev.InitLockThread(threadSyncId);
	static_cast<ProcImpl>(this)->ThreadWake();
}

template<class QueItemType, class ProcImpl>
void d912pxy_async_upload_thread<QueItemType, ProcImpl>::AddToFinishList(void * ptr)
{
	finishList->WriteElement(ptr);
}

template<class QueItemType, class ProcImpl>
UINT32 d912pxy_async_upload_thread<QueItemType, ProcImpl>::ItemsOnQueue()
{
	return buffer->TotalElements();
}

template<class QueItemType, class ProcImpl>
d912pxy_upload_item * d912pxy_async_upload_thread<QueItemType, ProcImpl>::GetUploadMem(UINT32 size)
{
	if (!ulMem)
	{
		ulMem = d912pxy_s.pool.upload.GetUploadObject(size * 2);
#ifdef ENABLE_METRICS
		ulMemFootprintAligned += ulMem->GetSize();
#endif
	} else if (!ulMem->HaveFreeSpace(size))
	{
		ulMem->Release();
		ulMem = d912pxy_s.pool.upload.GetUploadObject((UINT)max(size, ulMem->GetSize() * 2));

#ifdef ENABLE_METRICS		
		ulMemFootprintAligned += ulMem->GetSize();
#endif
	}

#ifdef ENABLE_METRICS
	ulMemFootprint += size;	
#endif

	return ulMem;
}

template<class QueItemType, class ProcImpl>
UINT32 d912pxy_async_upload_thread<QueItemType, ProcImpl>::GetMemFootprintMB()
{
	return (UINT32)(ulMemFootprint >> 20);
}

template<class QueItemType, class ProcImpl>
UINT32 d912pxy_async_upload_thread<QueItemType, ProcImpl>::GetMemFootprintAlignedMB()
{
	return (UINT32)(ulMemFootprintAligned >> 20);
}

template<class QueItemType, class ProcImpl>
void d912pxy_async_upload_thread<QueItemType, ProcImpl>::CheckInterrupt()
{
	if (d912pxy_s.dev.InterruptThreads())
	{
		static_cast<ProcImpl>(this)->OnThreadInterrupt();

		if (ulMem)
		{
			ulMem->Release();
			ulMem = NULL;
		}

		d912pxy_s.dev.LockThread(threadSyncId);

#ifdef ENABLE_METRICS		
		ulMemFootprint = 0;
		ulMemFootprintAligned = 0;
#endif

		static_cast<ProcImpl>(this)->ThreadWake();
	}
}

template class d912pxy_async_upload_thread<d912pxy_texture_load_item, d912pxy_texture_loader*>;
template class d912pxy_async_upload_thread<d912pxy_vstream_lock_data, d912pxy_buffer_loader*>;

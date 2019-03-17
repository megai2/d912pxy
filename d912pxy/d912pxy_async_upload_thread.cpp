#include "stdafx.h"

template<class QueItemType, class ProcImpl>
d912pxy_async_upload_thread<QueItemType, ProcImpl>::d912pxy_async_upload_thread(d912pxy_device * dev, UINT queueSize, UINT syncId, UINT throttleFactor, const wchar_t* objN, const char* thrdName) : d912pxy_noncom(dev, objN), d912pxy_thread(thrdName)
{
	buffer = new d912pxy_ringbuffer<QueItemType>(queueSize, 2);
	threadSyncId = syncId;

	uploadCount = 0;
	uploadTrigger = throttleFactor;
}

template<class QueItemType, class ProcImpl>
d912pxy_async_upload_thread<QueItemType, ProcImpl>::~d912pxy_async_upload_thread()
{
	delete buffer;
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

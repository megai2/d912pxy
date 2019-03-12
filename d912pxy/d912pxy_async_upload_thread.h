#pragma once
#include "stdafx.h"

template <class QueItemType, class ProcImpl>
class d912pxy_async_upload_thread : public d912pxy_noncom, public d912pxy_thread
{
public:
	d912pxy_async_upload_thread(d912pxy_device* dev, UINT queueSize, UINT syncId, UINT throttleFactor, const wchar_t* objN, const char* thrdName);
	~d912pxy_async_upload_thread();

	void QueueItem(QueItemType* it);

	void ThreadJob();
	void ThreadInitProc();

private:
	void CheckInterrupt();
	
	d912pxy_ringbuffer<QueItemType*>* buffer;

	d912pxy_thread_lock writeLock;
	UINT threadSyncId;

	UINT uploadCount;
	UINT uploadTrigger;
};


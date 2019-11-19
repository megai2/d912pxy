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
#pragma once
#include "stdafx.h"

template <class QueItemType, class ProcImpl>
class d912pxy_async_upload_thread : public d912pxy_noncom, public d912pxy_thread
{
public:
	d912pxy_async_upload_thread();
	~d912pxy_async_upload_thread();

	void Init(UINT queueSize, UINT syncId, UINT throttleFactor, const wchar_t* objN, const char* thrdName);
	void UnInit();

	void QueueItem(QueItemType it);

	void ThreadJob();
	void ThreadInitProc();

	void AddToFinishList(void* ptr);

	UINT32 ItemsOnQueue();

	d912pxy_upload_item* GetUploadMem(UINT32 size);

	UINT32 GetMemFootprintMB();
	UINT32 GetMemFootprintAlignedMB();

protected:
	void CheckInterrupt();
	d912pxy_ringbuffer<void*>* finishList;

	d912pxy_upload_item* ulMem;

private:	
	UINT64 ulMemFootprint;
	UINT64 ulMemFootprintAligned;

	d912pxy_ringbuffer<QueItemType>* buffer;
	
	d912pxy_thread_lock writeLock;
	UINT threadSyncId;

	UINT uploadCount;
	UINT uploadTrigger;	
};


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

d912pxy_texture_loader::d912pxy_texture_loader(d912pxy_device* dev) : d912pxy_noncom(dev, L"texture loader"), d912pxy_thread("d912pxy texld")
{
	poolPtr = 0;

	buffer = new d912pxy_ringbuffer<d912pxy_texture_load_item*>(PXY_INNER_MAX_ASYNC_TEXLOADS, 0);

	d912pxy_s(texloadThread) = this;

	InitializeCriticalSection(&writeLock);
}

d912pxy_texture_loader::~d912pxy_texture_loader()
{
	Stop();
	
	delete buffer;
}

void d912pxy_texture_loader::IssueUpload(d912pxy_surface * surf, void* mem, UINT subRes)
{
	d912pxy_texture_load_item* it = &pool[poolPtr];

	it->ul = mem;
	it->surf = surf;
	it->subRes = subRes;	

	++poolPtr;
	if (poolPtr >= PXY_INNER_MAX_ASYNC_TEXLOADS)
		poolPtr = 0;
	
	//megai2: as ringbufer is not growing, we hit overrun and crash if we overwriting pool data
	surf->ThreadRef(1);

	EnterCriticalSection(&writeLock);
	buffer->WriteElement(it);	
	LeaveCriticalSection(&writeLock);
//	it->surf->DelayedLoad(it->ul, it->subRes);


	SignalWork();
}

void d912pxy_texture_loader::ThreadJob()
{
	while (buffer->HaveElements())
	{
		d912pxy_texture_load_item* it = buffer->GetElement();
		
		it->surf->DelayedLoad(it->ul, it->subRes);

		buffer->Next();
	}

	CheckInterrupt();
}

void d912pxy_texture_loader::ThreadInitProc()
{
	d912pxy_s(dev)->InitLockThread(PXY_INNER_THREADID_TEX_LOADER);
}

void d912pxy_texture_loader::CheckInterrupt()
{
	if (m_dev->InterruptThreads())
	{
		m_dev->LockThread(PXY_INNER_THREADID_TEX_LOADER);
	}
}

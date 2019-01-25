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

d912pxy_surface_pool::d912pxy_surface_pool(d912pxy_device* dev) : d912pxy_pool<d912pxy_surface*>(dev)
{
	d912pxy_s(pool_surface) = this;

	InitializeCriticalSection(&mtMutex);

	table = new d912pxy_memtree2(4, 4096, 2, 0);

	this->rwMutex = (CRITICAL_SECTION*)malloc(sizeof(CRITICAL_SECTION)*1);
	InitializeCriticalSection(&this->rwMutex[0]);
}

d912pxy_surface_pool::~d912pxy_surface_pool()
{
	d912pxy_s(pool_surface) = NULL;

	table->Begin();

	while (!table->IterEnd())
	{
		UINT64 cid = table->CurrentCID();
		if (cid)
		{
			d912pxy_ringbuffer<d912pxy_surface*>* item = (d912pxy_ringbuffer<d912pxy_surface*>*)cid;

			while (item->HaveElements())
			{
				item->GetElement()->Release();
				item->Next();
			}

			delete item;
		}
		table->Next();
	}

	delete table;

	free(this->rwMutex);
}

d912pxy_surface * d912pxy_surface_pool::GetSurface(UINT width, UINT height, D3DFORMAT fmt, UINT levels, UINT arrSz)
{
	UINT uidPrecursor[] = {
		width,
		height,
		(UINT)fmt,
		levels,
		arrSz
	};

	UINT uid = table->memHash32s(uidPrecursor, 5 * 4);

	d912pxy_surface* ret = NULL;
	
	PoolRW(uid, &ret, 0);

	if (!ret)
	{
		ret = new d912pxy_surface(m_dev, width, height, fmt, 0, &levels, arrSz);
		ret->MarkPooled(uid);
	}
	else {
		ret->PooledAction(1);
	}

	return ret;
}

d912pxy_surface * d912pxy_surface_pool::AllocProc(UINT32 cat)
{
	return nullptr;
}

d912pxy_ringbuffer<d912pxy_surface*>* d912pxy_surface_pool::GetCatBuffer(UINT32 cat)
{
	d912pxy_ringbuffer<d912pxy_surface*>* ret = NULL;

	EnterCriticalSection(&mtMutex);

	table->PointAtNH(&cat);
	ret = (d912pxy_ringbuffer<d912pxy_surface*>*)table->CurrentCID();

	if (!ret)
	{
		ret = new d912pxy_ringbuffer<d912pxy_surface*>(64, 2);
		table->SetValue((UINT64)ret);
	}

	LeaveCriticalSection(&mtMutex);

	return ret;
}

void d912pxy_surface_pool::PoolRW(UINT32 cat, d912pxy_surface ** val, UINT8 rw)
{
	d912pxy_ringbuffer<d912pxy_surface*>* tbl = GetCatBuffer(cat);

	if (rw)
	{
		if (!*val)
		{
			//			EnterCriticalSection(&allocMutex);

			*val = AllocProc(cat);

			//			LeaveCriticalSection(&allocMutex);
		}
		else {

			(*val)->AddRef();

			d912pxy_s(thread_cleanup)->Watch(*val);

			EnterCriticalSection(&rwMutex[0]);

			tbl->WriteElement(*val);

			LeaveCriticalSection(&rwMutex[0]);
		}
	}
	else {
		EnterCriticalSection(&rwMutex[0]);

		if (tbl->HaveElements())
		{
			*val = tbl->GetElement();
			tbl->Next();
		}
		else
			*val = NULL;

		LeaveCriticalSection(&rwMutex[0]);
	}
}

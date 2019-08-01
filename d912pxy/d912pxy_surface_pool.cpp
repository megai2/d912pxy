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
#include "d912pxy_surface_pool.h"

d912pxy_surface_pool::d912pxy_surface_pool() : d912pxy_pool<d912pxy_surface*, d912pxy_surface_pool*>()
{

}

d912pxy_surface_pool::~d912pxy_surface_pool()
{
	

	pRunning = 0;

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

	PXY_FREE(this->rwMutex);
}

void d912pxy_surface_pool::Init(D3D12_HEAP_FLAGS memPoolFlag)
{
#ifdef ENABLE_METRICS
	poolSize = 0;
#endif

	memPoolSize = d912pxy_s.config.GetValueUI32(PXY_CFG_POOLING_SURFACE_ALLOC_STEP);
	memPoolHeapType = D3D12_HEAP_TYPE_DEFAULT;
	memPoolHeapFlags = memPoolFlag;

	d912pxy_pool<d912pxy_surface*, d912pxy_surface_pool*>::Init();

	config = d912pxy_s.config.GetValueXI64(PXY_CFG_POOLING_SURFACE_LIMITS);

	table = new d912pxy_memtree2(4, 4096, 2);

	PXY_MALLOC(this->rwMutex, sizeof(d912pxy_thread_lock) * 1, d912pxy_thread_lock*);

	this->rwMutex[0].Init();
}

d912pxy_surface * d912pxy_surface_pool::GetSurface(UINT width, UINT height, D3DFORMAT fmt, UINT levels, UINT arrSz, UINT Usage, UINT32* srvFeedback)
{
	UINT uidPrecursor[] = {
		width + (height << 16),		
		levels + (arrSz << 8),
		(UINT)fmt,
		Usage
	};

	UINT uid = table->memHash32s(uidPrecursor, 4 * 4);

	d912pxy_surface* ret = NULL;
	
	PoolRW(uid, &ret, 0);

	if (!ret)
	{
		LOG_DBG_DTDM2("surface pool miss: %u %u %u %u %u %u", width, height, fmt, arrSz, levels, Usage);

		ret = d912pxy_surface::d912pxy_surface_com(width, height, fmt, Usage, D3DMULTISAMPLE_NONE, 0, 0, &levels, arrSz, srvFeedback);
		ret->MarkPooled(uid);
	}
	else {
		ret->SetDHeapIDFeedbackPtr(srvFeedback);
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

	mtMutex.Hold();

	table->PointAtMem(&cat, 4);
	ret = (d912pxy_ringbuffer<d912pxy_surface*>*)table->CurrentCID();

	if (!ret)
	{
		ret = new d912pxy_ringbuffer<d912pxy_surface*>(64, 2);
		table->SetValue((UINT64)ret);
	}

	mtMutex.Release();

	return ret;
}

void d912pxy_surface_pool::PoolRW(UINT32 cat, d912pxy_surface ** val, UINT8 rw)
{
	d912pxy_ringbuffer<d912pxy_surface*>* tbl = GetCatBuffer(cat);

	if (rw)
	{
		if (!*val)
		{
			*val = AllocProc(cat);		
		}
		else {

			(*val)->AddRef();

			PoolUnloadProc(*val, tbl);

			rwMutex[0].Hold();

			tbl->WriteElement(*val);

			rwMutex[0].Release();
		}
	}
	else {
		rwMutex[0].Hold();

		if (tbl->HaveElements())
		{
			*val = tbl->GetElement();
			tbl->Next();
		}
		else
			*val = NULL;

		rwMutex[0].Release();
	}
}

void d912pxy_surface_pool::EarlyInitProc()
{
}

void d912pxy_surface_pool::PoolUnloadProc(d912pxy_surface * val, d912pxy_ringbuffer<d912pxy_surface*>* tbl)
{
	if (tbl->TotalElements() > (config & 0xFFFF))
	{
		val->NoteDeletion(GetTickCount());

		if (config & 0x10000)
			val->PooledAction(0);
		else 
			d912pxy_s.thread.cleanup.Watch(val);
	}
}

#ifdef ENABLE_METRICS

void d912pxy_surface_pool::ChangePoolSize(INT dlt)
{
	poolSize += dlt;
}

#endif

ID3D12Resource * d912pxy_surface_pool::GetPlacedSurface(D3D12_RESOURCE_DESC* dsc, D3D12_RESOURCE_STATES initialState)
{
	ID3D12Resource* ret = NULL;

	UINT64 size = d912pxy_s.dx12.dev->GetResourceAllocationInfo(0, 1, dsc).SizeInBytes;

	if (!memPool || (size >= memPoolSize))
	{
	fallback:
		return 0;
	}
	else {
		ret = CreatePlacedResource(size, dsc, initialState);

		if (!ret)
		{
			LOG_ERR_DTDM("CreatePlacedResource failed with po %llX ps %llX", memPoolOffset, memPoolSize);
			goto fallback;
		}

	}

	return ret;
}

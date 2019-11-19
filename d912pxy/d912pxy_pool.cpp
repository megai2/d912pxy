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
#include "d912pxy_pool.h"

template<class ElementType, class ProcImpl>
d912pxy_pool<ElementType, ProcImpl>::d912pxy_pool()
{
}

template<class ElementType, class ProcImpl>
d912pxy_pool<ElementType, ProcImpl>::~d912pxy_pool()
{

}

template<class ElementType, class ProcImpl>
void d912pxy_pool<ElementType, ProcImpl>::Init()
{
	pRunning = 1;

	NonCom_Init(L"obj pool");

	static_cast<ProcImpl>(this)->EarlyInitProc();

	memPoolSize = memPoolSize << 20;//megai2: convert from Mb to bytes
	memPoolLock = new d912pxy_thread_lock();

	memPool = NULL;

	if (memPoolSize)
		CreateMemPool();
}

template<class ElementType, class ProcImpl>
void d912pxy_pool<ElementType, ProcImpl>::UnInit()
{
	if (memPool)
		memPool->Release();

	delete memPoolLock;

	d912pxy_noncom::UnInit();
}

template<class ElementType, class ProcImpl>
void d912pxy_pool<ElementType, ProcImpl>::PoolRW(UINT32 cat, ElementType * val, UINT8 rw)
{
	d912pxy_ringbuffer<ElementType>* tbl = GetCatBuffer(cat);
	
	if (rw)
	{
		if (!*val)
		{
			*val = static_cast<ProcImpl>(this)->AllocProc(cat);
		}
		else {

			(*val)->AddRef();

			PoolUnloadProc(*val, cat);

			rwMutex[cat].Hold();

			tbl->WriteElement(*val);

			rwMutex[cat].Release();
		}
	}
	else {
		rwMutex[cat].Hold();

		if (tbl->HaveElements())
		{
			*val = tbl->GetElement();
			tbl->Next();
		}
		else
			*val = NULL;

		rwMutex[cat].Release();
	}
}

template<class ElementType, class ProcImpl>
d912pxy_ringbuffer<ElementType>* d912pxy_pool<ElementType, ProcImpl>::GetCatBuffer(UINT32 cat)
{
	return NULL;
}

template<class ElementType, class ProcImpl>
void d912pxy_pool<ElementType, ProcImpl>::PoolUnloadProc(ElementType val, UINT32 cat)
{
	val->NoteDeletion(GetTickCount());

	d912pxy_s.thread.cleanup.Watch(val);
}

template<class ElementType, class ProcImpl>
void d912pxy_pool<ElementType, ProcImpl>::WarmUp(UINT cat)
{
	ElementType v = static_cast<ProcImpl>(this)->AllocProc(cat);
	PoolRW(cat, &v, 1);	
	v->Release();
}

template<class ElementType, class ProcImpl>
void d912pxy_pool<ElementType, ProcImpl>::CreateMemPool()
{
	if (memPool)
		memPool->Release();

	memPoolOffset = 0;

	D3D12_HEAP_DESC heapDsc = {
		memPoolSize,
		d912pxy_s.dev.GetResourceHeap(memPoolHeapType),
		0,
		memPoolHeapFlags
	};

	HRESULT hr = d912pxy_s.dx12.dev->CreateHeap(
		&heapDsc,
		IID_PPV_ARGS(&memPool)
	);
	
	if (hr != S_OK)
	{
		LOG_ERR_DTDM("mempool create error = %08X", hr);
		memPool = NULL;
	}
}

template<class ElementType, class ProcImpl>
ID3D12Resource * d912pxy_pool<ElementType, ProcImpl>::CreatePlacedResource(UINT64 size, D3D12_RESOURCE_DESC * rsDesc, D3D12_RESOURCE_STATES initialState)
{
	ID3D12Resource * ret = NULL;

	memPoolLock->Hold();

	if (memPoolOffset + size >= memPoolSize)
	{
		CreateMemPool();

		if (!memPool)
		{
			memPoolLock->Release();
			return NULL;
		}

		memPoolOffset = 0;
	}

	HRESULT cprHR = d912pxy_s.dx12.dev->CreatePlacedResource(
		memPool,
		memPoolOffset,
		rsDesc,
		initialState,
		0,
		IID_PPV_ARGS(&ret)
	);

	if (FAILED(cprHR))
	{
		memPoolLock->Release();
		return NULL;
	}

	memPoolOffset += size;

	memPoolLock->Release();

	return ret;
}

template class d912pxy_pool<d912pxy_vstream*, d912pxy_vstream_pool*>;
template class d912pxy_pool<d912pxy_upload_item*, d912pxy_upload_pool*>;
template class d912pxy_pool<d912pxy_surface*, d912pxy_surface_pool*>;
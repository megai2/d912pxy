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
#pragma once
#include "stdafx.h"

template <class ElementType, class ProcImpl>
class d912pxy_pool_memcat : public d912pxy_pool<ElementType, ProcImpl>
{
public:
  	d912pxy_pool_memcat();
	~d912pxy_pool_memcat();

	void Init(UINT32 iBitIgnore, UINT32 iBitLimit, d912pxy_config_value limitCfg);

	d912pxy_ringbuffer<ElementType>* GetCatBuffer(UINT32 cat);

	void PoolUnloadProc(ElementType val, UINT32 cat);

	UINT MemCatFromSize(UINT sz);
	UINT MemCatToSize(UINT cat);

	UINT IsPoolHaveFreeSpace();
	void AddMemoryToPool(INT sz);
	UINT32 GetMemoryInPoolMb();

private:
	UINT32 bitIgnore;
	UINT32 bitLimit;
	UINT32 bitCnt;

	UINT32 maxMemoryInPool;
	UINT32 memoryInPool;
	UINT32 peristentUsage;	
protected:
	void CreateMemPool();
	ID3D12Resource* CreatePlacedResource(UINT32 size, D3D12_RESOURCE_DESC* rsDesc);

	d912pxy_ringbuffer<ElementType>** memTable;

	ID3D12Heap* memPool;
	UINT64 memPoolOffset;
	UINT64 memPoolSize;
	D3D12_HEAP_TYPE memPoolHeapType;
	d912pxy_thread_lock* memPoolLock;
};

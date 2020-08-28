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

class d912pxy_surface_pool : public d912pxy_pool<d912pxy_surface*, d912pxy_surface_pool*>
{
	struct Uid
	{
		uint16_t w;
		uint16_t h;
		uint8_t levels;
		uint8_t arrSz;
		uint8_t usage;
		uint32_t fmt;
	};

public:
	d912pxy_surface_pool();
	~d912pxy_surface_pool();

	void Init(D3D12_HEAP_FLAGS memPoolFlag);
	void UnInit();

	d912pxy_surface* GetSurface(UINT width, UINT height, D3DFORMAT fmt, UINT levels, UINT arrSz, UINT Usage, UINT32* srvFeedback);

	d912pxy_surface* AllocProc(UINT32 cat);
	d912pxy_ringbuffer<d912pxy_surface*>* GetCatBuffer(UINT32 cat);

	void PoolRW(UINT32 cat, d912pxy_surface** val, UINT8 rw);

	void EarlyInitProc();

	void PoolUnloadProc(d912pxy_surface* val, d912pxy_ringbuffer<d912pxy_surface*>* tbl);

#ifdef ENABLE_METRICS
	UINT64 GetPoolSizeMB() { return poolSize >> 20; };
	void ChangePoolSize(INT dlt);
#endif

	ID3D12Resource* GetPlacedSurface(D3D12_RESOURCE_DESC* dsc, D3D12_RESOURCE_STATES initialState);
	   
private:
	typedef d912pxy::Memtree<Uid, d912pxy_ringbuffer<d912pxy_surface*>*, d912pxy::Hash32> CatTable;
	CatTable table;
	
	bool disableGC;
	UINT persistentItems;

#ifdef ENABLE_METRICS
	UINT64 poolSize;
#endif	
};


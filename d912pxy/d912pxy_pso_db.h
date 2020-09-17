/*
MIT License

Copyright(c) 2018-2020 megai2

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

#pragma pack(push, 1)

#define PXY_PSO_CACHE_KEYFILE_NAME 1

class d912pxy_pso_db : public d912pxy_noncom, public d912pxy_thread
{
public:
	d912pxy_pso_db();
	~d912pxy_pso_db();

	void Init();
	void UnInit();

	d912pxy_pso_item* GetByDescMT(d912pxy_trimmed_pso_desc* desc);

	//compiler thread

	void EnqueueCompile(d912pxy_pso_item* item);
	UINT GetCompileQueueLength();
	void LockCompileQue(UINT lock);
	void ThreadJob();

	//precompile 
	void LoadCachedData();
	bool IsCacheSavingEnabled()
	{
		return saveCache;
	}

private:
	d912pxy_trimmed_pso_desc::IdStorage cacheIndexes;
	uint32_t cacheIncID;
	bool saveCache;

	d912pxy_ringbuffer<d912pxy_pso_item*>* psoCompileQue;
	d912pxy_thread_lock compileQueLock;
	void CheckCompileQueueLock();
	void SaveKeyToCache(d912pxy_trimmed_pso_desc::StorageKey key, d912pxy_trimmed_pso_desc * desc);
};

#pragma pack(pop)
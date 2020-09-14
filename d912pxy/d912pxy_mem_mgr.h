/*
MIT License

Copyright(c) 2019 AlraiLux, megai2

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

Alrai: Let's do this.
*/

#pragma once
#include"stdafx.h"
//#include "d912pxy_noncom.h"

typedef struct d912pxy_dbg_mem_block {
	char* file;
	int line;
	char* function;
	size_t sz;
	UINT32 trashCheck;
	UINT uid;
} d912pxy_dbg_mem_block;

static UINT global_d912pxy_mem_mgr_live = 0;


class d912pxy_mem_mgr : public d912pxy_noncom {

public:

	d912pxy_mem_mgr();
	~d912pxy_mem_mgr();

	void UnInit();

	UINT64 GetPageSize();

	void ReleaseReservedVARange(intptr_t base);
	void CommitVARange(intptr_t base, UINT64 size);
	void DeCommitVARange(intptr_t base, UINT64 size);
	intptr_t ReserveVARangeAligned(UINT64 pow2shift, UINT64 addedSize);

	bool pxy_malloc_dbg(void** cp, size_t sz, const char* file, const int line, const char* function); 
	bool pxy_realloc_dbg(void** cp, size_t sz, const char* file, const int line, const char* function); 
	void pxy_free_dbg(void** cp, const char* file, const int line, const char* function); // Frees and NULLs the given pointer.	

	void* pxy_malloc_retry(size_t sz);// Returns success or fail. Will attempt until max number of retries then fail.
	void* pxy_realloc_retry(void* block, size_t sz);// Returns success or fail. Will attempt until max number of retries then fail.

	void* pxy_malloc(size_t sz); // Returns success or fail.
	void* pxy_realloc(void* block, size_t sz);  // Returns success or fail. Passed pointer shouldn't get changed if realloc failed.
	void pxy_free(void* block);

	static void* pxy_malloc_dbg_uninit(size_t sz, const char* file, const int line, const char* function);

	void LogLeaked();

	void Init();
	void PostInit();
#ifdef _DEBUG
	void StartTrackingBlocks() { allowTrackBlocks = 1; };
	UINT32 GetMemoryUsedMB() { return (UINT32)(memUsed >> 20); };
	UINT32 GetVAMemoryUsedMB() { return (UINT32)(memVAUsed >> 20); };
#else 
	void StartTrackingBlocks() { };

	UINT32 GetMemoryUsedMB() { return 0; };
	UINT32 GetVAMemoryUsedMB() { return 0; };
#endif

private:

	void* inMalloc(size_t sz);
	void* inRealloc(void* block, size_t sz);
	void inFree(void* cp);

	SYSTEM_INFO sysinf;	

#ifdef _DEBUG
	d912pxy_thread_lock blocksAllocated;
	std::map<UINT, d912pxy_dbg_mem_block*> blockList;

	UINT8 recursionCheck;
	UINT8 allowTrackBlocks = 0;

	d912pxy_StackWalker* stkWlk;

	std::atomic<LONG64> memUsed;
	LONG64 memVAUsed;
#endif
};
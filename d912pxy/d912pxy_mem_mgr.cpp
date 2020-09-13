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

Alrai: We're putting all calls to malloc, realloc, etc. here so it's easier to 
change stuff later. Hopefully we'll catch possible nullptr's that can happen when 
system is under stress or whenever alloc calls just decide to fail.
*/

#include "stdafx.h"
#include "d912pxy_stackwalker.h"

static UINT32 recordOprtNewCaller = 0;

HANDLE g_procHeap = GetProcessHeap();

void * operator new(std::size_t n)
{
#ifdef _DEBUG
	void* ret = NULL;

	if (!global_d912pxy_mem_mgr_live) {
		return d912pxy_mem_mgr::pxy_malloc_dbg_uninit(n, __FILE__, __LINE__, __FUNCTION__);
	}
	else {
		if (recordOprtNewCaller)
			d912pxy_s.mem.pxy_malloc_dbg(&ret, n, 0, 0, "operator new");
		else 
			d912pxy_s.mem.pxy_malloc_dbg(&ret, n, "<no data on new caller>", __LINE__, "operator new");
	}

#else
	void* ret;
	PXY_MALLOC(ret, n, void*);
#endif	
	return ret;
}

void operator delete(void * p) 
{
	PXY_FREE(p);
}


d912pxy_mem_mgr::d912pxy_mem_mgr() : d912pxy_noncom() {	
}

d912pxy_mem_mgr::~d912pxy_mem_mgr() {
	
}

void d912pxy_mem_mgr::UnInit()
{
#ifdef _DEBUG
	delete stkWlk;
	LogLeaked();
#endif

	d912pxy_noncom::UnInit();

	/*	free(d912pxy_s(memMgr));
		d912pxy_s(memMgr) = NULL;*/
}

UINT64 d912pxy_mem_mgr::GetPageSize()
{
	return sysinf.dwPageSize;
}

void d912pxy_mem_mgr::ReleaseReservedVARange(intptr_t base)
{
	VirtualFree((void*)base, 0, MEM_RELEASE);
}

void d912pxy_mem_mgr::CommitVARange(intptr_t base, UINT64 size)
{
#ifdef _DEBUG
	memVAUsed += size;
#endif

	if (VirtualAlloc((void*)base, size, MEM_COMMIT, PAGE_READWRITE) != (void*)base)
	{				
		LOG_ERR_THROW2(HRESULT_FROM_WIN32(GetLastError()), "CommitVARange fail");
	}
}

void d912pxy_mem_mgr::DeCommitVARange(intptr_t base, UINT64 size)
{
#ifdef _DEBUG
	memVAUsed -= size;
#endif

	if (!VirtualFree((void*)base, size, MEM_DECOMMIT))
	{
		LOG_ERR_THROW2(HRESULT_FROM_WIN32(GetLastError()), "DeCommitVARange fail");
	}
}

intptr_t d912pxy_mem_mgr::ReserveVARangeAligned(UINT64 pow2shift, UINT64 addedSize)
{
	intptr_t incBase = 1ULL << pow2shift;

	MEMORY_BASIC_INFORMATION memInfo;

	intptr_t ret = incBase;

	while (VirtualQuery((void*)ret, &memInfo, sizeof(MEMORY_BASIC_INFORMATION)))
	{
		if (memInfo.BaseAddress != (void*)ret)
			goto nextBlock;

		if (memInfo.AllocationBase)
			goto nextBlock;

		if (memInfo.State != MEM_FREE)
			goto nextBlock;

		if (memInfo.RegionSize < (size_t)(incBase + addedSize))
			goto nextBlock;

		if (VirtualAlloc((void*)ret, incBase + addedSize, MEM_RESERVE, PAGE_READWRITE) != (void*)ret)
		{
			LOG_ERR_THROW2(HRESULT_FROM_WIN32(GetLastError()), "ReserveVARangeAligned fail");
		}

		return ret;

		nextBlock:
			ret += incBase;
	}

	LOG_ERR_THROW2(-1, "ReserveVARangeAligned no space");

	return 0;
}

void* d912pxy_mem_mgr::inRealloc(void* block, size_t sz) { // Returns pointer or nullptr if failed.
	
	return HeapReAlloc(g_procHeap, 0, block, sz);
	/*
	//make new
	void* tempPoint = inMalloc(sz);
	

	//copy - we need to know the length of the first block...
	MEMORY_BASIC_INFORMATION memInfo;
	VirtualQuery(block, &memInfo, sizeof(memInfo));

	//copy
	CopyMemory(tempPoint, block, memInfo.RegionSize);

	//delete old
	inFree(block);
	
	return tempPoint;*/

}

void* d912pxy_mem_mgr::inMalloc(size_t sz) { // Returns pointer or nullptr if failed.

	return HeapAlloc(g_procHeap, 0, sz);
	//return VirtualAlloc(NULL, sz, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

}

void d912pxy_mem_mgr::inFree(void* block) {

	HeapFree(g_procHeap, 0, block);
	//VirtualFree(block, 0, MEM_RELEASE);
}


bool d912pxy_mem_mgr::pxy_malloc_dbg(void** cp, size_t sz, const char* file, const int line, const char* function) { // Calls pxy_malloc until it gets a success or fails after trying "tries" times.
#ifdef _DEBUG
	/*if (*cp != NULL) { // Were we passed a non null pointer to malloc? Possible memory leak condition.
		LOG_ERR_DTDM("A malloc was called with a possible valid pointer. Size requested: %u. %S %u %S", sz, file, line, function);
		//pxy_free(*cp); // Let's free that current pointer to avoid a memory leak. // Nevermind, we can't assume just because it isn't null that we should free it... but we will log it.
	}*/

	sz += sizeof(d912pxy_dbg_mem_block);

	void* tempPointer = inMalloc(sz);

	if (!tempPointer)
	{
		LOG_ERR_DTDM("malloc_retry(%llu) called @ %S %u %S", sz, file, line, function);

		tempPointer = pxy_malloc_retry(sz);

		if (!tempPointer)
		{
			LOG_ERR_DTDM("A malloc failed. Debug Info: %S %u %S", file, line, function);			
			return false;
		}
	}

	d912pxy_dbg_mem_block* blkdsc = (d912pxy_dbg_mem_block*)tempPointer;

	blkdsc->file = (char*)file;
	blkdsc->function = (char*)function;
	blkdsc->line = line;
	blkdsc->sz = sz;
	blkdsc->trashCheck = 0xAAAAAAAA;

	memUsed += sz;

	*cp = (void*)((intptr_t)tempPointer + sizeof(d912pxy_dbg_mem_block));

	blocksAllocated.Hold();

	if (recursionCheck || !allowTrackBlocks)
	{
		blkdsc->trashCheck = 0xAAAAAAA9;
		blocksAllocated.Release();
		return true;
	}

	recursionCheck = 1;

	if (file == NULL)
	{
		stkWlk->ShowCallstack();				
		blkdsc->file = stkWlk->ReturnCaller();
	}

	blockList[blocksAllocated.Add(1)] = blkdsc;
	blkdsc->uid = blocksAllocated.GetValue();

	recursionCheck = 0;

	blocksAllocated.Release();

#endif
	return true;
}

bool d912pxy_mem_mgr::pxy_realloc_dbg(void** cp, size_t sz, const char* file, const int line, const char* function) { // Calls pxy_realloc until it gets a success or fails after trying "tries" times.
	
#ifdef _DEBUG
	void* origBlk = (void*)((intptr_t)(*cp) - sizeof(d912pxy_dbg_mem_block));

	sz += sizeof(d912pxy_dbg_mem_block);

	void* tempPointer = inRealloc(origBlk, sz);

	if (!tempPointer)
	{
		LOG_ERR_DTDM("realloc_retry(%llu) called @ %S %u %S", sz, file, line, function);

		tempPointer = pxy_realloc_retry(origBlk, sz);

		if (!tempPointer)
		{
			LOG_ERR_DTDM("A realloc failed. Debug Info: %S %u %S", file, line, function);
			return false;
		}
	}

	d912pxy_dbg_mem_block* blkdsc = (d912pxy_dbg_mem_block*)tempPointer;

	memUsed += sz - blkdsc->sz;

	blkdsc->file = (char*)file;
	blkdsc->function = (char*)function;
	blkdsc->line = line;
	blkdsc->sz = sz;	

	*cp = (void*)((intptr_t)tempPointer + sizeof(d912pxy_dbg_mem_block));

	return true;
#endif
}

void d912pxy_mem_mgr::pxy_free_dbg(void** cp, const char* file, const int line, const char* function) { // Free and NULL
#ifdef _DEBUG
	if (*cp == NULL)
	{
		LOG_ERR_DTDM("free(NULL) @ %S %u %S", file, line, function);		
	}

	void* origBlk = (void*)((intptr_t)(*cp) - sizeof(d912pxy_dbg_mem_block));
	d912pxy_dbg_mem_block* blkdsc = (d912pxy_dbg_mem_block*)origBlk;
							  
	if (blkdsc->trashCheck != 0xAAAAAAAA)
	{
		if (blkdsc->trashCheck == 0xAAAAAAA9)
		{
			goto justFree;
		}

		LOG_ERR_DTDM("free trash check failed @ %S %u %S", file, line, function);
		*cp = NULL;
		return;
	}
	else {
		blkdsc->trashCheck = 0;
	}

	blocksAllocated.Hold();
	blockList.erase(blkdsc->uid);	
	blocksAllocated.Release();

	if (blkdsc->line == 0)
		free(blkdsc->file);

justFree:
	memUsed -= (LONG64)blkdsc->sz;

	inFree(origBlk);
	*cp = NULL;	
#endif
}

void * d912pxy_mem_mgr::pxy_malloc_retry(size_t sz)
{
	void* tempPointer;

	for (UINT i = 0; i < PXY_MEM_MGR_TRIES; i++) {

		tempPointer = inMalloc(sz);

		if (tempPointer != NULL) {		
			return tempPointer;
		}

		Sleep(PXY_MEM_MGR_RETRY_WAIT); // Wait a moment
	}

	return NULL;
}

void * d912pxy_mem_mgr::pxy_realloc_retry(void * block, size_t sz)
{
	void* tempPointer;

	for (UINT i = 0; i < PXY_MEM_MGR_TRIES; i++) {

		tempPointer = inRealloc(block, sz);

		if (tempPointer != NULL) {			
			return tempPointer;
		}

		Sleep(PXY_MEM_MGR_RETRY_WAIT); // Wait a moment
	}

	return NULL;
}

void * d912pxy_mem_mgr::pxy_malloc(size_t sz)
{
	void* ret = inMalloc(sz);

	if (!ret)
	{
		LOG_ERR_DTDM("malloc_retry(%llu) called", sz);

		ret = pxy_malloc_retry(sz);

		if (!ret)
			LOG_ERR_THROW2(-1, "out of memory @ malloc");
	}

	return ret;
}

void * d912pxy_mem_mgr::pxy_realloc(void * block, size_t sz)
{
	void* ret = inRealloc(block, sz);

	if (!ret)
	{
		LOG_ERR_DTDM("realloc_retry(%llu) called", sz);

		ret = pxy_realloc_retry(block, sz);

		if (!ret)
			LOG_ERR_THROW2(-1, "out of memory @ realloc");
	}

	return ret;

}

void d912pxy_mem_mgr::pxy_free(void * block)
{
	inFree(block);
}

void * d912pxy_mem_mgr::pxy_malloc_dbg_uninit(size_t sz, const char * file, const int line, const char * function)
{
	sz += sizeof(d912pxy_dbg_mem_block);

	void* tempPointer = HeapAlloc(GetProcessHeap(), 0, sz);

	if (!tempPointer)
	{
		//megai2: that is critical
		MessageBox(0, L"memmgr pre init malloc fail", L"d912pxy", MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), -1);
	}

	d912pxy_dbg_mem_block* blkdsc = (d912pxy_dbg_mem_block*)tempPointer;

	blkdsc->file = (char*)file;
	blkdsc->function = (char*)function;
	blkdsc->line = line;
	blkdsc->sz = 0;
	blkdsc->trashCheck = 0xAAAAAAA9;	

	return (void*)((intptr_t)tempPointer + sizeof(d912pxy_dbg_mem_block));
}

void d912pxy_mem_mgr::LogLeaked()
{
#ifdef _DEBUG
	if (!blockList.empty())
	{
		LOG_ERR_DTDM("found %llu leaked dynamic allocated blocks", blockList.size());

		for (std::map<UINT, d912pxy_dbg_mem_block*>::iterator it = blockList.begin(); it != blockList.end(); ++it)
		{
			LOG_ERR_DTDM("%S %u %S : %u bytes", it->second->file, it->second->line, it->second->function, it->second->sz);
		}

		blockList.clear();
	}
#endif
}

void d912pxy_mem_mgr::Init()
{
	global_d912pxy_mem_mgr_live = 1;

#ifdef _DEBUG
	blocksAllocated.SetValue(0);
	blockList.clear();

	stkWlk = new d912pxy_StackWalker(3, 4);

	recursionCheck = 0;
	memUsed = 0;
	memVAUsed = 0;
#endif
}

void d912pxy_mem_mgr::PostInit()
{
	NonCom_Init(L"memmgr");
	recordOprtNewCaller = d912pxy_s.config.GetValueUI32(PXY_CFG_LOG_DBG_MEM_MGR_SAVE_NEW_CALLER);

	GetSystemInfo(&sysinf);
}

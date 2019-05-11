/*
MIT License

Copyright(c) 2019 AlraiLux

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


class d912pxy_mem_mgr : public d912pxy_noncom {

public:

	d912pxy_mem_mgr(d912pxy_device* dev);
	~d912pxy_mem_mgr();

	bool pxy_malloc_dbg(void** cp, size_t sz, const char* file, const int line, const char* function); 
	bool pxy_realloc_dbg(void** cp, size_t sz, const char* file, const int line, const char* function); 
	void pxy_free_dbg(void** cp, const char* file, const int line, const char* function); // Frees and NULLs the given pointer.	

	void* pxy_malloc_retry(size_t sz);// Returns success or fail. Will attempt until max number of retries then fail.
	void* pxy_realloc_retry(void* block, size_t sz);// Returns success or fail. Will attempt until max number of retries then fail.

	void* pxy_malloc(size_t sz); // Returns success or fail.
	void* pxy_realloc(void* block, size_t sz);  // Returns success or fail. Passed pointer shouldn't get changed if realloc failed.
	void pxy_free(void* block);

private:

	void* inMalloc(size_t sz);
	void* inRealloc(void* block, size_t sz);
	void inFree(void* cp);

	d912pxy_thread_lock blocksAllocated;
	std::map<UINT, d912pxy_dbg_mem_block*> blockList;	
};
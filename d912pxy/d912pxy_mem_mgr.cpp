//#include "d912pxy_mem_mgr.h"
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

Alrai: We're putting all calls to malloc, realloc, etc. here so it's easier to 
change stuff later. Hopefully we'll catch possible nullptr's that can happen when 
system is under stress or whenever alloc calls just decide to fail.
*/

#include "stdafx.h"


d912pxy_mem_mgr::d912pxy_mem_mgr() : d912pxy_noncom(NULL, L"mem_mgr") { 

}

d912pxy_mem_mgr::~d912pxy_mem_mgr() {

}

void* d912pxy_mem_mgr::inRealloc(void* block, size_t sz) { // Returns pointer or nullptr if failed.
	
	return realloc(block, sz);

}

void* d912pxy_mem_mgr::inMalloc(size_t sz) { // Returns pointer or nullptr if failed.

	return malloc(sz);

}

void d912pxy_mem_mgr::inFree(void* block) {

	free(block);
}


bool d912pxy_mem_mgr::pxy_realloc(void** cp, size_t sz) {  // Returns success or fail. cp current pointer shouldn't get changed if realloc failed.
	void* tempPointer = *cp;

	tempPointer = inRealloc(*cp, sz);
	if (tempPointer != NULL) {
		*cp = tempPointer;
		return true;
	}
	else {
		LOG_ERR_DTDM("A realloc failed with nullptr.");
		return false;
	}	

}


bool d912pxy_mem_mgr::pxy_malloc(void** cp, size_t sz) { // Returns success or fail. cp will be set to new pointer if successful. Will only attempt once.
	if (*cp != NULL) { // Were we passed a non null pointer to malloc? Possible memory leak condition.
		LOG_ERR_DTDM("A malloc was called with an already valid pointer. Possible memory leak condition.");
	//	free(*cp); // Let's free that current pointer to avoid a memory leak. // Actually, this breaks the heap in some cases. Leaving it be for now.
	}

	void* tempPointer = inMalloc(sz);

	if (tempPointer != NULL) {
		*cp = tempPointer;
		return true;
	}
	else {
		LOG_ERR_DTDM("A malloc failed with nullptr.");
		return false;
	}

}

bool d912pxy_mem_mgr::pxy_malloc(void** cp, size_t sz, const char* source) { // Returns success or fail. cp will be set to new pointer if successful. Will only attempt once. Debugging.
	if (*cp != NULL) { // Were we passed a non null pointer to malloc? Possible memory leak condition.
		LOG_ERR_DTDM("A malloc was called from %S with an already valid pointer. Possible memory leak condition.", source);
		//	free(*cp); // Let's free that current pointer to avoid a memory leak. // THis currently corrupts the heap. Yay.
	}

	void* tempPointer = inMalloc(sz);

	if (tempPointer != NULL) {
		*cp = tempPointer;
		return true;
	}
	else {
		LOG_ERR_DTDM("A malloc failed with nullptr.");
		return false;
	}

}

bool d912pxy_mem_mgr::pxy_malloc_retry(void** cp, size_t sz, UINT tries) { // Calls pxy_malloc until it gets a success or fails after trying "tries" times.
	
	for (UINT i = 0; i < tries; i++) {
		if (pxy_malloc(cp, sz)) return true;
		Sleep(3); // Wait a moment before trying again- but not too long.

	}
	
	return false;

}

bool d912pxy_mem_mgr::pxy_malloc_retry(void** cp, size_t sz, UINT tries, const char* source) { // Calls pxy_malloc until it gets a success or fails after trying "tries" times.

	for (UINT i = 0; i < tries; i++) {
		if (pxy_malloc(cp, sz, source)) return true;
		Sleep(3); // Wait a moment

	}

	return false;

}

bool d912pxy_mem_mgr::pxy_realloc_retry(void** cp, size_t sz, UINT tries) { // Calls pxy_realloc until it gets a success or fails after trying "tries" times.

	for (UINT i = 0; i < tries; i++) {
		if (pxy_malloc(cp, sz)) return true;
		Sleep(3); // Wait a moment

	}

	return false;

}

void d912pxy_mem_mgr::pxy_free(void* cp) { // Free and NULL

	inFree(cp);
	cp = NULL;
}
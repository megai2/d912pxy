/*
MIT License

Copyright(c) 2019 megai2

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

d912pxy_mem_block d912pxy_mem_block::use(void * ptr, size_t size)
{
	return d912pxy_mem_block(ptr, size);
}

d912pxy_mem_block d912pxy_mem_block::alloc(size_t size)
{
	void* newMem;
	PXY_MALLOC(newMem, size, void*);
	return d912pxy_mem_block(newMem, size);
}

d912pxy_mem_block d912pxy_mem_block::allocZero(size_t size)
{
	auto ret = d912pxy_mem_block::alloc(size);
	ret.FillZero();
	return ret;
}

d912pxy_mem_block d912pxy_mem_block::from(void * ptr, size_t size)
{
	if (!ptr)
		return d912pxy_mem_block(nullptr, 0);

	auto ret = d912pxy_mem_block::alloc(size);

	memcpy(ret.ptr(), ptr, size);

	return ret;
}

d912pxy_mem_block::d912pxy_mem_block(void * ptr, size_t size) :
	iPtr(ptr),
	iSz(size)
{	
}

d912pxy_mem_block::~d912pxy_mem_block()
{
}

void d912pxy_mem_block::Delete()
{
	PXY_FREE(iPtr);
	iPtr = nullptr;
}

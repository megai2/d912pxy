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

template<class ElementType>
d912pxy_pool_memcat<ElementType>::d912pxy_pool_memcat(d912pxy_device * dev, UINT32 iBitIgnore, UINT32 iBitLimit) : d912pxy_pool<ElementType>(dev)
{
	bitIgnore = iBitIgnore;
	bitLimit = iBitLimit;
	bitCnt = iBitLimit - iBitIgnore;

	memTable = (d912pxy_ringbuffer<ElementType>**)malloc(sizeof(void*)*bitCnt);
	this->rwMutex = (CRITICAL_SECTION*)malloc(sizeof(CRITICAL_SECTION)*bitCnt);

	for (int i = 0; i != bitCnt; ++i)
	{
		memTable[i] = new d912pxy_ringbuffer<ElementType>(64, 2);
		InitializeCriticalSection(&this->rwMutex[i]);
	}
}

template<class ElementType>
d912pxy_pool_memcat<ElementType>::~d912pxy_pool_memcat()
{
	free(memTable);
	free(this->rwMutex);
}

template<class ElementType>
d912pxy_ringbuffer<ElementType>* d912pxy_pool_memcat<ElementType>::GetCatBuffer(UINT32 cat)
{
	return memTable[cat];
}

template<class ElementType>
UINT d912pxy_pool_memcat<ElementType>::MemCatFromSize(UINT sz)
{
	UINT64 iv = (sz >> bitIgnore) + 1 * ((sz & ((1 << (bitIgnore + 1)) - 1)) != 0);
	if (!iv)
		return 0;
	UINT ret = 0;
	while (iv > (1ULL << ret))
	{
		++ret;
	}

	return ret;
}

template<class ElementType>
UINT d912pxy_pool_memcat<ElementType>::MemCatToSize(UINT cat)
{
	return 1 << (cat + bitIgnore);
}

template class d912pxy_pool_memcat<d912pxy_vstream*>;
template class d912pxy_pool_memcat<d912pxy_upload_item*>;
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

template<class ElementType, class ProcImpl>
d912pxy_pool_memcat<ElementType, ProcImpl>::d912pxy_pool_memcat(d912pxy_device * dev, UINT32 iBitIgnore, UINT32 iBitLimit, d912pxy_config_value limitCfg, ProcImpl* singleton) : d912pxy_pool<ElementType, ProcImpl>(dev, singleton)
{
	bitIgnore = iBitIgnore;
	bitLimit = iBitLimit;
	bitCnt = iBitLimit - iBitIgnore;
	instantUnload = 0;

	if (!memMgr.pxy_malloc_retry((void**)& limits, sizeof(UINT16)*bitCnt, PXY_MEM_MGR_TRIES, "d912pxy_pool_memcat")) return;
	//limits = (UINT16*)malloc(sizeof(UINT16)*bitCnt);
	ZeroMemory(limits, sizeof(UINT16)*bitCnt);

	UINT performaWarmUp = 0;

	if (limitCfg != PXY_CFG_CNT)
	{
		wchar_t* vals = d912pxy_s(config)->GetValueRaw(limitCfg);

		if ((vals[1] != L'x') || (vals[bitCnt * 5 + 2] != L'L'))
		{
			this->LOG_ERR_THROW2(-1, "Wrong pooling limit config value");
		}
		else {
			wchar_t tmp[5];
			tmp[4] = 0;

			performaWarmUp = vals[bitCnt * 5 + 3] == L'1';
			instantUnload = vals[bitCnt * 5 + 4] == L'1';

			for (int i = 0; i != bitCnt; ++i)
			{
				*((UINT64*)&tmp[0]) = *((UINT64*)&vals[2 + i * 5]);

				swscanf(&tmp[0], L"%hX", &limits[i]);
			}
		}		
	}


	if (!memMgr.pxy_malloc_retry((void**)&memTable, (sizeof(void*)*bitCnt), PXY_MEM_MGR_TRIES, "d912pxy_pool_memcat")) return;
	//memTable = (d912pxy_ringbuffer<ElementType>**)malloc(sizeof(void*)*bitCnt);

	if (!memMgr.pxy_malloc_retry((void**)&this->rwMutex, (sizeof(d912pxy_thread_lock)*bitCnt), PXY_MEM_MGR_TRIES, "d912pxy_pool_memcat")) return;
	//this->rwMutex = (d912pxy_thread_lock*)malloc(sizeof(d912pxy_thread_lock)*bitCnt);

	for (int i = 0; i != bitCnt; ++i)
	{
		memTable[i] = new d912pxy_ringbuffer<ElementType>(64, 2);
		this->rwMutex[i].Init();
	}

	if (performaWarmUp)
	{
		for (int i = 0; i != bitCnt; ++i)
		{		
			for (int j = 0; j != limits[i]; ++j)
				this->WarmUp(i);
		}
	}
}

template<class ElementType, class ProcImpl>
d912pxy_pool_memcat<ElementType, ProcImpl>::~d912pxy_pool_memcat()
{
	free(memTable);
	free(this->rwMutex);
}

template<class ElementType, class ProcImpl>
d912pxy_ringbuffer<ElementType>* d912pxy_pool_memcat<ElementType, ProcImpl>::GetCatBuffer(UINT32 cat)
{
	return memTable[cat];
}

template<class ElementType, class ProcImpl>
void d912pxy_pool_memcat<ElementType, ProcImpl>::PoolUnloadProc(ElementType val, UINT32 cat)
{
	if (GetCatBuffer(cat)->TotalElements() >= limits[cat])
	{
		val->NoteDeletion(GetTickCount());
		if (instantUnload)
			val->PooledAction(0);
		else 
			d912pxy_s(thread_cleanup)->Watch(val);
	}
}

template<class ElementType, class ProcImpl>
UINT d912pxy_pool_memcat<ElementType, ProcImpl>::MemCatFromSize(UINT sz)
{
	UINT64 iv = (sz >> bitIgnore) + 1 * ((sz & ((1 << (bitIgnore)) - 1)) != 0);
	if (!iv)
		return 0;
	UINT ret = 0;
	while (iv > (1ULL << ret))
	{
		++ret;
	}

	return ret;
}

template<class ElementType, class ProcImpl>
UINT d912pxy_pool_memcat<ElementType, ProcImpl>::MemCatToSize(UINT cat)
{
	return 1 << (cat + bitIgnore);
}

template class d912pxy_pool_memcat<d912pxy_vstream*, d912pxy_vstream_pool*>;
template class d912pxy_pool_memcat<d912pxy_upload_item*, d912pxy_upload_pool*>;
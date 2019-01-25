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
d912pxy_pool<ElementType>::d912pxy_pool(d912pxy_device* dev) : d912pxy_noncom(dev, L"object pool")
{
	InitializeCriticalSection(&pooledActionCS);	
	InitializeCriticalSection(&allocMutex);	
}

template<class ElementType>
d912pxy_pool<ElementType>::~d912pxy_pool()
{

}

template<class ElementType>
void d912pxy_pool<ElementType>::PoolRW(UINT32 cat, ElementType * val, UINT8 rw)
{
	d912pxy_ringbuffer<ElementType>* tbl = GetCatBuffer(cat);
	
	if (rw)
	{
		if (!*val)
		{
//			EnterCriticalSection(&allocMutex);

			*val = AllocProc(cat);

//			LeaveCriticalSection(&allocMutex);
		}
		else {

			(*val)->AddRef();

			d912pxy_s(thread_cleanup)->Watch(*val);

			EnterCriticalSection(&rwMutex[cat]);

			tbl->WriteElement(*val);

			LeaveCriticalSection(&rwMutex[cat]);
		}
	}
	else {
		EnterCriticalSection(&rwMutex[cat]);

		if (tbl->HaveElements())
		{
			*val = tbl->GetElement();
			tbl->Next();
		}
		else
			*val = NULL;

		LeaveCriticalSection(&rwMutex[cat]);
	}
}

template<class ElementType>
void d912pxy_pool<ElementType>::PooledActionLock()
{
	EnterCriticalSection(&pooledActionCS);
}

template<class ElementType>
void d912pxy_pool<ElementType>::PooledActionUnLock()
{
	LeaveCriticalSection(&pooledActionCS);
}

template<class ElementType>
d912pxy_ringbuffer<ElementType>* d912pxy_pool<ElementType>::GetCatBuffer(UINT32 cat)
{
	return NULL;
}

template<class ElementType>
ElementType d912pxy_pool<ElementType>::AllocProc(UINT32 cat)
{
	return NULL;
}

template class d912pxy_pool<d912pxy_vstream*>;
template class d912pxy_pool<d912pxy_upload_item*>;
template class d912pxy_pool<d912pxy_surface*>;
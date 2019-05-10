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
d912pxy_linked_list<ElementType>::d912pxy_linked_list()
{	
	nodePool = new d912pxy_ringbuffer<d912pxy_linked_list_element*>(0x40, 2);

	base = 0;
	end = 0;
}

template<class ElementType>
d912pxy_linked_list<ElementType>::~d912pxy_linked_list()
{
	IterStart();

	while (Iterating())
	{
		IterRemove();
	}

	while (nodePool->HaveElements())
	{
		free(nodePool->GetElement());
		nodePool->Next();
	}

	delete nodePool;
}

template<class ElementType>
void d912pxy_linked_list<ElementType>::Insert(ElementType v)
{
	d912pxy_linked_list_element* node = PooledNode();
	node->value = (void*)v;
	node->next = 0;

	lock.Hold();
	if (!base)
		base = node;
	else 		
		end->next = node;

	end = node;
	lock.Release();
}

template<class ElementType>
void d912pxy_linked_list<ElementType>::IterStart()
{
	iter = base;
	iterPrev = base;
}

template<class ElementType>
void d912pxy_linked_list<ElementType>::IterNext()
{
	d912pxy_linked_list_element * prev = iter;
	iter = iter->next;
	iterPrev = prev;			
}

template<class ElementType>
void d912pxy_linked_list<ElementType>::IterRemove()
{
	iterPrev->next = iter->next;	

	lock.Hold();

	if (iter == base)
	{
		base = iter->next;
		iterPrev = base;
	}

	if (iter == end)
		end = iterPrev;

	if (nodePool->HaveFreeSpace())
		nodePool->WriteElement(iter);
	else
		free(iter);

	lock.Release();

	if (iterPrev)
		iter = iterPrev->next;
	else
		iter = 0;
}

template<class ElementType>
UINT d912pxy_linked_list<ElementType>::Iterating()
{
	return iter != 0;
}

template<class ElementType>
ElementType d912pxy_linked_list<ElementType>::Value()
{
	return (ElementType)iter->value;
}

template<class ElementType>
d912pxy_linked_list_element * d912pxy_linked_list<ElementType>::PooledNode()
{
	d912pxy_linked_list_element * ret = 0;
	if (nodePool->HaveElements())
	{
		ret = nodePool->GetElement();
		nodePool->Next();
	}
	else {
		memMgr.pxy_malloc_retry((void**)&ret, sizeof(d912pxy_linked_list_element), PXY_MEM_MGR_TRIES, "d912pxy_linked_list");
		//ret = (d912pxy_linked_list_element*)malloc(sizeof(d912pxy_linked_list_element));
	}

	return ret;
}


template class d912pxy_linked_list<d912pxy_comhandler*>;
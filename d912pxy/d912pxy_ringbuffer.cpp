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
d912pxy_ringbuffer<ElementType>::d912pxy_ringbuffer(UINT iMaxElements, UINT iGrow) : d912pxy_noncom(0, L"ringbuffer")
{
	UINT memSize = sizeof(ElementType)*iMaxElements;
	bufferData = (intptr_t)malloc(memSize);

	maxElements = iMaxElements;
	grow = iGrow;
	writePoint = bufferData;
	readPoint = bufferData;
	bufferEnd = writePoint + memSize;
	writed = 0;
}

template<class ElementType>
d912pxy_ringbuffer<ElementType>::~d912pxy_ringbuffer()
{
	free((void*)bufferData);
}

template<class ElementType>
void d912pxy_ringbuffer<ElementType>::WriteElement(ElementType ele)
{
	if (writed >= maxElements)
	//if ((writePoint <= readPoint) && ((writePoint + (UINT)sizeof(ElementType)) > readPoint))
	{
		if (grow > 1)
		{			
			growthLock.Hold();

			UINT expandElements = maxElements * (grow - 1);		
			UINT oldMemSize = sizeof(ElementType)*maxElements; 
			UINT addMemSize = sizeof(ElementType)*expandElements;			

			LOG_DBG_DTDM("growing ringbuffer by %u elements", expandElements);
			
			writePoint -= bufferData;
			readPoint -= bufferData;

			bufferData = (intptr_t)realloc((void*)bufferData, addMemSize + oldMemSize);

			writePoint += bufferData;
			readPoint += bufferData;
			bufferEnd = bufferData + addMemSize + oldMemSize;

			//it's not end yet, cuz we allocating memory to point of overruning so we must copy memory to be readed into end of new memory block
			memcpy((void*)(readPoint + addMemSize), (void*)(readPoint), oldMemSize - (readPoint - bufferData));
			readPoint += addMemSize;
			maxElements += expandElements;

			growthLock.Release();
		}
		else {
			LOG_ERR_THROW2(-1, "ring buffer overrun");
			return;
		}
	}

	*((ElementType*)writePoint) = ele;
	writePoint += sizeof(ElementType);

	if (writePoint == bufferEnd)
		writePoint = bufferData;

	InterlockedAdd(&writed,1);
}

template<class ElementType>
ElementType d912pxy_ringbuffer<ElementType>::GetElement()
{
	return *((ElementType*)readPoint);
}

template<class ElementType>
ElementType d912pxy_ringbuffer<ElementType>::PopElement()
{
	ElementType ret = GetElement();
	Next();

	return ret;
}

template<class ElementType>
UINT d912pxy_ringbuffer<ElementType>::HaveElements()
{
	return (writed != 0);
}

template<class ElementType>
UINT d912pxy_ringbuffer<ElementType>::HaveFreeSpace()
{
	return (writed < maxElements);
}

template<class ElementType>
void d912pxy_ringbuffer<ElementType>::Next()
{
	if (!writed)
		return;

	InterlockedDecrement(&writed);

	readPoint += sizeof(ElementType);

	if (readPoint >= bufferEnd)
		readPoint = bufferData;
}

template<class ElementType>
ElementType d912pxy_ringbuffer<ElementType>::PopElementMTG()
{
	growthLock.Hold();

	ElementType ret = GetElement();
	Next();

	growthLock.Release();

	return ret;	
}

template class d912pxy_ringbuffer<d912pxy_comhandler*>;
template class d912pxy_ringbuffer<d912pxy_gpu_cmd_list*>;
template class d912pxy_ringbuffer<d912pxy_batch*>;
template class d912pxy_ringbuffer<d912pxy_pso_cache_item*>;
template class d912pxy_ringbuffer<d912pxy_texture_load_item>;
template class d912pxy_ringbuffer<d912pxy_resource*>;
template class d912pxy_ringbuffer<d912pxy_upload_item*>;
template class d912pxy_ringbuffer<d912pxy_vstream*>;
template class d912pxy_ringbuffer<d912pxy_surface*>;
template class d912pxy_ringbuffer<d912pxy_shader*>;
template class d912pxy_ringbuffer<UINT32>;
template class d912pxy_ringbuffer<UINT64>;
template class d912pxy_ringbuffer<d912pxy_linked_list_element*>;
template class d912pxy_ringbuffer<d912pxy_vstream_lock_data>;
template class d912pxy_ringbuffer<void*>;
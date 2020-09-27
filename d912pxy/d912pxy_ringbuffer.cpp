/*
MIT License

Copyright(c) 2018-2020 megai2

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
d912pxy_ringbuffer<ElementType>::d912pxy_ringbuffer(UINT iMaxElements, UINT iGrow) : d912pxy_noncom(L"ringbuffer")
{
	UINT memSize = sizeof(ElementType)*iMaxElements;

	PXY_MALLOC(bufferData, memSize, intptr_t);

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
	PXY_FREE(bufferData);

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

			//bufferData = (intptr_t)realloc((void*)bufferData, addMemSize + oldMemSize);
			PXY_REALLOC(bufferData, addMemSize + oldMemSize, intptr_t);

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

	++writed;
}

template<class ElementType>
void d912pxy_ringbuffer<ElementType>::WriteElementFast(ElementType ele)
{
	*((ElementType*)writePoint) = ele;
	writePoint += sizeof(ElementType);

	if (writePoint == bufferEnd)
		writePoint = bufferData;
}

template<class ElementType>
void d912pxy_ringbuffer<ElementType>::WriteElementMT(ElementType ele)
{
	writeLock.Hold();

	WriteElement(ele);

	writeLock.Release();
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
ElementType d912pxy_ringbuffer<ElementType>::PopElementFast()
{
	ElementType ret = GetElement();

	readPoint += sizeof(ElementType);

	if (readPoint >= bufferEnd)
		readPoint = bufferData;

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

	--writed;

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

template<class ElementType>
void * d912pxy_ringbuffer<ElementType>::GetBufferBase()
{
	return (void*)bufferData;
}

template<class ElementType>
ElementType d912pxy_ringbuffer<ElementType>::GetElementOffset(UINT index)
{
	return *GetElementOffsetPtr(index);
}

template<class ElementType>
ElementType* d912pxy_ringbuffer<ElementType>::GetElementOffsetPtr(UINT index)
{
	intptr_t bufOffsetPtr = readPoint + index * sizeof(ElementType);

	if (bufOffsetPtr >= bufferEnd)
	{
		bufOffsetPtr = (bufOffsetPtr - bufferEnd) % (maxElements * sizeof(ElementType)) + bufferData;
	}

	return ((ElementType*)bufOffsetPtr);
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
template class d912pxy_ringbuffer<d912pxy_vfs_pck_chunk*>;
template class d912pxy_ringbuffer<float>;
template class d912pxy_ringbuffer<d912pxy_pso_item*>;
template class d912pxy_ringbuffer<d912pxy_pso_item**>;
template class d912pxy_ringbuffer<d912pxy_trimmed_pso_desc>;
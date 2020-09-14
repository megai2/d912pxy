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
#pragma once
#include "d912pxy_noncom.h"

template <class ElementType>
class d912pxy_ringbuffer:
	public d912pxy_noncom
{
public:
	d912pxy_ringbuffer(UINT iMaxElements, UINT iGrow);
	~d912pxy_ringbuffer();

	void WriteElement(ElementType ele);
	void WriteElementFast(ElementType ele);
	void WriteElementMT(ElementType ele);
	ElementType GetElement();
	ElementType PopElement();
	ElementType PopElementFast();
	
	UINT HaveElements();
	UINT HaveFreeSpace();
	void Next();
	
	UINT TotalElements() { return writed; };

	ElementType PopElementMTG();

	void* GetBufferBase();
	ElementType GetElementOffset(UINT index);
	ElementType* GetElementOffsetPtr(UINT index);
		
private:
	intptr_t bufferData;
	intptr_t writePoint;
	intptr_t readPoint;
	intptr_t bufferEnd;

	LONG maxElements;
	UINT grow;
	std::atomic<LONG> writed;

	d912pxy_thread_lock growthLock;
	d912pxy_thread_lock writeLock;

};


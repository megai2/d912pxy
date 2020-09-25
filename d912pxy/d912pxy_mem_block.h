/*
MIT License

Copyright(c) 2019-2020 megai2

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
#include "stdafx.h"

class d912pxy_mem_block {
public:	
	d912pxy_mem_block() : iPtr(nullptr), iSz(0) {};
	~d912pxy_mem_block();

	void Delete();

	bool isNullptr() { return iPtr == nullptr; };
	
	template<class T>
	T* c_arr() {
		return (T*)iPtr;
	}

	void FillZero()
	{
		ZeroMemory(iPtr, iSz);
	};

	size_t size()
	{
		return iSz;
	}

	void* ptr()
	{
		return iPtr;
	}

	void* block_end()
	{
		return (void*)((intptr_t)iPtr + iSz);
	}

	template<class T>
	static d912pxy_mem_block use(T* ptr)
	{
		return use((void*)ptr, sizeof(T));
	}

	template<class T>
	static d912pxy_mem_block alloc()
	{
		auto ret = alloc(sizeof(T));

		return ret;
	}

	template<class T>
	static d912pxy_mem_block alloc(T** target)
	{
		auto ret = alloc<T>();

		*target = (T*)ret.ptr();

		return ret;
	}

	template<class T>
	static d912pxy_mem_block allocZero(T** target, UINT elements)
	{
		auto ret = allocZero(sizeof(T) * elements);

		*target = (T*)ret.ptr();

		return ret;
	}

	static d912pxy_mem_block use(void* ptr, size_t size);
	static d912pxy_mem_block alloc(size_t size);
	static d912pxy_mem_block allocZero(size_t size);
	static d912pxy_mem_block from(void* ptr, size_t size);
	static d912pxy_mem_block null() {
		return d912pxy_mem_block(nullptr, 0);
	}

protected:	
	d912pxy_mem_block(void* ptr, size_t size);

	void* iPtr;
	size_t iSz;
};

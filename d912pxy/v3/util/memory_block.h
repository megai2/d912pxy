/*
MIT License

Copyright(c) 2020 megai2

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

namespace d912pxy
{

	class MemoryArea : public BaseObject
	{
	protected:
		void* ptr = nullptr;
		uintptr_t size = 0;
				
	public:
		MemoryArea() = default;
		MemoryArea(void* in_ptr, uintptr_t in_size) : ptr(in_ptr), size(in_size) {};
		uintptr_t getSize() const { return size; }
		
		template<class T>
		T* c_arr() const { return (T*)ptr; }

		void* getPtr() const { return ptr; }

		template<class T>
		T* end() const { return (T*)((void*)((uintptr_t)ptr + size)); }
	};

	class MemoryBlock : public MemoryArea
	{

	public:
		MemoryBlock() = default;
		MemoryBlock(void* in_ptr, uintptr_t in_size) : MemoryArea(in_ptr, in_size) {}
		MemoryBlock(void* in_ptr) : MemoryArea(in_ptr, 0) {}
		MemoryBlock(uintptr_t size);
		~MemoryBlock();

		void alloc(uintptr_t size);
		void realloc(uintptr_t newSize);
		void free();
	};

}



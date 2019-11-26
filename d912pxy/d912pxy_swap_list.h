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
#pragma once

template<class T>
class d912pxy_swap_list {
public:
	d912pxy_swap_list() : index(0) {};
	~d912pxy_swap_list() {};

	T* operator->() {
		return objArr[index];
	}

	void Next()
	{
		index += 1;
		index %= PXY_INNER_GPU_QUEUE_BUFFER_COUNT;
	}

	void Add(T* obj)
	{
		objArr[index++] = obj;
		index %= PXY_INNER_GPU_QUEUE_BUFFER_COUNT;
	}

	void Cleanup() {
		for (int i = 0; i != PXY_INNER_GPU_QUEUE_BUFFER_COUNT; ++i)
			delete objArr[i];
	}

private:
	UINT index;

	T* objArr[PXY_INNER_GPU_QUEUE_BUFFER_COUNT];
};
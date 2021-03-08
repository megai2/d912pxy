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
	namespace Trivial
	{
		template<typename Element, typename IndexType = intptr_t, int initialSize = 256>
		class PushBuffer
		{
			IndexType maxElements;
			IndexType last;
			LinearArray<Element> storage;

		public:
			PushBuffer()
				: maxElements(initialSize)
				, last(0)
			{
				storage.init(initialSize);
				storage.zeroMem(0, initialSize);
			}

			IndexType next()
			{
				++last;
				if (last >= maxElements)
				{
					auto oldMax = maxElements;
					maxElements <<= 1;
					storage.resize(maxElements);
					storage.zeroMem(oldMax, oldMax);
				}
				return last;
			}

			void push(const Element& v) { auto newIdx = next(); storage.get(newIdx) = v; }
			void reset() { last = 0; }
			void clear() { storage.clear(); };

			IndexType headIdx() { return last; }
			Element& head() { return storage.get(last); }
			Element& operator[](IndexType idx) { return storage.get(idx); }
		};

		template<typename Element, typename IndexType = intptr_t, int initialSize = 256>
		class IteratedPushBuffer
		{
			int cur = 1;
		protected:
			PushBuffer<Element, IndexType, initialSize> data;
		public:
			bool empty() { return cur > data.headIdx(); }
			Element& next()	{ return data[cur++]; }
		};

	}
}
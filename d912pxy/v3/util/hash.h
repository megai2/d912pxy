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
	template<typename InnerType, uint64_t base, uint64_t multiplier>
	struct XorHash
	{
		typedef InnerType Data;

		union {
			InnerType value = 0;
			uint8_t mem[sizeof(InnerType)];
		};

		XorHash() = default;

		template<typename T>
		XorHash(const T& obj)
		{
			from(obj);
		}

		template<typename T>
		void from(const T& obj)
		{
			value = base;
			uint8_t* objMem = (uint8_t*)&obj;

			for (uintptr_t i = 0; i < sizeof(T); ++i)
			{
				value ^= *objMem;
				value *= multiplier;
				++objMem;
			}
		}

		template<>
		void from(const MemoryArea& obj)
		{
			value = base;
			uint8_t* objMem = obj.c_arr<uint8_t>();

			for (uintptr_t i = 0; i < obj.getSize(); ++i)
			{
				value ^= *objMem;
				value *= multiplier;
				++objMem;
			}
		}

		uint8_t operator [] (int index) const { return mem[index]; }
		bool operator==(const XorHash& r) const { return value == r.value; }
	};

	typedef XorHash<uint64_t, 0xcbf29ce484222325, 1099511628211> Hash64;
	typedef XorHash<uint32_t, 0x811c9dc5, 16777619> Hash32;

	template<typename InnerType>
	struct RawHash
	{
		typedef InnerType Data;

		union {
			InnerType value = 0;
			uint8_t mem[sizeof(InnerType)];
		};

		RawHash() = default;
		RawHash(const InnerType& obj) { from(obj); }
		void from(const InnerType& obj) { value = obj; }
		uint8_t operator [] (int index) const { return mem[index]; }
		bool operator==(const RawHash& r) const { return value == r.value; }
	};

}



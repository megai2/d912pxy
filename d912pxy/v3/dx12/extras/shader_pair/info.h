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
	namespace extras
	{
		namespace ShaderPair
		{
			enum class DrawType
			{
				simple,
				pass_start,
				pass_end
			};

			struct Info
			{
				wchar_t* name = nullptr;
				DrawType drawType;
				d912pxy_shader_pair_hash_type spair;
			};

			class InfoStorage : public d912pxy_noncom
			{
				Memtree<d912pxy_shader_pair_hash_type, Info, RawHash<d912pxy_shader_pair_hash_type>> storage;

				void readFData(MemoryBlock& mb);

			public:
				InfoStorage();
				~InfoStorage();

				void Init();

				Info& find(d912pxy_shader_pair_hash_type pair);
				d912pxy_shader_pair_hash_type getSpairForMarker(const wchar_t* name);
				void reload();
			};
		}
	}
}


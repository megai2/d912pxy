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

namespace d912pxy {
namespace extras {
namespace IFrameMods {
	
	class StateHolder
	{
		const d912pxy_replay_thread_context& prevState;
		uint32_t restorationMask;

	public:
		enum
		{
			ST_PSO = 0x1,
			ST_VSTREAM0 = 0x2,
			ST_INDEX = 0x4,
			ST_PRIMTOPO = 0x8,
			ST_RTDS = 0x10
		};

		StateHolder(const StateHolder&) = delete;
		StateHolder(const d912pxy_replay_thread_context& rpState, uint32_t statesToBeChanged);
		~StateHolder();	
	};

} //namespace IFrameMods
} //namespace extras
} //namespace d912pxy

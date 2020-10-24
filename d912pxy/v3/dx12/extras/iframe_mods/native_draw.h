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

			struct NativeDrawData
			{
				const d912pxy::MemoryArea& index;
				const d912pxy::MemoryArea& vertex;
				const d912pxy::MemoryArea& cb0;
				uint32_t vstride;
			};

			class NativeDraw
			{
				ID3D12PipelineState* pso;
				d912pxy_vstream* ibuf;
				d912pxy_vstream* vbuf;
				d912pxy_vstream* cbuf;
				uint32_t vstride;
				uint32_t indexCount;

			public:
				NativeDraw(const NativeDraw&) = delete;
				NativeDraw(ID3D12PipelineState* psoObj, const NativeDrawData& data);
				~NativeDraw();

				void draw(const d912pxy_replay_thread_context& rpCtx);
			};

		} //namespace IFrameMods
	} //namespace extras
} //namespace d912pxy

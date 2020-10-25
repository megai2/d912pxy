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

			template<int Length>
			struct JitterSequence
			{
				struct Elem
				{
					float jX;
					float jY;
					float invW;
					float invH;
				};

				static constexpr int length = Length+1;
				static constexpr int elemSize = sizeof(Elem);
				static constexpr int size = length * elemSize;

				Elem sequence[Length];

				void advanceIdx(int& idx) { idx = (idx + 1) % Length; }

				uint64_t cbOffset(int idx) { return elemSize * (idx + 1); }

				Elem haltonSeq(int idx)
				{
					//ref https://github.com/cenit/jburkardt/blob/master/halton/halton.cpp

					int   prime[2]     = { 2   , 3    };
					float prime_inv[2] = { 1.0f, 0.5f };
					int   t[2]         = { idx , idx  };
					float r[2]         = { 0.0f, 0.0f };
					int d = 0;

					while (0 < (t[0] + t[1]))
					{
						for (int j = 0; j < 2; j++)
						{
							d = t[j] % prime[j];
							r[j] = r[j] + d*prime_inv[j];
							prime_inv[j] = prime_inv[j] / prime[j];
							t[j] = t[j] / prime[j];
						}
					}

					return Elem{ r[0] + 0.5f, r[1] + 0.5f, 0, 0 };
				}

				JitterSequence()
				{
					sequence[0] = Elem{ 0, 0, 0, 0 };
					//fill with 2d halton sequence
					for (int i = 1; i < Length; ++i)
						sequence[i] = haltonSeq(i);

				}

				void adjustInverseWH(int w, int h)
				{
					float iwh[2] = { 1.0f / w, 1.0f / h };
					for (int i = 1; i < Length; ++i)
					{
						sequence[i].invW = iwh[0];
						sequence[i].invH = iwh[1];
					}
				}

				void loadTo(d912pxy_vstream* cbuf) 
				{ 
					void* ptr;
					cbuf->Lock(0, 0, &ptr, 0);
					memcpy(ptr, (void*)sequence, size);
					cbuf->Unlock();					
				}
			};
	
			class GenericTAA : public ModHandler
			{				
				d912pxy_surface* prevFrame = nullptr;
				d912pxy_surface* currentFrame = nullptr;

				d912pxy_surface* surfFromTempl(D3DSURFACE_DESC& descTempl);
				void resetAdditionalFrames(d912pxy_surface* from);

				d912pxy_pso_item* taaShader = nullptr;
				d912pxy::extras::IFrameMods::NativeDraw* taaDraw = nullptr;
				PassDetector* uiPass;

				d912pxy_vstream* jitterCBuf = nullptr;
				int jitterCBufRSIdx = 0;
				int jitterIdx = 0;
				JitterSequence<16> jitterSeq;

				void checkNativeDrawLoaded();
				
			public:
				GenericTAA(const wchar_t* preUiLastDraw, const wchar_t* uiFirstDraw, int cbufferRSIdx);

				void setJitter(bool enable, d912pxy_replay_thread_context* ctx);

				void UnInit();
				void RP_PreDraw(d912pxy_replay_item::dt_draw_indexed* rpItem, d912pxy_replay_thread_context* rpContext);
				void IFR_Start();
			};

}
}
}

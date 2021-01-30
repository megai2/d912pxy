/*
MIT License

Copyright(c) 2021 megai2

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

	class PassDetector2 : public ModHandler
	{ 

	public:
		struct PassTarget
		{
			uint32_t width;
			uint32_t height;
			uint16_t frame_order;
			uint8_t rtarget_index;
			uint8_t tied_bits;
			d912pxy_surface* surf;

			bool bbSized;
			int spairTag;
		};

	private:
		uint32_t bb_width;
		uint32_t bb_height;
		uint32_t c_frame_order = 0;
		std::atomic<Trivial::PushBuffer<PassTarget>*> lastPassTargets { nullptr };
		d912pxy_swap_list<Trivial::PushBuffer<PassTarget>> passTargets;

		void recordTarget(d912pxy_surface* surf, uint8_t index, uint8_t bits);

	public:
		PassDetector2();
		~PassDetector2();
	
		void RP_RTDSChange(d912pxy_replay_item::dt_om_render_targets* rpItem, d912pxy_replay_thread_context* rpContext) override;
		void UI_Draw() override;
		void RP_FrameStart() override;		

		void IFR_Start();
		void IFR_End();
	};

}
}
}
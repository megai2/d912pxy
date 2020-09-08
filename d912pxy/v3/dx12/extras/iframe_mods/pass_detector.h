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

	class PassDetector : public ModHandler
	{
		bool inPass;
		bool justEntered;
		bool justExited;
		bool triggerOnSameRT;
		int RTDSmask;

		d912pxy_shader_pair_hash_type prevLastDrawSpair;
		d912pxy_shader_pair_hash_type firstDrawSpair;
		
		enum 
		{
			SURF_RT = 0,
			SURF_DS = 1,
			SURF_LAST = 2,
			SURF_TRACKED = 4
		};

		d912pxy_surface* surfaces[6];		

		void enter(d912pxy_replay_thread_context* rpContext);
		void exit();
		void neutral();

	public:
		PassDetector(const wchar_t* prevLastDrawMarker, const wchar_t* firstDrawMarker, int iRTDSmask, bool iTriggerOnSameTargets);

		d912pxy_surface* getSurf(bool RT=true, bool last=false) { return surfaces[(last ? SURF_LAST : 0) + (RT ? SURF_RT : SURF_DS)]; };

		bool inside() { return inPass; }
		bool exited() { return justExited; }
		bool entered() { return justEntered; }

		void RP_PreDraw(d912pxy_replay_item::dt_draw_indexed* rpItem, d912pxy_replay_thread_context* rpContext);
		void RP_PostDraw(d912pxy_replay_item::dt_draw_indexed* rpItem, d912pxy_replay_thread_context* rpContext);
		void RP_RTDSChange(d912pxy_replay_item::dt_om_render_targets* rpItem, d912pxy_replay_thread_context* rpContext);
	};

}
}
}
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
		enum { 
			PT_BIT_RT = 0x1, 
			PT_BIT_DS = 0x2, 
			PT_BIT_RTW = 0x4, 
			PT_BIT_DSW = 0x8, 
			PT_BIT_BB_SIZED = 0x10, 
			PT_BIT_SAME_SIZE = 0x20,
			PT_MASK_MULTIWRITE = PT_BIT_RTW | PT_BIT_DSW
		};

		struct PassTarget
		{
			uint32_t width;
			uint32_t height;
			uint16_t frame_order;
			uint16_t rtarget_index;
			uint8_t bits;
			uint64_t hash;
			const wchar_t* dbgName;
			uint32_t draws;

			d912pxy_surface* surf;
			int spairTag;
		};

	private:
		uint32_t bb_width = 0;
		uint32_t bb_height = 0;
		uint32_t draws_per_pass;
		bool bbChanged = true;
		int c_frame_order = 0;

		struct UniqueTargetsPB : public Trivial::PushBuffer<d912pxy_surface*, uint16_t>
		{
			void resetWithCleanup()
			{
				if (!headIdx())
					return;

				for (int i = 1; i <= headIdx(); ++i)
					this->operator[](i)->ThreadRef(-1);

				reset();
			}

			void pushWithRef(d912pxy_surface* v)
			{
				v->ThreadRef(1);
				push(v);
			}
		};

		struct PerFrameData
		{
			Trivial::PushBuffer<PassTarget> passTargets;
			UniqueTargetsPB uniqueTargetsArr[2];
			uint64_t hash;

			void reset()
			{
				Hash64::init(hash);
				passTargets.reset();
				uniqueTargetsArr[0].resetWithCleanup();
				uniqueTargetsArr[1].resetWithCleanup();
			}
		};

		std::atomic<PerFrameData*> externFrData { nullptr };
		PerFrameData* lastFrData;
		d912pxy_swap_list<PerFrameData> frData;
		d912pxy_surface* lastTargets[2] = { 0 };

		struct NamedTarget
		{
			bool ds;
			bool converge;
			int index;
			int minimalPassNum;
		};

		struct NamedPass
		{
			uint64_t hash;
			uint64_t name;
			const wchar_t* dbgName;
			Trivial::PushBuffer<NamedTarget> targets;
		};

		Trivial::PushBuffer<NamedPass*> namedPasses;
		uint64_t newPassName = 0;
		uint64_t acctivePassName = 0;
		NamedPass* activePass = nullptr;
		bool newPass = false;

		void recordTarget(d912pxy_surface* surf, uint8_t bits);

	public:
		PassDetector2();
		~PassDetector2();

		int loadLinks(const wchar_t* passStrName);

		int registerName() { return ++newPassName; }
		void linkName(uint64_t name, uint64_t hash, const wchar_t* dbg_name);
		intptr_t linkTarget(uint64_t hash, const NamedTarget& tgt);
		bool inPass(uint64_t name) { return acctivePassName == name; }
		bool passChanged() { return newPass; }

		//TODO: track this too
		bool isConvergingToLastFrame() { return true; }

		//1 frame lag here, as we can't detect what frame we have until it ends
		//should be fine as resources are usually same between frames even if frame have small changes
		uint64_t getLastFrameHash();
		bool copyLastSurfaceNamed(intptr_t idx, d912pxy_surface* target, ID3D12GraphicsCommandList* cl);
		bool copyLastSurface(bool ds, bool converge, int index, int minimalPassNum, d912pxy_surface* target, ID3D12GraphicsCommandList* cl);
		uint64_t getCurrentHash() { return frData->hash; }

		d912pxy_surface* makeBBsizedTarget(D3DFORMAT fmt, const wchar_t* dbgMarker);

		bool isBBChangedThisFrame() { return bbChanged; }

		void RP_RTDSChange(d912pxy_replay_item::dt_om_render_targets* rpItem, d912pxy_replay_thread_context* rpContext) override;
		void UI_Draw() override;
		void RP_FrameStart() override;
		void RP_PostDraw(d912pxy_replay_item::dt_draw_indexed*, d912pxy_replay_thread_context*) override;

		void IFR_Start();
		void IFR_End();
	};

}
}
}
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

			class ReshadeCompat : public ModHandler
			{

			public:
				enum
				{
					TARGET_COLOR = 0,
					TARGET_DEPTH = 1,
					TARGET_OPAQUE = 2,
					TARGET_GBUF0 = 3,
					TARGET_GBUF1 = 4,
					TARGET_EARLY_DEPTH = 5,
					TARGET_BB_SIZED_COUNT = 6,
					TARGET_SHCONSTS0 = 6,
					TARGET_TOTAL_COUNT = 7
				};

			private:
				bool shouldRecordShConsts = false;
				PassDetector2* passes;
				d912pxy_surface* targets[TARGET_BB_SIZED_COUNT] = { 0 };

				uint64_t uiPass;
				uint64_t depthPass;
				uint64_t shadowPass;
				uint64_t normalPass;
				uint64_t resolvePass;
				uint64_t transparentPass;

				union rsad_Data
				{
					float fv4[4];
					uint32_t ui4[4];
				};

				typedef void (*rsad_supplyTexture)(uint32_t idx, ID3D12Resource* res);
				typedef void (*rsad_setData)(uint32_t idx, rsad_Data* value);

				HMODULE reshadeAddonLib;
				rsad_supplyTexture rsadSupplyTexture;
				rsad_setData rsadSetData;
				ShaderConstRecorder* shConsts = nullptr;

			public:
				ReshadeCompat();

				//void Init();
				void UnInit();

				void RP_PSO_Change(d912pxy_replay_item::dt_pso_raw* rpItem, d912pxy_replay_thread_context* rpContext) override;
				//void RP_PreDraw(d912pxy_replay_item::dt_draw_indexed* rpItem, d912pxy_replay_thread_context* rpContext) override;
				void RP_PostDraw(d912pxy_replay_item::dt_draw_indexed* rpItem, d912pxy_replay_thread_context* rpContext) override;
				void RP_RTDSChange(d912pxy_replay_item::dt_om_render_targets* rpItem, d912pxy_replay_thread_context* rpContext) override;
				void RP_FrameStart() override;

				//void IFR_Start() override;
				//void IFR_End() override;
			};

		}
	}
}
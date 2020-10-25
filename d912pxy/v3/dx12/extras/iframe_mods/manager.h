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

			struct ModConfigEntry
			{
				wchar_t* raw = nullptr;

				bool valid() const { return raw != nullptr; }
			};

			class ModHandler : public d912pxy_noncom
			{
			public:
				virtual void Init() {};
				virtual void UnInit() {};

				virtual void RP_PSO_Change(d912pxy_replay_item::dt_pso_raw*, d912pxy_replay_thread_context*) {};
				virtual void RP_PreDraw(d912pxy_replay_item::dt_draw_indexed*, d912pxy_replay_thread_context*) {};
				virtual void RP_PostDraw(d912pxy_replay_item::dt_draw_indexed*, d912pxy_replay_thread_context*) {};
				virtual void RP_RTDSChange(d912pxy_replay_item::dt_om_render_targets*, d912pxy_replay_thread_context*) {};

				virtual void IFR_Start() {};
				virtual void IFR_End() {};
			};

			class Manager : public d912pxy_noncom
			{
				Trivial::PushBuffer<ModHandler*> modList;
				Memtree<MemoryArea, ModConfigEntry, Hash64> configValues;

				void parseData(const MemoryBlock& data);
				void loadConfig();

			public:
				Manager();
				~Manager();

				const ModConfigEntry& configVal(const wchar_t* key);
				void pushMod(ModHandler* mod);
			
				void Init();
				void UnInit();

				void RP_PSO_Change(d912pxy_replay_item::dt_pso_raw* rpItem, d912pxy_replay_thread_context* rpContext);
				void RP_PreDraw(d912pxy_replay_item::dt_draw_indexed* rpItem, d912pxy_replay_thread_context* rpContext);
				void RP_PostDraw(d912pxy_replay_item::dt_draw_indexed* rpItem, d912pxy_replay_thread_context* rpContext);
				void RP_RTDSChange(d912pxy_replay_item::dt_om_render_targets* rpItem, d912pxy_replay_thread_context* rpContext);

				void IFR_Start();
				void IFR_End();
			};

		}
	}
}
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
#include "stdafx.h"

using namespace d912pxy::extras::IFrameMods;

void Manager::parseData(const d912pxy::MemoryBlock& data)
{
	KeyValue::Reader kvReader(data);
	while (!kvReader.empty())
	{
		auto& i = kvReader.next();

		ModConfigEntry newEntry { d912pxy_helper::strdupw(i.value) };
		configValues[MemoryArea((void*)i.key, lstrlenW(i.key))] = newEntry;
	}
}

void Manager::loadConfig()
{
	DirReader dirReader(d912pxy_helper::GetFilePath(FP_IFRAME_MODS_BASE_PATH)->w, d912pxy_s.config.GetValueRaw(PXY_CFG_EXTRAS_IFRAME_MOD_SOURCE));

	while (!dirReader.empty())
		parseData(dirReader.next());

	LOG_INFO_DTDM("Readed %u iframe mod config files", dirReader.readed());
}

Manager::Manager()
{
}

Manager::~Manager()
{
}

const ModConfigEntry& Manager::configVal(const wchar_t* key)
{
	return configValues[MemoryArea((void*)key, lstrlenW(key))];
}

void Manager::pushMod(ModHandler* mod)
{
	modList.push(mod);
}

void Manager::Init()
{
	loadConfig();
	auto& mainMod = configVal(L"primary_mod");	

	if (!mainMod.valid())
	{
		LOG_ERR_DTDM("Primary mod not found while iframe mods are enabled");
	}
	else if (lstrcmpW(mainMod.raw, L"debug") == 0)
	{
		//nothing right now
	}
	else if (lstrcmpW(mainMod.raw, L"gw2_taa") == 0)
	{
		ModHandler* gw2taa = (ModHandler*)(new Gw2TAA());
		//TODO
		//1. debug current code as load order is wrong
		//4. mix last saved frame with new one via "decent" taa algo
		//5. apply jitter in "right" places via pass detector
	}
	else if (lstrcmpW(mainMod.raw, L"gw2_rtx") == 0)
	{
		d912pxy::error::fatal(L"someday");
	}
}

//TODO: tidy this up somehow, literally this is one thing route 1 to many

void Manager::UnInit()
{
	for (int i = 1; i < modList.headIdx(); ++i)
	{
		modList[i]->UnInit();
		delete modList[i];
	}

	for (auto i = configValues.begin();i<configValues.end(); ++i)
	{
		if (i.value().valid())
			PXY_FREE(i.value().raw);
	}
}

void d912pxy::extras::IFrameMods::Manager::RP_PSO_Change(d912pxy_replay_item::dt_pso_raw* rpItem, d912pxy_replay_thread_context* rpContext)
{
	for (int i = 1; i < modList.headIdx(); ++i)
		modList[i]->RP_PSO_Change(rpItem, rpContext);
}

void Manager::RP_PreDraw(d912pxy_replay_item::dt_draw_indexed* rpItem, d912pxy_replay_thread_context* rpContext)
{
	for (int i = 1; i < modList.headIdx(); ++i)	
		modList[i]->RP_PreDraw(rpItem, rpContext);
}

void Manager::RP_PostDraw(d912pxy_replay_item::dt_draw_indexed* rpItem, d912pxy_replay_thread_context* rpContext)
{
	for (int i = 1; i < modList.headIdx(); ++i)
		modList[i]->RP_PostDraw(rpItem, rpContext);
}

void Manager::RP_RTDSChange(d912pxy_replay_item::dt_om_render_targets* rpItem, d912pxy_replay_thread_context* rpContext)
{
	for (int i = 1; i < modList.headIdx(); ++i)
		modList[i]->RP_RTDSChange(rpItem, rpContext);
}

void Manager::IFR_Start()
{
	for (int i = 1; i < modList.headIdx(); ++i)
		modList[i]->IFR_Start();
}

void Manager::IFR_End()
{
	for (int i = 1; i < modList.headIdx(); ++i)
		modList[i]->IFR_End();
}

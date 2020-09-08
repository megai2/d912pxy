#include "stdafx.h"
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

using namespace d912pxy::extras::IFrameMods;

bool d912pxy::extras::IFrameMods::Gw2TAA::shouldApplyJitter()
{
	return normDepthPass->inside() || resolvePass->inside();
}

Gw2TAA::Gw2TAA()
{
	taa = new GenericTAA(d912pxy_s.iframeMods.configVal(L"post_proc_apply"), nullptr);
	d912pxy_s.iframeMods.pushMod(taa);

}

void Gw2TAA::RP_PSO_Change(d912pxy_replay_item::dt_pso_raw* rpItem, d912pxy_replay_thread_context* rpContext)
{
	if (shouldApplyJitter())
	{
		;//d912pxy_s.render.db.shader.UseCustomCommonCode("taa_affected");
	}
	else
		;//d912pxy_s.render.db.shader.UseCustomCommonCode(nullptr);
}

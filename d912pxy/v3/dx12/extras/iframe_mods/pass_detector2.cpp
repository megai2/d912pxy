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
#include "stdafx.h"

using namespace d912pxy::extras::IFrameMods;

void PassDetector2::recordTarget(d912pxy_surface* surf, uint8_t index, uint8_t bits)
{
	PassTarget pt;
	pt.surf = surf;
	pt.frame_order = c_frame_order;
	pt.tied_bits = bits;
	pt.rtarget_index = index;

	const D3DSURFACE_DESC& sdsc = surf->GetL0Desc();
	pt.height = sdsc.Height;
	pt.width = sdsc.Width;

	if ((pt.width == bb_width) && (pt.height == bb_height))
	{
		pt.bbSized = true;
	}

	passTargets->push(pt);
}

PassDetector2::PassDetector2() : ModHandler()
{
	passTargets.Add(new Trivial::PushBuffer<PassTarget>());
	passTargets.Add(new Trivial::PushBuffer<PassTarget>());
	passTargets.Add(new Trivial::PushBuffer<PassTarget>());

	d912pxy_s.iframeMods.pushMod(this);
}

d912pxy::extras::IFrameMods::PassDetector2::~PassDetector2()
{
	passTargets.Cleanup();
}

void PassDetector2::RP_RTDSChange(d912pxy_replay_item::dt_om_render_targets* rpItem, d912pxy_replay_thread_context* rpContext)
{

	if (!passTargets->headIdx() && rpItem->rtv[0])
	{
		const D3DSURFACE_DESC& bbDesc = rpItem->rtv[0]->GetL0Desc();
		bb_height = bbDesc.Height;
		bb_width = bbDesc.Width;
	}

	uint8_t bits = rpItem->dsv ? 1 : 0;
	bits |= rpItem->rtv[0] ? 2 : 0;

	if (rpItem->dsv)
		recordTarget(rpItem->dsv, 0, bits);

	if (rpItem->rtv[0])
		recordTarget(rpItem->rtv[0], 1, bits);

	++c_frame_order;
}

void d912pxy::extras::IFrameMods::PassDetector2::UI_Draw()
{
	ImGui::Begin("PassDetect2");

	Trivial::PushBuffer<PassTarget>& localLPT = *lastPassTargets.load();

	for (int i = 0; i < localLPT.headIdx(); ++i)
	{
		const PassTarget& pt = localLPT[i];

		ImGui::Text("%u order %u bits %u %ux%u %s", i, pt.frame_order, pt.tied_bits, pt.width, pt.height, pt.bbSized ? "bb" : "");
	}

	ImGui::End();
}

void d912pxy::extras::IFrameMods::PassDetector2::RP_FrameStart()
{
	c_frame_order = 0;
	lastPassTargets = passTargets.operator->();
	passTargets.Next();
	passTargets->reset();
}

void d912pxy::extras::IFrameMods::PassDetector2::IFR_Start()
{
}

void d912pxy::extras::IFrameMods::PassDetector2::IFR_End()
{
}

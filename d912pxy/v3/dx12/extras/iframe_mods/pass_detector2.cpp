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

void PassDetector2::recordTarget(d912pxy_surface* surf, uint8_t bits)
{
	PassTarget pt;
	pt.surf = surf;
	pt.frame_order = c_frame_order;
	pt.bits = bits;

	auto& uniqueTargets = frData->uniqueTargetsArr[bits & PT_BIT_DS ? 1 : 0];

	uint16_t index = 0;
	for (uint16_t i = 1; i <= uniqueTargets.headIdx(); ++i)
	{
		if (uniqueTargets[i] == surf)
		{
			index = i;
			break;
		}
	}
	if (!index)
	{
		uniqueTargets.pushWithRef(surf);
		index = uniqueTargets.headIdx();
	}
	pt.rtarget_index = index;

	const D3DSURFACE_DESC& sdsc = surf->GetL0Desc();
	pt.height = sdsc.Height;
	pt.width = sdsc.Width;

	pt.bits |= ((pt.width == bb_width) && (pt.height == bb_height)) ? PT_BIT_BB_SIZED : 0;

	Hash64::step(frData->hash, pt.bits);

	pt.hash = frData->hash;

	if (namedPasses.headIdx())
		//FIXME: link should do sorted insert and this will optimize to very simple code
		for (int i = 1; i <= namedPasses.headIdx(); ++i)
		{
			if (namedPasses[i].hash == frData->hash)
				acctivePassName = namedPasses[i].name;
		}

	frData->passTargets.push(pt);
}

PassDetector2::PassDetector2() : ModHandler()
{
	frData.AddN(3);

	d912pxy_s.iframeMods.pushMod(this);
}

d912pxy::extras::IFrameMods::PassDetector2::~PassDetector2()
{
	frData.Cleanup();
}

uint64_t d912pxy::extras::IFrameMods::PassDetector2::getLastFrameHash()
{
	return lastFrData->hash;
}

bool d912pxy::extras::IFrameMods::PassDetector2::copyLastSurface(bool ds, bool converge, int index, int minimalPassNum, d912pxy_surface* target, ID3D12GraphicsCommandList* cl)
{
	auto& uniqueTargets = lastFrData->uniqueTargetsArr[ds ? 1 : 0];

	if (converge)
	{
		if (!isConvergingToLastFrame())
			return false;
	}

	if (uniqueTargets.headIdx() < index)
		return false;

	if (c_frame_order < minimalPassNum)
		return false;

	d912pxy_surface* src = uniqueTargets[index];
	const D3DSURFACE_DESC& tgtDesc = target->GetL0Desc();
	const D3DSURFACE_DESC& srcDesc = src->GetL0Desc();

	if ((tgtDesc.Width != srcDesc.Width) || (tgtDesc.Height != srcDesc.Height) || (tgtDesc.Format != srcDesc.Format))
		return false;

	src->BCopyToWStates(target, 3, cl, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, src->getContextState());

	return true;
}

d912pxy_surface* d912pxy::extras::IFrameMods::PassDetector2::makeBBsizedTarget(D3DFORMAT fmt)
{
	UINT levels = 1;
	d912pxy_surface* ret = d912pxy_surface::d912pxy_surface_com(
		bb_width,
		bb_height,
		fmt,
		0,
		D3DMULTISAMPLE_NONE,
		0,
		false,
		&levels,
		1,
		nullptr
	);
	ret->ConstructResource();

	return ret;
}

void PassDetector2::RP_RTDSChange(d912pxy_replay_item::dt_om_render_targets* rpItem, d912pxy_replay_thread_context* rpContext)
{
	uint8_t bits = rpItem->dsv ? PT_BIT_DSW : 0;
	bits |= rpItem->rtv[0] ? PT_BIT_RTW : 0;

	if ((bits & PT_MASK_MULTIWRITE) == PT_MASK_MULTIWRITE)
	{
		const D3DSURFACE_DESC& dsc1 = rpItem->dsv->GetL0Desc();
		const D3DSURFACE_DESC& dsc2 = rpItem->rtv[0]->GetL0Desc();

		if ((dsc1.Width == dsc2.Width) && (dsc1.Height == dsc2.Height))
			bits |= PT_BIT_SAME_SIZE;
	}

	newPass = false;

	if (rpItem->dsv && (rpItem->dsv != lastTargets[0]))
	{
		lastTargets[0] = rpItem->dsv;
		recordTarget(rpItem->dsv, bits | PT_BIT_DS);
		newPass |= true;
	}

	if (rpItem->rtv[0] && (rpItem->rtv[0] != lastTargets[1]))
	{
		lastTargets[1] = rpItem->rtv[0];
		recordTarget(rpItem->rtv[0], bits | PT_BIT_RT);
		newPass |= true;
	}

	if (newPass)
		++c_frame_order;
}

void d912pxy::extras::IFrameMods::PassDetector2::UI_Draw()
{	
	PerFrameData& pfd = *externFrData.load();

	ImGui::Begin("PassDetect2");

	ImGui::Text("Frame hash: %016llX", pfd.hash);

	ImGui::Text("Total DS: %u RT: %u", pfd.uniqueTargetsArr[1].headIdx(), pfd.uniqueTargetsArr[0].headIdx());

	ImGui::Separator();

	for (int i = 1; i <= pfd.passTargets.headIdx(); ++i)
	{
		const PassTarget& pt = pfd.passTargets[i];

		ImGui::Text("%03u: %03u-%03x %04ux%04u %016llX %s %s %s %s", pt.frame_order, pt.rtarget_index, pt.bits, pt.width, pt.height, pt.hash,
			pt.bits & PT_BIT_BB_SIZED ? "bb" : "  ", pt.bits & PT_BIT_SAME_SIZE ? "ss" : "  ", ((pt.bits & PT_MASK_MULTIWRITE) == PT_MASK_MULTIWRITE) ? "mw" : "  ",
			pt.bits & PT_BIT_DS ? "ds" : "rt");
	}


	ImGui::End();
}

void d912pxy::extras::IFrameMods::PassDetector2::RP_FrameStart()
{
	c_frame_order = 0;
	lastFrData = frData.operator->();
	externFrData = lastFrData;
	frData.Next();
	frData->reset();

	auto pp = d912pxy_s.dev.GetPrimarySwapChain()->GetCurrentPP();
	bbChanged = (bb_height != pp.BackBufferHeight) || (bb_width != pp.BackBufferWidth);
	bb_height = pp.BackBufferHeight;
	bb_width = pp.BackBufferWidth;

	lastTargets[0] = nullptr;
	lastTargets[1] = nullptr;
}

void d912pxy::extras::IFrameMods::PassDetector2::IFR_Start()
{
}

void d912pxy::extras::IFrameMods::PassDetector2::IFR_End()
{
}

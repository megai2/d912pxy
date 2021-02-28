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

const D3DFORMAT targetFormats[ReshadeCompat::TARGET_COUNT] =
{
	D3DFMT_A8B8G8R8,
	D3DFMT_D24X8,
	D3DFMT_A8B8G8R8,
	D3DFMT_A8B8G8R8,
	D3DFMT_A8B8G8R8
};

ReshadeCompat::ReshadeCompat()
{
	wchar_t* preUiPass = d912pxy_s.iframeMods.configVal(L"pre_ui_pass").raw;
	wchar_t* uiPassInitial = d912pxy_s.iframeMods.configVal(L"ui_pass_initial").raw;
	uint32_t uiPassRTDSmask = d912pxy_s.iframeMods.configVal(L"ui_pass_RTDS_mask").ui32();
	uint32_t uiPassTargetLock = d912pxy_s.iframeMods.configVal(L"ui_pass_target_lock").ui32();

	passes = new PassDetector2();
	d912pxy_s.iframeMods.pushMod(passes);

	uiPass = passes->registerName();
	depthPass = passes->registerName();
	resolvePass = passes->registerName();
	shadowPass = passes->registerName();
	normalPass = passes->registerName();

	//TODO: read hashes from cfg

	d912pxy_s.iframeMods.pushMod(this);
}

void ReshadeCompat::RP_RTDSChange(d912pxy_replay_item::dt_om_render_targets* rpItem, d912pxy_replay_thread_context* rpContext)
{
	if (!passes->passChanged())
		return;

	//TODO: indexes are changing between individual pass hashes, route them
	//TODO: all other stuff
	if (passes->inPass(uiPass))
		passes->copyLastSurface(false, false, 5, 5, targets[TARGET_COLOR], rpContext->cl);
}

void d912pxy::extras::IFrameMods::ReshadeCompat::RP_FrameStart()
{
	if (passes->isBBChangedThisFrame())
	{
		for (int i = 0; i < TARGET_COUNT; ++i)
		{
			if (targets[i])
				targets[i]->Release();
			targets[i] = passes->makeBBsizedTarget(targetFormats[i]);
		}
	}
}

void ReshadeCompat::RP_PSO_Change(d912pxy_replay_item::dt_pso_raw* rpItem, d912pxy_replay_thread_context* rpContext)
{
	if (passes->inPass(uiPass))
	{
		if (rpItem->rawState.val.rt[0].writeMask)
			rpItem->rawState.val.rt[0].writeMask |= D3D12_COLOR_WRITE_ENABLE_ALL;
	}
}
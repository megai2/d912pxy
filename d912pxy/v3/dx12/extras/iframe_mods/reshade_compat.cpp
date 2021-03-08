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
	D3DFMT_A8R8G8B8,
	D3DFMT_D24X8,
	D3DFMT_A8R8G8B8,
	D3DFMT_A8R8G8B8,
	D3DFMT_A8R8G8B8
};

ReshadeCompat::ReshadeCompat()
{
	passes = new PassDetector2();

	uiPass = passes->loadLinks(d912pxy_s.iframeMods.configValM(L"reshade_compat_pass_ui").raw);
	depthPass = passes->loadLinks(d912pxy_s.iframeMods.configValM(L"reshade_compat_pass_depth").raw);
	resolvePass = passes->loadLinks(d912pxy_s.iframeMods.configValM(L"reshade_compat_pass_resolve").raw);
	shadowPass = passes->loadLinks(d912pxy_s.iframeMods.configValM(L"reshade_compat_pass_shadow").raw);
	normalPass = passes->loadLinks(d912pxy_s.iframeMods.configValM(L"reshade_compat_pass_normal").raw);

	d912pxy_s.iframeMods.pushMod(this);

	reshadeAddonLib = LoadLibrary(L"./addons/gw2enhanced/gw2enhanced.addon");
	error::check(reshadeAddonLib != NULL, L"<game root>/addons/gw2enhanced/gw2enhanced.addon are not found or can't be loaded!");

	rsadSupplyTexture = (rsad_supplyTexture)GetProcAddress(reshadeAddonLib, "rsadSupplyTexture");
	error::check(rsadSupplyTexture != nullptr, L"rsadSupplyTexture is not found in reshade addon");
	rsadSetData = (rsad_setData)GetProcAddress(reshadeAddonLib, "rsadSetData");
	error::check(rsadSupplyTexture != nullptr, L"rsadSetData is not found in reshade addon");

}

void ReshadeCompat::RP_RTDSChange(d912pxy_replay_item::dt_om_render_targets* rpItem, d912pxy_replay_thread_context* rpContext)
{
	if (!passes->passChanged())
		return;

	if (passes->inPass(uiPass))
		passes->copyLastSurfaceNamed(TARGET_COLOR+1, targets[TARGET_COLOR], rpContext->cl);

	//TODO: all other stuff
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

			if (rsadSupplyTexture)
				rsadSupplyTexture(i, targets[i]->GetD12Obj());
		}
	}
}

void d912pxy::extras::IFrameMods::ReshadeCompat::UnInit()
{
	for (int i = 0; i < TARGET_COUNT; ++i)
	{
		if (targets[i])
			targets[i]->Release();
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
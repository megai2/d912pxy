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

ShaderConstRecorder::ShaderConstRecorder()
{
	UINT levels = 1;
	target = d912pxy_surface::d912pxy_surface_com(
			256,
			8,
			D3DFMT_R32F,
			D3DUSAGE_RENDERTARGET,
			D3DMULTISAMPLE_NONE,
			0,
			false,
			&levels,
			1,
			nullptr
		);

	wchar_t buf[256];
	wsprintf(buf, L"ShaderConstRecorder_target_%p", this);
	target->GetD12Obj()->SetName(buf);

	NativeShader fshd(L"shConstRecorder");
	fshd.getDesc().RTVFormats[0] = DXGI_FORMAT_R32_FLOAT;

	filler = new NativeFullRTDraw(fshd.ptr());
}

ShaderConstRecorder::~ShaderConstRecorder()
{
	target->Release();
	delete filler;
}

d912pxy_surface* ShaderConstRecorder::getTarget()
{
	return target;
}

void ShaderConstRecorder::record(const d912pxy_replay_thread_context& ctx)
{
	StateHolder rtHolder(ctx, StateHolder::ST_RTDS);

	D3D12_CPU_DESCRIPTOR_HANDLE dh = target->GetDHeapHandle();
	target->BTransitTo(0, D3D12_RESOURCE_STATE_RENDER_TARGET, ctx.cl);
	ctx.cl->OMSetRenderTargets(1, &dh, false, nullptr);
	filler->draw(ctx);
	target->BTransitTo(0, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, ctx.cl);
}

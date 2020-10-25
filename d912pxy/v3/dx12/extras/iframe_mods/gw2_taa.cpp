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

void d912pxy::extras::IFrameMods::Gw2TAA::initAndSetRSOverride()
{
	D3D12_DESCRIPTOR_RANGE ranges[3];
	d912pxy_s.render.iframe.FillPrimaryRSDescriptorRanges(ranges);

	D3D12_ROOT_PARAMETER rootParameters[5];
	d912pxy_s.render.iframe.FillPrimaryRSParameters(rootParameters, ranges);

	//b1 is for TAA data/params
	rootParameters[jitterCbufRSIdx].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[jitterCbufRSIdx].Descriptor.RegisterSpace = 0;
	rootParameters[jitterCbufRSIdx].Descriptor.ShaderRegister = 1;
	rootParameters[jitterCbufRSIdx].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC staticPCF;
	d912pxy_s.render.iframe.FillPrimaryRSstaticPCFSampler(staticPCF);

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.NumParameters = 5;
	rootSignatureDesc.NumStaticSamplers = 1;
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.pStaticSamplers = &staticPCF;

	d912pxy_s.render.iframe.OverrideRootSignature(d912pxy_s.dev.ConstructRootSignature(&rootSignatureDesc));
}

Gw2TAA::Gw2TAA()
{
	taa = new GenericTAA(d912pxy_s.iframeMods.configVal(L"post_proc_apply").raw, nullptr, jitterCbufRSIdx);
	d912pxy_s.iframeMods.pushMod(taa);

	initAndSetRSOverride();
	d912pxy_hlsl_generator::overrideCommonInclude(d912pxy_s.iframeMods.configVal(L"common_hlsl_override").raw);
}

void Gw2TAA::RP_PSO_Change(d912pxy_replay_item::dt_pso_raw* rpItem, d912pxy_replay_thread_context* rpContext)
{
	taa->setJitter(shouldApplyJitter(), rpContext);
}

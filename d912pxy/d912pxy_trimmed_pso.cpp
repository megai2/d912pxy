/*
MIT License

Copyright(c) 2018-2020 megai2

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

D3D12_GRAPHICS_PIPELINE_STATE_DESC d912pxy_trimmed_pso_desc::singleFullPSO = { 0 };

d912pxy_trimmed_pso_desc::d912pxy_trimmed_pso_desc() :
	val(),
	ref()
{
	
}

d912pxy_trimmed_pso_desc::~d912pxy_trimmed_pso_desc()
{
}

const d912pxy_trimmed_pso_desc::ValuePart& d912pxy_trimmed_pso_desc::GetValuePart()
{
	val.vdeclHash = ref.InputLayout->GetHash();

	return val;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC* d912pxy_trimmed_pso_desc::GetPSODesc()
{
	singleFullPSO.VS = { 0 };
	singleFullPSO.PS = { 0 };
	singleFullPSO.InputLayout = *ref.InputLayout->GetD12IA_InputElementFmt();
	singleFullPSO.NumRenderTargets = val.NumRenderTargets;

	//TODO: check about blend states for different RTs in DX9
	for (int i = 0; i<val.NumRenderTargets;++i)
	{
		singleFullPSO.BlendState.RenderTarget[i].SrcBlend = (D3D12_BLEND)val.rt[0].blend.src;
		singleFullPSO.BlendState.RenderTarget[i].SrcBlendAlpha = (D3D12_BLEND)val.rt[0].blend.srcAlpha;
		singleFullPSO.BlendState.RenderTarget[i].DestBlend = (D3D12_BLEND)val.rt[0].blend.dest;
		singleFullPSO.BlendState.RenderTarget[i].DestBlendAlpha = (D3D12_BLEND)val.rt[0].blend.destAlpha;
		singleFullPSO.BlendState.RenderTarget[i].BlendEnable = val.rt[0].blend.enable;
		singleFullPSO.BlendState.RenderTarget[i].RenderTargetWriteMask = val.rt[0].blend.writeMask;
		singleFullPSO.BlendState.RenderTarget[i].BlendOp = (D3D12_BLEND_OP)val.rt[0].blend.op;
		singleFullPSO.BlendState.RenderTarget[i].BlendOpAlpha = (D3D12_BLEND_OP)val.rt[0].blend.opAlpha;
	}

	for (int i = 0; i < PXY_INNER_MAX_RENDER_TARGETS; ++i)
		singleFullPSO.RTVFormats[i] = (DXGI_FORMAT)val.rt[i].format;

	singleFullPSO.RasterizerState.FillMode = (D3D12_FILL_MODE)val.rast.fillMode;

	singleFullPSO.RasterizerState.FrontCounterClockwise = val.rast.cullMode == D3DCULL_CW;
	singleFullPSO.RasterizerState.CullMode = val.rast.cullMode != D3DCULL_NONE ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_NONE;

	singleFullPSO.RasterizerState.SlopeScaledDepthBias = val.rast.slopeScaledDepthBias;
	singleFullPSO.RasterizerState.AntialiasedLineEnable = val.rast.antialiasedLineEnable;
	singleFullPSO.RasterizerState.DepthBias = val.rast.depthBias;

	//singleFullPSO.DepthStencilState = desc->DepthStencilState;

	singleFullPSO.DepthStencilState.DepthEnable = val.ds.enable;
	singleFullPSO.DepthStencilState.DepthWriteMask = (D3D12_DEPTH_WRITE_MASK)val.ds.writeMask;
	singleFullPSO.DepthStencilState.DepthFunc = (D3D12_COMPARISON_FUNC)val.ds.func;
	singleFullPSO.DepthStencilState.StencilEnable = val.ds.stencilEnable;
	singleFullPSO.DepthStencilState.FrontFace.StencilFailOp = (D3D12_STENCIL_OP)val.ds.frontFace.failOp;
	singleFullPSO.DepthStencilState.FrontFace.StencilPassOp = (D3D12_STENCIL_OP)val.ds.frontFace.passOp;
	singleFullPSO.DepthStencilState.FrontFace.StencilDepthFailOp = (D3D12_STENCIL_OP)val.ds.frontFace.depthFailOp;
	singleFullPSO.DepthStencilState.FrontFace.StencilFunc = (D3D12_COMPARISON_FUNC)val.ds.frontFace.func;
	singleFullPSO.DepthStencilState.BackFace.StencilFailOp = (D3D12_STENCIL_OP)val.ds.backFace.failOp;
	singleFullPSO.DepthStencilState.BackFace.StencilPassOp = (D3D12_STENCIL_OP)val.ds.backFace.passOp;
	singleFullPSO.DepthStencilState.BackFace.StencilDepthFailOp = (D3D12_STENCIL_OP)val.ds.backFace.depthFailOp;
	singleFullPSO.DepthStencilState.BackFace.StencilFunc = (D3D12_COMPARISON_FUNC)val.ds.backFace.func;
	singleFullPSO.DepthStencilState.StencilReadMask = val.ds.stencilReadMask;
	singleFullPSO.DepthStencilState.StencilWriteMask = val.ds.stencilWriteMask;
	singleFullPSO.DSVFormat = (DXGI_FORMAT)val.ds.format;

	return &singleFullPSO;
}

d912pxy_shader_pair_hash_type d912pxy_trimmed_pso_desc::GetShaderPairUID()
{
	return d912pxy_s.render.db.shader.GetPairUID(ref.VS, ref.PS);
}

void d912pxy_trimmed_pso_desc::HoldRefs(const bool yes)
{
	const INT refd = yes ? 1 : -1;
	ref.InputLayout->ThreadRef(refd);
	ref.PS->ThreadRef(refd);
	ref.VS->ThreadRef(refd);
}

bool d912pxy_trimmed_pso_desc::haveValidRefs()
{
	return (ref.PS != nullptr) && (ref.VS != nullptr) && (ref.InputLayout != nullptr);
}

d912pxy_mem_block d912pxy_trimmed_pso_desc::Serialize()
{
	serialized_data* output;
	auto mem = d912pxy_mem_block::alloc(&output);

	output->val = val;
	UINT unused;
	memcpy(output->declData, ref.InputLayout->GetDeclarationPtr(&unused), sizeof(D3DVERTEXELEMENT9) * PXY_INNER_MAX_VDECL_LEN);

	return mem;
}

bool d912pxy_trimmed_pso_desc::DeSerialize(d912pxy_mem_block data)
{
	if (data.size() != sizeof(serialized_data))
		return false;

	serialized_data* input = data.c_arr<serialized_data>();
	val = input->val;
	ref.InputLayout = d912pxy_vdecl::d912pxy_vdecl_com(input->declData);

	return true;
}

void d912pxy_trimmed_pso_desc::SetupBaseFullPSO(ID3D12RootSignature* defaultRootSignature)
{
	singleFullPSO.SampleDesc.Count = 1;
	singleFullPSO.SampleDesc.Quality = 0;
	singleFullPSO.SampleMask = 0xFFFFFFFF;
	singleFullPSO.RasterizerState.DepthBiasClamp = 0;
	singleFullPSO.RasterizerState.DepthClipEnable = 1;
	singleFullPSO.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	singleFullPSO.pRootSignature = defaultRootSignature;
}
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

ID3D12RootSignature* d912pxy_trimmed_pso_desc::defaultRootSignature = nullptr;

d912pxy_trimmed_pso_desc::d912pxy_trimmed_pso_desc() :
	val(),
	ref()
{
	
}

d912pxy_trimmed_pso_desc::~d912pxy_trimmed_pso_desc()
{
}

d912pxy_trimmed_pso_desc_key d912pxy_trimmed_pso_desc::GetKey()
{
	val.vdeclHash = ref.InputLayout->GetHash();

	return d912pxy_memtree2::memHash32s(&val, sizeof(value_part));
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC d912pxy_trimmed_pso_desc::GetPSODesc()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC ret = GetBaseFullPSO();

	ret.VS = *ref.VS->GetCode();
	ret.PS = *ref.PS->GetCode();
	ret.InputLayout = *ref.InputLayout->GetD12IA_InputElementFmt();
	ret.NumRenderTargets = val.NumRenderTargets;

	//for (numrt)
	{
		ret.BlendState.RenderTarget[0].SrcBlend = (D3D12_BLEND)val.rt0.blend.src;
		ret.BlendState.RenderTarget[0].SrcBlendAlpha = (D3D12_BLEND)val.rt0.blend.srcAlpha;
		ret.BlendState.RenderTarget[0].DestBlend = (D3D12_BLEND)val.rt0.blend.dest;
		ret.BlendState.RenderTarget[0].DestBlendAlpha = (D3D12_BLEND)val.rt0.blend.destAlpha;
		ret.BlendState.RenderTarget[0].BlendEnable = val.rt0.blend.enable;
		ret.BlendState.RenderTarget[0].RenderTargetWriteMask = val.rt0.blend.writeMask;
		ret.BlendState.RenderTarget[0].BlendOp = (D3D12_BLEND_OP)val.rt0.blend.op;
		ret.BlendState.RenderTarget[0].BlendOpAlpha = (D3D12_BLEND_OP)val.rt0.blend.opAlpha;
		ret.RTVFormats[0] = (DXGI_FORMAT)val.rt0.format;
	}

	ret.RasterizerState.FillMode = (D3D12_FILL_MODE)val.rast.fillMode;

	ret.RasterizerState.FrontCounterClockwise = val.rast.cullMode == D3DCULL_CW;
	ret.RasterizerState.CullMode = val.rast.cullMode != D3DCULL_NONE ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_NONE;

	ret.RasterizerState.SlopeScaledDepthBias = val.rast.slopeScaledDepthBias;
	ret.RasterizerState.AntialiasedLineEnable = val.rast.antialiasedLineEnable;
	ret.RasterizerState.DepthBias = val.rast.depthBias;

	//ret.DepthStencilState = desc->DepthStencilState;

	ret.DepthStencilState.DepthEnable = val.ds.enable;
	ret.DepthStencilState.DepthWriteMask = (D3D12_DEPTH_WRITE_MASK)val.ds.writeMask;
	ret.DepthStencilState.DepthFunc = (D3D12_COMPARISON_FUNC)val.ds.func;
	ret.DepthStencilState.StencilEnable = val.ds.stencilEnable;
	ret.DepthStencilState.FrontFace.StencilFailOp = (D3D12_STENCIL_OP)val.ds.frontFace.failOp;
	ret.DepthStencilState.FrontFace.StencilPassOp = (D3D12_STENCIL_OP)val.ds.frontFace.passOp;
	ret.DepthStencilState.FrontFace.StencilDepthFailOp = (D3D12_STENCIL_OP)val.ds.frontFace.depthFailOp;
	ret.DepthStencilState.FrontFace.StencilFunc = (D3D12_COMPARISON_FUNC)val.ds.frontFace.func;
	ret.DepthStencilState.BackFace.StencilFailOp = (D3D12_STENCIL_OP)val.ds.backFace.failOp;
	ret.DepthStencilState.BackFace.StencilPassOp = (D3D12_STENCIL_OP)val.ds.backFace.passOp;
	ret.DepthStencilState.BackFace.StencilDepthFailOp = (D3D12_STENCIL_OP)val.ds.backFace.depthFailOp;
	ret.DepthStencilState.BackFace.StencilFunc = (D3D12_COMPARISON_FUNC)val.ds.backFace.func;
	ret.DepthStencilState.StencilReadMask = val.ds.stencilReadMask;
	ret.DepthStencilState.StencilWriteMask = val.ds.stencilWriteMask;
	ret.DSVFormat = (DXGI_FORMAT)val.ds.format;

	return ret;
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

	d912pxy_vdecl* test = d912pxy_vdecl::d912pxy_vdecl_com(ref.InputLayout->GetDeclarationPtr(&unused));

	if (test->GetHash() != ref.InputLayout->GetHash())
	{
		assert(0, "rehash failed");
	}

	test->Release();

	return mem;
}

void d912pxy_trimmed_pso_desc::DeSerialize(d912pxy_mem_block data)
{
	serialized_data* input = data.c_arr<serialized_data>();
	val = input->val;
	ref.InputLayout = d912pxy_vdecl::d912pxy_vdecl_com(input->declData);
}

const D3D12_GRAPHICS_PIPELINE_STATE_DESC d912pxy_trimmed_pso_desc::GetBaseFullPSO()
{
	//TODO: check if this compiles to optimal no write-repat code
	D3D12_GRAPHICS_PIPELINE_STATE_DESC ret = { 0 };

	ret.SampleDesc.Count = 1;
	ret.SampleDesc.Quality = 0;
	ret.SampleMask = 0xFFFFFFFF;
	ret.RasterizerState.DepthBiasClamp = 0;
	ret.RasterizerState.DepthClipEnable = 1;
	ret.GS.pShaderBytecode = NULL;
	ret.DS.pShaderBytecode = NULL;
	ret.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	ret.pRootSignature = defaultRootSignature;

	return ret;
}
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

void NativeShader::compile()
{
	HRESULT ret = d912pxy_s.dx12.dev->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso));

	error::check(SUCCEEDED(ret), L"Native PSO is not compiled due to hr %lu", ret);
}

ComPtr<ID3DBlob> NativeShader::CompileSh(const char* profile, const wchar_t* shName)
{
	wchar_t targetFile[1024];
	ComPtr<ID3DBlob> ret, eret;
	wsprintf(targetFile, L"%s%s/native_shaders/%s.%S", 
		d912pxy_helper::GetFilePath(FP_IFRAME_MODS_BASE_PATH)->w, 
		d912pxy_s.config.GetValueRaw(PXY_CFG_EXTRAS_IFRAME_MOD_SOURCE),
		shName, profile);

	HRESULT hret = d912pxy_s.imports.d3d_compiler.CompileFromFile(
		targetFile, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", 
		profile, D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES | D3DCOMPILE_DEBUG, 0, &ret, &eret);

	if (eret && eret->GetBufferPointer())
		LOG_WARN_DTDM("Native shader %s compile message = %S", targetFile, eret->GetBufferPointer());

	error::check(SUCCEEDED(hret), L"Native shader %s is not compiled due to hr %lu", targetFile, hret);

	return ret;
}

NativeShader::NativeShader(const wchar_t* shName) : d912pxy_noncom(L"NativeShader")
{
	vsBlob = CompileSh("vs_5_1", shName);
	psBlob = CompileSh("ps_5_1", shName);

	desc.pRootSignature = d912pxy_s.render.iframe.getDefaultRS();
	desc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	desc.VS.BytecodeLength = vsBlob->GetBufferSize();
	desc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	desc.PS.BytecodeLength = psBlob->GetBufferSize();

	//setup some common defaults

	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.SampleMask = 0xFFFFFFFF;
	desc.RasterizerState.DepthBiasClamp = 0;
	desc.RasterizerState.DepthClipEnable = 1;
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.NumRenderTargets = 1;
	desc.BlendState.RenderTarget[0].BlendEnable = false;
	desc.BlendState.RenderTarget[0].RenderTargetWriteMask = 0xF;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	desc.InputLayout.NumElements = 1;
	desc.InputLayout.pInputElementDescs = &defaultFv4Pos;

	defaultFv4Pos.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	defaultFv4Pos.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	defaultFv4Pos.SemanticName = "POSN0E";
}

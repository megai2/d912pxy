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

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_DEVICE_SHADERS

HRESULT d912pxy_device::SetVertexShader(IDirect3DVertexShader9* pShader)
{
	d912pxy_s.render.state.pso.VShader(PXY_COM_LOOKUP(pShader, shader));
		
	return D3D_OK;
}

HRESULT d912pxy_device::SetPixelShader(IDirect3DPixelShader9* pShader) 
{
	d912pxy_s.render.state.pso.PShader(PXY_COM_LOOKUP(pShader, shader));
		
	return D3D_OK;
}

HRESULT d912pxy_device::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{ 
#ifdef _DEBUG
	if (PXY_INNER_MAX_SHADER_CONSTS <= ((StartRegister + Vector4fCount) * 4))
	{
		LOG_DBG_DTDM("too many shader consts, trimming");
		Vector4fCount = PXY_INNER_MAX_SHADER_CONSTS/4 - StartRegister;
	}
#endif

	d912pxy_s.render.batch.SetShaderConstF(0, StartRegister, Vector4fCount, (float*)pConstantData);

	return D3D_OK;
}

HRESULT d912pxy_device::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) 
{ 

#ifdef _DEBUG
	if (PXY_INNER_MAX_SHADER_CONSTS <= ((StartRegister + Vector4fCount) * 4))
	{
		LOG_DBG_DTDM3("too many shader consts, trimming");
		Vector4fCount = PXY_INNER_MAX_SHADER_CONSTS/4 - StartRegister;
	}
#endif

	d912pxy_s.render.batch.SetShaderConstF(1, StartRegister, Vector4fCount, (float*)pConstantData);


	return D3D_OK;
}

HRESULT d912pxy_device::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
	IDirect3DVertexShader9* vsObj = PXY_COM_CAST_(IDirect3DVertexShader9, d912pxy_s.render.state.pso.GetVShader());
	if (vsObj)
		vsObj->AddRef();

	*ppShader = vsObj;		
	return D3D_OK;
}

HRESULT d912pxy_device::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
	IDirect3DPixelShader9* psObj = PXY_COM_CAST_(IDirect3DPixelShader9, d912pxy_s.render.state.pso.GetPShader());
	if (psObj)
		psObj->AddRef();

	*ppShader = psObj;
	return D3D_OK;
}

ID3D12RootSignature * d912pxy_device::ConstructRootSignature(D3D12_ROOT_SIGNATURE_DESC* rootSignatureDesc)
{
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;

	ID3D12RootSignature* rsObj;

	HRESULT ret;

	ret = d912pxy_s.imports.dx12.SerializeRootSignature(rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

	if (FAILED(ret))
	{
		if ((error != NULL) && (error->GetBufferSize()))
		{
			LOG_ERR_DTDM("error: %S", error->GetBufferPointer());
		}
		LOG_ERR_THROW2(ret, "SerializeRootSignature failed");
	}

	LOG_ERR_THROW2(d912pxy_s.dx12.dev->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rsObj)), "CreateRootSignature failed");

	return rsObj;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
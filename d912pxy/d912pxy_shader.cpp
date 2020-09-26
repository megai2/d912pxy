/*
MIT License

Copyright(c) 2018-2019 megai2

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

d912pxy_shader * d912pxy_shader::d912pxy_shader_com(PXY_INSTANCE_PAR UINT isVs, const DWORD * origCode, d912pxy_shader_uid uid)
{
	d912pxy_com_object* ret = d912pxy_s.com.AllocateComObj(PXY_COM_OBJ_SHADER);
	
	static const wchar_t* objName[2] = {
		L"pshader",
		L"vshader"
	};
	
	new (&ret->shader)d912pxy_shader(objName[isVs], origCode, uid, isVs);
	
	ret->vtable = d912pxy_com_route_get_vtable(PXY_COM_ROUTE_SHADER);

	return &ret->shader;
}

d912pxy_shader::d912pxy_shader(const wchar_t * shtName, const DWORD* fun, d912pxy_shader_uid uid, UINT shdType) : d912pxy_comhandler(PXY_COM_OBJ_SHADER, shtName)
{	
	if (fun)
	{
		mUID = d912pxy_s.render.db.shader.GetUID((DWORD*)fun, &oLen);

		PXY_MALLOC(oCode, oLen * 4, DWORD*);
		memcpy(oCode, fun, oLen * 4);
	}
	else {
		oCode = 0;
		oLen = 0;
		mUID = uid;
	}		

	shaderType = shdType;

	pairs = new d912pxy_ringbuffer<d912pxy_shader_pair_hash_type>(0x10, 2);

	bytecode.code = 0;
	bytecode.blob = nullptr;	
}

d912pxy_shader::~d912pxy_shader()
{
	delete pairs;

	if (oCode) {
		PXY_FREE(oCode);
	}
	if ((!bytecode.blob) && (bytecode.code)) {
		PXY_FREE(bytecode.code);
	}
}

D3D12_SHADER_BYTECODE * d912pxy_shader::GetCode()
{
	if (!bytecode.code)
	{	
		d912pxy_shader_replacer* replacer = new d912pxy_shader_replacer(oCode, oLen, mUID, shaderType);
		bytecode = replacer->GetCode();
		delete replacer;

		if (oCode)
			PXY_FREE(oCode);

		oCode = NULL;
	}

	return (D3D12_SHADER_BYTECODE*)&bytecode;
}

d912pxy_mem_block d912pxy_shader::GetHLSLSource()
{
	if (oCode)
		return d912pxy_shader_replacer(oCode, oLen, mUID, shaderType).GetHLSL();
	else
		LOG_ERR_DTDM("No original DXBC present to generate HLSL for %016llX", mUID);

	return d912pxy_mem_block::null();
}

d912pxy_shader_uid d912pxy_shader::GetID()
{
	return mUID;
}

void d912pxy_shader::NotePairUsage(d912pxy_shader_pair_hash_type pairHash)
{
	pairs->WriteElement(pairHash);
}

UINT d912pxy_shader::FinalReleaseCB()
{
	return 1;
}

void d912pxy_shader::RemovePairs()
{
	while (pairs->HaveElements())
	{
		d912pxy_shader_pair_hash_type ha = pairs->GetElement();

		d912pxy_s.render.db.shader.DeletePair(ha);

		pairs->Next();
	}
}

#define D912PXY_METHOD_IMPL_CN d912pxy_shader

D912PXY_METHOD_IMPL_NC_(ULONG, ReleaseWithPairRemoval)(THIS)
{
	ULONG ret = Release();

	if (!ret)
	{
		RemovePairs();
	}

	return ret;
}

#undef D912PXY_METHOD_IMPL_CN

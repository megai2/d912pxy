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

d912pxy_shader_db::d912pxy_shader_db() 
{

}


d912pxy_shader_db::~d912pxy_shader_db()
{
}

void d912pxy_shader_db::Init()
{
	NonCom_Init(L"shader database");
}

void d912pxy_shader_db::UnInit()
{
	for (auto iter = shaderPairs.begin(); iter < shaderPairs.end(); ++iter)
	{
		d912pxy_shader_pair*& pair = iter.value();
		delete pair;
	}

	d912pxy_noncom::UnInit();
}

UINT32 d912pxy_shader_db::CalcTokenCount(DWORD* code)
{
	UINT32 ret = 0;
	//use inst len for sh ver 2 and up
	bool sm1 = D3DSHADER_VERSION_MAJOR(code[ret++]) <= 1;

	d912pxy_dxbc9::token tok;
	tok.load(code[ret], d912pxy_dxbc9::token_type::unk);

	while (tok.iType != d912pxy_dxbc9::token_type::end)
	{
		ret += tok.length(sm1);
		tok.load(code[ret], d912pxy_dxbc9::token_type::unk);
	}
	return ret + 1;
}

d912pxy_shader_uid d912pxy_shader_db::GetUID(DWORD * code, UINT32* len)
{
	*len = CalcTokenCount(code);
	return d912pxy::Hash64(d912pxy::MemoryArea((void*)code, (*len * sizeof(DWORD)))).value;
}

d912pxy_shader_pair_hash_type d912pxy_shader_db::GetPairUID(d912pxy_shader * vs, d912pxy_shader * ps)
{
	d912pxy_shader_uid pdc[2] = { vs->GetID(), ps->GetID() };

	LOG_DBG_DTDM2("ShaderPair %016llX %016llX", pdc[0], pdc[1]);

	d912pxy_shader_pair_hash_type ha = (d912pxy_shader_pair_hash_type)(pdc[0] ^ pdc[1]);

	return ha;
}

d912pxy_shader_pair* d912pxy_shader_db::GetPair(d912pxy_shader* vs, d912pxy_shader* ps)
{
	d912pxy_shader_pair_hash_type ha = GetPairUID(vs, ps);

	d912pxy::mt::containter::OptRef<ShaderPairStorage> ref(shaderPairs, ha);

	if (!ref.val)
	{
		d912pxy_shader_uid pdc[2] = { vs->GetID(), ps->GetID() };

		ref.add() = new d912pxy_shader_pair(ha, pdc);
	}

	return *ref.val;
}
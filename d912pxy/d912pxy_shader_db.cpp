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

d912pxy_shader_db::d912pxy_shader_db(d912pxy_device* dev) : d912pxy_noncom(dev, L"shader database")
{
	shaderPairs = new d912pxy_memtree2(sizeof(d912pxy_shader_uid)*2, 0xFF, 2, 0);
	shaderCodes = new d912pxy_memtree(sizeof(d912pxy_shader_uid), 0xFF, 2, 0);

	InitializeCriticalSection(&treeAcCS);

	d912pxy_s(sdb) = this;
}


d912pxy_shader_db::~d912pxy_shader_db()
{
	shaderPairs->Begin();

	while (!shaderPairs->IterEnd())
	{
		UINT64 cid = shaderPairs->CurrentCID();
		if (cid)
		{
			d912pxy_shader_pair* item = (d912pxy_shader_pair*)cid;

			delete item;
		}
		shaderPairs->Next();
	}

	delete shaderPairs;

	shaderCodes->Begin();

	while (!shaderCodes->IterEnd())
	{
		UINT64 cid = shaderCodes->CurrentCID();
		if (cid)
		{
			d912pxy_shader_code_item* item = (d912pxy_shader_code_item*)cid;

			delete item;
		}
		shaderCodes->Next();
	}

	delete shaderCodes;

}

d912pxy_shader_uid d912pxy_shader_db::GetUID(DWORD * code, UINT32* len)
{
	UINT64 hash = 0xcbf29ce484222325;
	UINT ctr = 0;

	while (code[ctr >> 2] != 0x0000FFFF)
	{
		UINT8 dataByte = ((UINT8*)code)[ctr];

		hash = hash ^ dataByte;
		hash = hash * 1099511628211;
		++ctr;
	}

	*len = (ctr >> 2) + 1;

	return hash;
}

d912pxy_shader_code d912pxy_shader_db::GetCode(d912pxy_shader_uid UID, d912pxy_shader * shader)
{
/*	shaderCodes->PointAt(&UID);

	d912pxy_shader_code_item* it = (d912pxy_shader_code_item*)shaderCodes->CurrentCID();

	if (it)
	{
		return it->GetCode();
	}
	else*/ {

		d912pxy_shader_replacer* replacer = new d912pxy_shader_replacer(shader->GetOCode(), shader->GetOLen(), UID);

		d912pxy_shader_code ret = replacer->GetCode();

		shader->SetMaxVars(replacer->GetMaxVars());

		delete replacer;

		return ret;
	}
}

d912pxy_shader_pair * d912pxy_shader_db::GetPair(d912pxy_vshader* vs, d912pxy_pshader* ps)
{
	//d912pxy_shader_uid pdc[2] = { (UINT64)vs, (UINT64)ps };
	d912pxy_shader_uid pdc[2] = { vs->GetID(), ps->GetID() };

	LOG_DBG_DTDM2("ShaderPair %016llX %016llX", pdc[0], pdc[1]);
	
 	UINT32 ha = shaderPairs->memHash32(pdc);
	
	EnterCriticalSection(&treeAcCS);

	shaderPairs->PointAtNH(&ha);

	d912pxy_shader_pair* it = (d912pxy_shader_pair*)shaderPairs->CurrentCID();

	if (it)
	{
		LeaveCriticalSection(&treeAcCS);
		return it;
	}
	else {
		it = new d912pxy_shader_pair(ha, m_dev);

		vs->NotePairUsage(ha);
		ps->NotePairUsage(ha);

		shaderPairs->SetValue((intptr_t)it);

		LeaveCriticalSection(&treeAcCS);

		return it;
	}
}

void d912pxy_shader_db::DeletePair(UINT32 ha)
{
	EnterCriticalSection(&treeAcCS);
	shaderPairs->PointAtNH(&ha);

	d912pxy_shader_pair* it = (d912pxy_shader_pair*)shaderPairs->CurrentCID();
	
	if (it)
		it->Release();

	shaderPairs->SetValue(0);

	LeaveCriticalSection(&treeAcCS);
}

void d912pxy_shader_db::CleanUnusedPairs()
{
	UINT32 time = GetTickCount();
	UINT32 cnt;
	d912pxy_memtree2_node* pool = shaderPairs->AsyncIterBase(&cnt);
	
	for (UINT32 i =0; i!=cnt;++i)
	{		
		EnterCriticalSection(&treeAcCS);
		d912pxy_shader_pair* it = (d912pxy_shader_pair*)pool[i].contentId;
		if (it)
		{
			if ((it->GetLastAccessTime() + PXY_INNER_SHADER_PAIR_LIFETIME) < time)
			{
				delete it;
				pool[i].contentId = 0;
			}			
		}
		LeaveCriticalSection(&treeAcCS);
		//megai2: make sleep on this thread, cuz we need make this huge cleanup on background
		Sleep(0);
	}
}

d912pxy_shader_code_item::d912pxy_shader_code_item(d912pxy_shader_uid mUID) : d912pxy_noncom(NULL, L"shader code cache item")
{
	//TODO load CSO and other needed stuff from disk
}

d912pxy_shader_code_item::~d912pxy_shader_code_item()
{
}

d912pxy_shader_code d912pxy_shader_code_item::GetCode()
{
	return code;
}

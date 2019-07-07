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

d912pxy_shader_pair::d912pxy_shader_pair(d912pxy_shader_pair_hash_type nodeId, d912pxy_shader_uid* shd, d912pxy_device* dev) : d912pxy_noncom( L"shader pair")
{	
	maxPsoId = 512;

	UINT32 msz = sizeof(d912pxy_pso_cache_item*)*maxPsoId;

	PXY_MALLOC(psoItems, msz, d912pxy_pso_cache_item**);
	ZeroMemory(psoItems, msz);

	node = nodeId;

	memcpy(shdUID, shd, sizeof(d912pxy_shader_uid) * 2);

}

d912pxy_shader_pair::~d912pxy_shader_pair()
{
	if (d912pxy_s(sdb)->GetPrecompileFlag() & PXY_SDB_PSO_PRECOMPILE_SAVE)
	{
		d912pxy_shader_pair_cache_entry entryData;
		ZeroMemory(&entryData, sizeof(d912pxy_shader_pair_cache_entry));

		entryData.vs = shdUID[0];
		entryData.ps = shdUID[1];

		for (int i = 1; i != maxPsoId; ++i)
		{
			if (psoItems[i] != 0)
			{
				entryData.compiled[i >> 6] |= 1ULL << (i & 0x3F);
			}
		}

		d912pxy_s(vfs)->ReWriteFileH(node, &entryData, sizeof(d912pxy_shader_pair_cache_entry), PXY_VFS_BID_PSO_PRECOMPILE_LIST);
	}

	for (int i = 0; i != maxPsoId; ++i)
	{
		if (psoItems[i] != 0)
		{
			psoItems[i]->Release();
		}
	}

	PXY_FREE(psoItems);
}

void d912pxy_shader_pair::PrecompilePSO(UINT32 idx, d912pxy_trimmed_dx12_pso * dsc)
{
	CheckArrayAllocation(idx);

	d912pxy_pso_cache_item* ret = d912pxy_pso_cache_item::d912pxy_pso_cache_item_com(dsc);

	ret->Compile();

	psoItems[idx] = ret;
}

void d912pxy_shader_pair::CheckArrayAllocation(UINT32 idx)
{
	if (idx >= maxPsoId)
	{
		intptr_t oldEnd = maxPsoId * sizeof(d912pxy_pso_cache_item*);

		LOG_DBG_DTDM3("GetPSOCacheData realloc %u => %u", maxPsoId, idx + 100);

		intptr_t extendSize = ((idx - maxPsoId) + 100) * sizeof(d912pxy_pso_cache_item*);

		maxPsoId = idx + 100;

		//psoItems = (d912pxy_pso_cache_item**)realloc(psoItems, maxPsoId * sizeof(d912pxy_pso_cache_item*));
		PXY_REALLOC(psoItems, maxPsoId * sizeof(d912pxy_pso_cache_item*), d912pxy_pso_cache_item**);

		ZeroMemory((void*)((intptr_t)psoItems + oldEnd), extendSize);
	}
}

d912pxy_pso_cache_item* d912pxy_shader_pair::GetPSOCacheData(UINT32 idx, d912pxy_trimmed_dx12_pso* dsc)
{
	CheckArrayAllocation(idx);
	
	d912pxy_pso_cache_item* ret = psoItems[idx];

	if (!ret)
	{		
		ret = d912pxy_pso_cache_item::d912pxy_pso_cache_item_com(dsc);

		d912pxy_s(psoCache)->CompileItem(ret);

		psoItems[idx] = ret;
	}

	return ret;
}

d912pxy_pso_cache_item * d912pxy_shader_pair::GetPSOCacheDataMT(UINT32 idx, d912pxy_trimmed_dx12_pso * dsc)
{
	d912pxy_pso_cache_item* ret = NULL;

	lock.LockedAdd(1);
	
	if (idx < maxPsoId)
		ret = psoItems[idx];

	lock.Add(-1);

	if (!ret)
	{
		lock.HoldWait(0);
		ret = GetPSOCacheData(idx, dsc);
		lock.Release();
	}

	return ret;
}

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

d912pxy_pso_db::d912pxy_pso_db()
{
}

d912pxy_pso_db::~d912pxy_pso_db()
{
}

void d912pxy_pso_db::Init()
{
	d912pxy_s.render.db.psoMTCompiler.Init();

	NonCom_Init(L"pso db");
	InitThread("d912pxy pso compile", 0);

	cacheIncID = 0;
	
	saveCache = d912pxy_s.config.GetValueB(PXY_CFG_SDB_SAVE_PSO_CACHE);

	if (d912pxy_s.config.GetValueB(PXY_CFG_SDB_LOAD_PSO_CACHE))
	{
		if (!d912pxy_s.config.GetValueB(PXY_CFG_SDB_KEEP_PAIRS))
			LOG_WARN_DTDM("pso cache load enabled but keep pairs is disabled, skipping cache load");
		else
			LoadCachedData();
	}

	psoCompileQue = new d912pxy_ringbuffer<d912pxy_pso_item*>(0xFFFF, 0);
}

void d912pxy_pso_db::UnInit()
{
	Stop();
	delete psoCompileQue;

	d912pxy_s.render.db.psoMTCompiler.UnInit();

	d912pxy_noncom::UnInit();
}

d912pxy_pso_item* d912pxy_pso_db::GetByDescMT(d912pxy_trimmed_pso_desc* desc)
{
	if (!desc->haveValidRefs())
	{
		LOG_DBG_DTDM3("fixed pipe draw issued, ignoring");
		return NULL;
	}

	auto key = desc->GetValuePart();
	uint32_t id = 0;
	bool newId = false;
	
	{
		d912pxy::mt::containter::OptRef<d912pxy_trimmed_pso_desc::IdStorage> lookup(cacheIndexes, key);
	
		if (!lookup.val)
		{
			id = ++cacheIncID;
			lookup.add() = id;
			newId = true;
		}
		else
			id = *lookup.val;
	}

	if (newId && saveCache)
		SaveKeyToCache(cacheIndexes.prepareKey(key), desc);

	return d912pxy_s.render.db.shader.GetPair(desc->ref.VS, desc->ref.PS)->GetPSOItemMT((UINT32)id, desc);
}

void d912pxy_pso_db::EnqueueCompile(d912pxy_pso_item* item)
{
	item->MarkPushedToCompile();
	psoCompileQue->WriteElementMT(item);
	SignalWork();
}

UINT d912pxy_pso_db::GetCompileQueueLength()
{
	return psoCompileQue->TotalElements();
}

void d912pxy_pso_db::LockCompileQue(UINT lock)
{
	if (lock)
	{
		if (compileQueLock.GetValue())
			return;

		compileQueLock.SetValue(1);

		SignalWork();

		compileQueLock.Wait(2);
	}
	else
		compileQueLock.SetValue(0);
}

void d912pxy_pso_db::ThreadJob()
{
	CheckCompileQueueLock();

	while (psoCompileQue->HaveElements())
	{
		d912pxy_pso_item* it = psoCompileQue->PopElementMTG();

		it->Compile();
	
		CheckCompileQueueLock();
	}
}

void d912pxy_pso_db::LoadCachedData()
{
	//todo: use proper buffer
	d912pxy_ringbuffer<d912pxy_trimmed_pso_desc> psoDescs(65535, 2);
	d912pxy_ringbuffer<d912pxy_pso_item**> psoItems(65535, 2);
	psoDescs.WriteElement(d912pxy_trimmed_pso_desc());
	
	{
		auto keyList = d912pxy_s.vfs.GetFileList(d912pxy_vfs_bid::pso_cache_keys);
		UINT keyIdx = 0;

		while (keyList->HaveElements())
		{

			auto keyName = keyList->PopElement();
			auto data = d912pxy_s.vfs.ReadFile(d912pxy_vfs_path(keyName, d912pxy_vfs_bid::pso_cache_keys));

			if (!data.isNullptr())
			{
				d912pxy_trimmed_pso_desc psoDesc;
				if (!psoDesc.DeSerialize(data))
				{
					LOG_ERR_DTDM("PSO with keyname %08lX have wrong data (old or corrupted)", keyName);
					continue;
				}
				data.Delete();

				if (psoDesc.ref.InputLayout->GetHash() != psoDesc.val.vdeclHash)
				{
					LOG_ERR_DTDM("PSO IA ref hash differs from saved in desc (%08lX != %08lX)", 
						psoDesc.ref.InputLayout->GetHash(), psoDesc.val.vdeclHash);
					psoDesc.ref.InputLayout->Release();
					continue;
				}

				auto psoValue = psoDesc.GetValuePart();
				auto psoKey = cacheIndexes.prepareKey(psoValue);

				if (psoKey.data() != keyName)
				{
					LOG_ERR_DTDM("PSO keyname %lX differs from %lX that defined by data in PSO", keyName, psoKey.data());
					psoDesc.ref.InputLayout->Release();
					continue;
				}

				++keyIdx;
				psoDescs.WriteElement(psoDesc);
				cacheIndexes.findPrepared(psoKey) = keyIdx;
			}
			else {
				LOG_ERR_DTDM("can't read PSO data with key %lX ", keyName);
			}
		}

		cacheIncID = keyIdx;

		LOG_INFO_DTDM("loaded %u PSO descriptions", cacheIncID);

		delete keyList;
	}

	if (!cacheIncID)
	{
		LOG_INFO_DTDM("No PSO descriptions found, precompile will fail, skipping it");
		return;
	}

	UINT psoItemsTotal = 0;
	UINT psoItemsCompiled = 0;

	{
		auto precompList = d912pxy_s.vfs.GetFileList(d912pxy_vfs_bid::pso_precompile_list);
		auto shaderBuffer = new d912pxy::Memtree<d912pxy_shader_uid, d912pxy_shader*, d912pxy::RawHash<d912pxy_shader_uid>>();

		while (precompList->HaveElements())
		{
			auto cacheElement = d912pxy_vfs_path(precompList->PopElement(), d912pxy_vfs_bid::pso_precompile_list);
			d912pxy_shader_pair_cache_entry* pairEntry = d912pxy_s.vfs.ReadFile(cacheElement).c_arr<d912pxy_shader_pair_cache_entry>();

			if (!pairEntry)
				continue;

			auto findShader = [&shaderBuffer](d912pxy_shader_uid uid)
			{
				d912pxy_shader*& shRef = shaderBuffer->find(uid);
				if (!shRef)
					shRef = d912pxy_shader::d912pxy_shader_com(PXY_SHADER_TYPE_VS, 0, uid);

				return shRef;
			};

			d912pxy_shader* vs = findShader(pairEntry->vs);
			d912pxy_shader* ps = findShader(pairEntry->ps);

			if (!cacheIndexes.containsPrepared(pairEntry->pso))
			{
				LOG_ERR_DTDM("Can't precompile PSO with VS: %llX PS: %llX DSC KEY: %lX due to missing PSO desc", 
					pairEntry->vs, pairEntry->ps, pairEntry->pso.data());
			}
			else {
				d912pxy_shader_pair* pair = d912pxy_s.render.db.shader.GetPair(vs, ps);
				pair->CheckArrayAllocation(cacheIncID);

				uint32_t psoDescIdx = cacheIndexes.findPrepared(pairEntry->pso);
				auto psoDesc = psoDescs.GetElementOffsetPtr(psoDescIdx);

				psoDesc->ref.PS = ps;
				psoDesc->ref.VS = vs;

				psoItems.WriteElement(pair->PrecompilePSO(psoDescIdx, psoDesc));
				++psoItemsCompiled;
			}

			++psoItemsTotal;

			PXY_FREE(pairEntry);
		}

		for (auto iter = shaderBuffer->begin(); iter < shaderBuffer->end(); ++iter)
		{
			d912pxy_shader*& shader = iter.value();
			if (shader)
				shader->Release();
		}

		delete shaderBuffer;
		delete precompList;
	}

	LOG_INFO_DTDM("Pushed %u out of %u PSO items to compilation", psoItemsCompiled, psoItemsTotal);

	size_t compilePendingItems = d912pxy_pso_item::GetTotalPendingItems();
	int compilerIntegrity = 10;
	while (compilePendingItems && compilerIntegrity)
	{
		LOG_INFO_DTDM("Waiting for PSO compiler threads to process %u items", compilePendingItems);
		Sleep(500);
		compilePendingItems = d912pxy_pso_item::GetTotalPendingItems();

		if ((compilePendingItems > 0) && (d912pxy_s.render.db.psoMTCompiler.getTotalQueueLength() == 0))
			--compilerIntegrity;
	}

	if (!compilerIntegrity)
	{
		LOG_ERR_DTDM("PSO compiler bugged out, skipping leftover PSO items");
	}

	//remove failed PSO items in order to compile them later on when game need them
	while (psoItems.HaveElements())
	{
		d912pxy_pso_item** i = psoItems.PopElement();
		if (!(*i)->GetPtr())
		{
			(*i)->Release();
			i = nullptr;
		}
	}

	psoDescs.PopElement();
	while (psoDescs.HaveElements())
	{
		psoDescs.PopElement().ref.InputLayout->Release();		
	}
}

void d912pxy_pso_db::CheckCompileQueueLock()
{
	if (compileQueLock.GetValue() == 1)
	{
		compileQueLock.SetValue(2);

		while (compileQueLock.GetValue() == 2)
		{
			Sleep(1000);
		}
	}
}

void d912pxy_pso_db::SaveKeyToCache(d912pxy_trimmed_pso_desc::StorageKey key, d912pxy_trimmed_pso_desc* desc)
{
	d912pxy_mem_block data = desc->Serialize();

	d912pxy_s.vfs.WriteFile(d912pxy_vfs_path(key.val.value, d912pxy_vfs_bid::pso_cache_keys), data);
	data.Delete();

	//if pso data get corrupted on save
	/*{
		auto data = d912pxy_s.vfs.ReadFile(d912pxy_vfs_path(key, d912pxy_vfs_bid::pso_cache_keys));

		if (!data.isNullptr())
		{
			d912pxy_trimmed_pso_desc psoDesc;
			psoDesc.DeSerialize(data);
			data.Delete();

			if (psoDesc.GetKey() != key)
			{
				LOG_ERR_THROW2(-1, "this is totally wrong");
			}
		}
	}*/

}

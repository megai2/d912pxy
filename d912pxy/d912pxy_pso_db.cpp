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

bool d912pxy_pso_db::allowRealtimeChecks = false;

d912pxy_pso_db::d912pxy_pso_db()
{
}

d912pxy_pso_db::~d912pxy_pso_db()
{
}

void d912pxy_pso_db::Init()
{
	NonCom_Init(L"pso db");
	InitThread("d912pxy pso compile", 0);

	cacheIndexes = new d912pxy_memtree2(sizeof(d912pxy_trimmed_pso_desc_hash), PXY_INNER_MAX_PSO_CACHE_NODES, 2);
	cacheIncID = 0;
	
	allowRealtimeChecks = d912pxy_s.config.GetValueB(PXY_CFG_SDB_ALLOW_REALTIME_CHECKS);

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

	delete cacheIndexes;

	d912pxy_noncom::UnInit();
}

d912pxy_pso_item* d912pxy_pso_db::GetByDescMT(d912pxy_trimmed_pso_desc* desc)
{
	if (!desc->haveValidRefs())
	{
		LOG_DBG_DTDM3("fixed pipe draw issued, ignoring");
		return NULL;
	}

	UINT32 key = desc->GetHash();
	UINT64 id = cacheIndexes->PointAtMemMTR(&key, 4);

	if (id == 0)
	{
		id = cacheIndexes->PointAtMemMTRW(&key, 4);

		if (!id)
			id = ++cacheIncID;

		cacheIndexes->PointAtMemMTW(id);

		if (saveCache)
			SaveKeyToCache(key, desc);
	}

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
	//todo2: use smth better than a memtree2 for everything ... ><
	d912pxy_ringbuffer<d912pxy_trimmed_pso_desc>* psoDescs = new d912pxy_ringbuffer<d912pxy_trimmed_pso_desc>(65535, 2);
	psoDescs->WriteElement(d912pxy_trimmed_pso_desc());
	
	{
		auto keyList = d912pxy_s.vfs.GetFileList(d912pxy_vfs_bid::pso_cache_keys);
		UINT keyIdx = 1;

		while (keyList->HaveElements())
		{

			auto keyName = keyList->PopElement();
			auto data = d912pxy_s.vfs.ReadFile(d912pxy_vfs_path(keyName, d912pxy_vfs_bid::pso_cache_keys));

			if (!data.isNullptr())
			{
				d912pxy_trimmed_pso_desc psoDesc;
				psoDesc.DeSerialize(data);
				data.Delete();

				psoDescs->WriteElement(psoDesc);

				auto psoHash = psoDesc.GetHash();
				cacheIndexes->PointAtMem(&psoHash, sizeof(psoHash));
				cacheIndexes->SetValue(keyIdx);
				++keyIdx;
			}
		}

		cacheIncID = keyIdx - 1;

		delete keyList;
	}

	{
		auto precompList = d912pxy_s.vfs.GetFileList(d912pxy_vfs_bid::pso_precompile_list);
		d912pxy_memtree2* shaderBuffer = new d912pxy_memtree2(sizeof(d912pxy_shader_uid), 100, 2);

		while (precompList->HaveElements())
		{
			auto cacheElement = d912pxy_vfs_path(precompList->PopElement(), d912pxy_vfs_bid::pso_precompile_list);
			d912pxy_shader_pair_cache_entry* pairEntry = d912pxy_s.vfs.ReadFile(cacheElement).c_arr<d912pxy_shader_pair_cache_entry>();

			if (!pairEntry)
				continue;

			cacheIndexes->PointAtMem(&pairEntry->pso, sizeof(d912pxy_trimmed_pso_desc_hash));

			//TODO: keep them alive in some temporary buffer
			shaderBuffer->PointAtMem(&pairEntry->vs, 8);
			d912pxy_shader* vs = (d912pxy_shader*)shaderBuffer->CurrentCID();
			if (!vs)
			{
				vs = d912pxy_shader::d912pxy_shader_com(PXY_SHADER_TYPE_VS, 0, pairEntry->vs);
				shaderBuffer->SetValue((UINT64)vs);
			}

			shaderBuffer->PointAtMem(&pairEntry->ps, 8);
			d912pxy_shader* ps = (d912pxy_shader*)shaderBuffer->CurrentCID();
			if (!ps)
			{
				ps = d912pxy_shader::d912pxy_shader_com(PXY_SHADER_TYPE_PS, 0, pairEntry->ps);
				shaderBuffer->SetValue((UINT64)ps);
			}

			if (!cacheIndexes->CurrentCID() || !vs->GetCode()->pShaderBytecode || !ps->GetCode()->pShaderBytecode)
			{
				LOG_ERR_DTDM("Can't precompile PSO with VS: %llX PS: %llX DSC KEY: %lX due to missing CSO/custom HLSL data/pso description", pairEntry->vs, pairEntry->ps, pairEntry->pso);
			}
			else {
				d912pxy_shader_pair* pair = d912pxy_s.render.db.shader.GetPair(vs, ps);

				auto psoDesc = psoDescs->GetElementOffsetPtr((UINT32)cacheIndexes->CurrentCID());

				psoDesc->ref.PS = ps;
				psoDesc->ref.VS = vs;

				pair->PrecompilePSO((UINT32)cacheIndexes->CurrentCID(), psoDesc);
			}

			PXY_FREE(pairEntry);
		}

		shaderBuffer->Begin();

		while (!shaderBuffer->IterEnd())
		{
			auto item = (d912pxy_shader*)shaderBuffer->CurrentCID();
			if (item)
				item->Release();

			shaderBuffer->Next();
		}

		delete shaderBuffer;
		delete precompList;
	}

	psoDescs->PopElement();
	while (psoDescs->HaveElements())
	{
		psoDescs->PopElement().ref.InputLayout->Release();		
	}
	delete psoDescs;
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

void d912pxy_pso_db::SaveKeyToCache(d912pxy_trimmed_pso_desc_hash key, d912pxy_trimmed_pso_desc* desc)
{
	d912pxy_mem_block data = desc->Serialize();

	d912pxy_s.vfs.WriteFile(d912pxy_vfs_path(key, d912pxy_vfs_bid::pso_cache_keys), data);
	data.Delete();
}

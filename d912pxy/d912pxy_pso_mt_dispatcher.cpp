/*
MIT License

Copyright(c) 2020 megai2

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
#include "d912pxy_pso_mt_dispatcher.h"

void d912pxy_pso_mt_dispatcher::Init()
{
	NonCom_Init(L"pso mt dispatcher");

	dxcThreads.clear();
	psoThreads.clear();

	uint32_t threads = d912pxy_s.config.GetValueUI32(PXY_CFG_MT_DXC_THREADS);
	if (threads == -1)
		threads = max(1, ((int32_t)d912pxy_s.dev.getCPUCoreCount() - 2)/2);//DXC threads are CPU heavy

	for (uint32_t i = 0; i < threads; ++i)
		dxcThreads.push_back(new DXCThread(dxcSubmissionLock));
		
	threads = d912pxy_s.config.GetValueUI32(PXY_CFG_MT_PSO_THREADS);
	if (threads == -1)
		threads = max(1, (int32_t)d912pxy_s.dev.getCPUCoreCount() - 2);

	for (uint32_t i = 0; i < threads; ++i)
		psoThreads.push_back(new PSOThread());
}

void d912pxy_pso_mt_dispatcher::UnInit()
{
	for (DXCThread* i : dxcThreads)
	{
		i->UnInit();
		delete i;
	}

	dxcThreads.clear();

	for (PSOThread* i : psoThreads)
	{
		i->UnInit();
		delete i;
	}
	psoThreads.clear();

	d912pxy_noncom::UnInit();
}

d912pxy_pso_mt_dispatcher::DXCThread* d912pxy_pso_mt_dispatcher::isAlreadyCompilingDerived(d912pxy_pso_item* item)
{
	for (DXCThread* i : dxcThreads)
	{
		if (strcmp(item->GetDerivedName(), i->getCurrentDerived()) == 0)
			return i;
	}

	return nullptr;
}

void d912pxy_pso_mt_dispatcher::queueCompilePSO(d912pxy_pso_item* item)
{
	if (!psoThreads.size())
	{
		item->PSOCompile();
		return;
	}

	psoThreads[rrIdxPSO]->enqueue(item);
	rrIdxPSO = (rrIdxPSO + 1) % psoThreads.size();
}

char* d912pxy_pso_mt_dispatcher::getQueueInfoStr()
{
	infoStr[0] = 0;

	char buf[128];

	strcat_s(infoStr, "DXC: ");

	if (!dxcThreads.size())
		strcat_s(infoStr, "n/a ");

	for (DXCThread* i : dxcThreads)
	{
		sprintf_s(buf, "%zu ", i->queueLength());
		strcat_s(infoStr, buf);
	}

	strcat_s(infoStr, "DX12: ");

	if (!psoThreads.size())
		strcat_s(infoStr, "n/a");

	for (PSOThread* i : psoThreads)
	{
		sprintf_s(buf, "%zu ", i->queueLength());
		strcat_s(infoStr, buf);
	}
	
	return infoStr;
}

size_t d912pxy_pso_mt_dispatcher::getTotalQueueLength()
{
	size_t ret = 0;

	for (DXCThread* i : dxcThreads)
		ret += i->queueLength();

	for (PSOThread* i : psoThreads)
		ret += i->queueLength();

	return ret;
}

void d912pxy_pso_mt_dispatcher::queueCompileDXC(d912pxy_pso_item* item)
{
	if (!dxcThreads.size())
	{
		item->DerivedCompile();
		return;
	}

	dxcThreads[rrIdxDXC]->enqueue(item);
	rrIdxDXC = (rrIdxDXC + 1) % dxcThreads.size();
}

d912pxy_pso_mt_dispatcher::CompilerThread::CompilerThread(const char* thrdName) : queue(256, 2)
{
	InitThread(thrdName, 0);
}

void d912pxy_pso_mt_dispatcher::CompilerThread::UnInit()
{
	Stop();
}

void d912pxy_pso_mt_dispatcher::CompilerThread::ThreadJob()
{
	while (queue.HaveElements())
	{
		d912pxy_pso_item* item = queue.PopElementMTG();

		CompileItem(item);
	}
}

void d912pxy_pso_mt_dispatcher::CompilerThread::enqueue(d912pxy_pso_item* item)
{	
	queue.WriteElementMT(item);
	SignalWork();
}

d912pxy_pso_mt_dispatcher::DXCThread::DXCThread(d912pxy::mt::sync::Lock& submission_lock)
	: CompilerThread("DXC compiler thread")
	, submissionLock(submission_lock)
{
	currentDerived[0] = 0;
}

void d912pxy_pso_mt_dispatcher::DXCThread::CompileItem(d912pxy_pso_item* item)
{
	{
		d912pxy::mt::sync::ScopedLock lock(submissionLock);
		DXCThread* anotherThread = d912pxy_s.render.db.psoMTCompiler.isAlreadyCompilingDerived(item);
		if (anotherThread)
		{
			anotherThread->enqueue(item);
			return;
		}

		//if DXC is not working on this item, we still have a chance that it was already finished
		if (item->RetryDerivedPresence())
			return;

		strcpy_s(currentDerived, item->GetDerivedName());
	}

	item->DerivedCompile();

	{
		d912pxy::mt::sync::ScopedLock lock(submissionLock);
		currentDerived[0] = 0;
	}
}

d912pxy_pso_mt_dispatcher::PSOThread::PSOThread()
	: CompilerThread("PSO compiler thread")
{}

void d912pxy_pso_mt_dispatcher::PSOThread::CompileItem(d912pxy_pso_item* item)
{
	item->PSOCompile();
}

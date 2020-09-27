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
#pragma once
#include "stdafx.h"

class d912pxy_pso_mt_dispatcher : public d912pxy_noncom
{
	class CompilerThread : public d912pxy_thread
	{
		d912pxy_ringbuffer<d912pxy_pso_item*> queue;
	public:
		CompilerThread(const char* thrdName);

		void UnInit();

		void ThreadJob();

		size_t queueLength() { return queue.TotalElements(); }

		virtual void CompileItem(d912pxy_pso_item*) { d912pxy::error::fatal(L"stub compiler method called!"); }

		void enqueue(d912pxy_pso_item* item);
	};

	class DXCThread : public CompilerThread
	{
		d912pxy::mt::sync::Lock& submissionLock;
		
		char currentDerived[64];
	public:
		DXCThread(d912pxy::mt::sync::Lock& submission_lock);
		void CompileItem(d912pxy_pso_item* item);

		char* getCurrentDerived() { return currentDerived; }
	};

	class PSOThread : public CompilerThread
	{
	public:
		PSOThread();

		void CompileItem(d912pxy_pso_item* item);
	};

	std::vector<DXCThread*> dxcThreads;
	std::vector<PSOThread*> psoThreads;
	uint32_t rrIdxDXC = 0;
	uint32_t rrIdxPSO = 0;

	static constexpr int infoStrLen = 4096;
	char infoStr[infoStrLen];

	d912pxy::mt::sync::Lock dxcSubmissionLock;
public:
	d912pxy_pso_mt_dispatcher() {}
	~d912pxy_pso_mt_dispatcher() {}

	void Init();
	void UnInit();

	DXCThread* isAlreadyCompilingDerived(d912pxy_pso_item* item);

	void queueCompileDXC(d912pxy_pso_item* item);
	void queueCompilePSO(d912pxy_pso_item* item);


	char* getQueueInfoStr();
	size_t getTotalQueueLength();
};

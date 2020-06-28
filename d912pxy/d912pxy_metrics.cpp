/*
MIT License

Copyright(c) 2019-2020 megai2

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
#include "d912pxy_metrics.h"

d912pxy_metrics::d912pxy_metrics()
{

}

d912pxy_metrics::~d912pxy_metrics()
{

}

void d912pxy_metrics::Init()
{
	NonCom_Init(L"metrics");

#ifdef ENABLE_METRICS
	LOG_INFO_DTDM("collecting frame and descriptor heap statistics, expect some performance drop");

	IP7_Client *l_pClient = P7_Get_Shared(TM("logger"));

	dheapMetrics = P7_Create_Telemetry(l_pClient, TM("dheap"));
	iframeMetrics = P7_Create_Telemetry(l_pClient, TM("iframe"));
	ohMetrics = P7_Create_Telemetry(l_pClient, TM("api overhead"));

	for (int i = 0; i != PXY_METRICS_IFRAME_COUNT; ++i)
	{
		iframeMetrics->Create(PXY_METRICS_IFRAME_TIME_NAMES[i], 0, 0, 60000, 30000, 1, &metricIFrameTimes[i]);
		iframeTime[i] = new Stopwatch();
	}

	for (int i = 0; i != PXY_INNER_MAX_DSC_HEAPS; ++i)
	{
		dheapMetrics->Create(PXY_METRICS_DHEAP_NAMES[i], 0, 0, d912pxy_dx12_heap_config[i].NumDescriptors, d912pxy_dx12_heap_config[i].NumDescriptors >> 1, 1, &metricDHeapSlots[i]);
	}

	int batchLimit = d912pxy_s.config.GetValueUI32(PXY_CFG_BATCHING_MAX_BATCHES_PER_IFRAME);

	iframeMetrics->Create(TM("counters / draws"), 0, 0, batchLimit, batchLimit / 2, 1, &metricIFrameDraws);
	iframeMetrics->Create(TM("counters / cleans"), 0, 0, batchLimit * 3, batchLimit, 1, &metricIFrameCleans);
	iframeMetrics->Create(TM("mem / pool / surf"), 0, 0, 1ULL << 12, 1ULL << 12, 1, &metricMemSurf);
	iframeMetrics->Create(TM("mem / pool / ul"), 0, 0, 1ULL << 10, 1ULL << 10, 1, &metricMemUl);
	iframeMetrics->Create(TM("mem / pool / vstream"), 0, 0, 1ULL << 10, 1ULL << 10, 1, &metricMemVStream);
	iframeMetrics->Create(TM("mem / ul / raw / buf"), 0, 0, 1ULL << 10, 1ULL << 10, 1, &metricMemUlFp[0]);
	iframeMetrics->Create(TM("mem / ul / raw / tex"), 0, 0, 1ULL << 10, 1ULL << 10, 1, &metricMemUlFp[1]);
	iframeMetrics->Create(TM("mem / ul / aligned / buf"), 0, 0, 1ULL << 10, 1ULL << 10, 1, &metricMemUlFp[2]);
	iframeMetrics->Create(TM("mem / ul / aligned / tex"), 0, 0, 1ULL << 10, 1ULL << 10, 1, &metricMemUlFp[3]);
	iframeMetrics->Create(TM("mem / heap"), 0, 0, 1024 * 5, 1024 * 15, 1, &metricMemHeap);
	iframeMetrics->Create(TM("mem / VA"), 0, 0, 1024 * 5, 1024 * 15, 1, &metricMemVA);
	iframeMetrics->Create(TM("mem / watched"), 0, 0, 65535, 0, 1, &metricMemWatched);
	iframeMetrics->Create(TM("derived / overhead"), 0, 0, 60000, 60000, 1, &metricOverhead);
	iframeMetrics->Create(TM("counters / GPUW depth"), 0, 0, 60000, 60000, 1, &metricGPUWDepth);
	iframeMetrics->Create(TM("counters / replay items"), 0, 0, 60000, 60000, 1, &metricReplayItems);

	iframeMetrics->Create(TM("mem / VRAM shared"), 0, 0, 1024 * 5, 1024 * 15, 1, &metricVRAMShared);
	iframeMetrics->Create(TM("mem / VRAM"), 0, 0, 1024 * 5, 1024 * 15, 1, &metricVRAM);

	iframeMetrics->Create(TM("derived / prep per batch"), 0, 0, 3000, 2000, 1, &metricIFramePerBatchPrep);
	iframeMetrics->Create(TM("derived / overhead per batch"), 0, 0, 3000, 2000, 1, &metricIFramePerBatchOverhead);
	iframeMetrics->Create(TM("derived / app prep"), 0, 0, 10000, 10000, 1, &metricIFrameAppPrep);

	iframeMetrics->Create(TM("thread / gpu"), 0, 0, 10000, 10000, 1, &metricGPUTime);

	for (int i = 0; i != MAX_OVERHEAD_STR_ID; ++i)
	{
		metricOH[i] = 65535;
	}

	l_pClient->Release();
	
#endif
}

void d912pxy_metrics::UnInit()
{
#ifdef ENABLE_METRICS
	FlushIFrameValues();

	for (int i = 0; i != PXY_METRICS_IFRAME_COUNT; ++i)
		delete iframeTime[i];

	dheapMetrics->Release();
	iframeMetrics->Release();
	ohMetrics->Release();
#endif

	d912pxy_noncom::UnInit();
}

#ifdef ENABLE_METRICS

void d912pxy_metrics::AddOverheadTime(UINT64 val, UINT id, const char* fun)
{	
	if (metricOH[id] == 65535)
	{
		static wchar_t buf[255];		
		wsprintf(buf, L"%S", fun);

		UINT len = lstrlenW(buf);

		int i = 0;
		while (i != len)
		{
			if (buf[i] == L'_')
			{
				buf[i] = L'/';
			}

			if (buf[i] == L':')
			{
				buf[i] = L'/';
				for (int j = i+1; j!=i+5;++j)
					buf[j] = L' ';

				break;
			}

			++i;
		}

		ohMetrics->Create(buf, 0, 0, 10000, 10000, 1, &metricOH[id]);
	} 

	metricOHval[id] += val;
}

void d912pxy_metrics::TrackIFrameTime(UINT start, UINT group)
{
	if (start)
		iframeTime[group]->Reset();
	else 
		iframeTime[group]->Stop();
}

void d912pxy_metrics::TrackDHeapSlots(UINT idx, UINT slots)
{
	dheapMetrics->AddU64(metricDHeapSlots[idx], slots);
}

void d912pxy_metrics::TrackGPUTime(UINT64 usTime)
{
	iframeMetrics->AddU64(metricGPUTime, usTime);
}

void d912pxy_metrics::TrackDrawCount(UINT draws)
{
	iframeMetrics->AddU64(metricIFrameDraws, draws);
	lastDraws = draws;
}

void d912pxy_metrics::TrackCleanupCount(UINT cleanups)
{
	iframeMetrics->AddU64(metricIFrameCleans, cleanups);
}

void d912pxy_metrics::TrackUploadMemUsage()
{	
	iframeMetrics->AddU64(metricMemUlFp[0], d912pxy_s.thread.bufld.GetMemFootprintMB());
	iframeMetrics->AddU64(metricMemUlFp[1], d912pxy_s.thread.texld.GetMemFootprintMB());
	iframeMetrics->AddU64(metricMemUlFp[2], d912pxy_s.thread.bufld.GetMemFootprintAlignedMB());
	iframeMetrics->AddU64(metricMemUlFp[3], d912pxy_s.thread.texld.GetMemFootprintAlignedMB());
}

void d912pxy_metrics::FlushIFrameValues()
{
	for (int i = 0; i != PXY_METRICS_IFRAME_COUNT; ++i)
	{
		if (i == PXY_METRICS_IFRAME_EXEC)
			iframeMetrics->AddU64(metricIFrameTimes[i], iframeTime[i]->GetStopTime() - iframeTime[PXY_METRICS_IFRAME_SYNC]->GetStopTime());
		else 
			iframeMetrics->AddU64(metricIFrameTimes[i], iframeTime[i]->GetStopTime());
	}


	UINT64 totalOH = 0;

	for (int i = 0; i != MAX_OVERHEAD_STR_ID; ++i)
	{
		if (metricOH[i] == 65535)
			continue;

		ohMetrics->AddU64(metricOH[i], metricOHval[i]);

		totalOH += metricOHval[i];

		metricOHval[i] = 0;
	}
	
	iframeMetrics->AddU64(metricOverhead, totalOH);
	iframeMetrics->AddU64(metricIFrameAppPrep,
		(iframeTime[PXY_METRICS_IFRAME_PREP]->GetStopTime() - totalOH 
			//megai2: we need to add gpu time and d912pxy sync time to correctly represent % of time spend on api calls in app on frame preparation
			+ iframeTime[PXY_METRICS_IFRAME_EXEC]->GetStopTime() + iframeTime[PXY_METRICS_IFRAME_SYNC]->GetStopTime() + iframeTime[PXY_METRICS_IFRAME_SYNC_WAKE]->GetStopTime()
		)*10000 / (iframeTime[PXY_METRICS_IFRAME_PREP]->GetStopTime() + 1));
	iframeMetrics->AddU64(metricIFramePerBatchPrep, iframeTime[PXY_METRICS_IFRAME_PREP]->GetStopTime() / (lastDraws + 1));
	iframeMetrics->AddU64(metricIFramePerBatchOverhead, totalOH / (lastDraws + 1));
	iframeMetrics->AddU64(metricMemHeap, d912pxy_s.mem.GetMemoryUsedMB());
	iframeMetrics->AddU64(metricMemVA, d912pxy_s.mem.GetVAMemoryUsedMB());
	iframeMetrics->AddU64(metricMemWatched, d912pxy_s.thread.cleanup.TotalWatchedItems());
	iframeMetrics->AddU64(metricMemVStream, d912pxy_s.pool.vstream.GetMemoryInPoolMb());
	iframeMetrics->AddU64(metricMemSurf, d912pxy_s.pool.surface.GetPoolSizeMB());
	iframeMetrics->AddU64(metricMemUl, d912pxy_s.pool.upload.GetMemoryInPoolMb());

	iframeMetrics->AddU64(metricVRAM,
		d912pxy_s.pool.surface.GetPoolSizeMB() +
		d912pxy_s.pool.vstream.GetMemoryInPoolMb() +
		(d912pxy_cbuffer::memUsage_V >> 20)		
	);

	iframeMetrics->AddU64(metricVRAMShared,
		d912pxy_s.pool.upload.GetMemoryInPoolMb() +
		(d912pxy_cbuffer::memUsage_UL >> 20)
	);

	
}

void d912pxy_metrics::TrackGPUWDepth(UINT64 depth)
{
	iframeMetrics->AddU64(metricGPUWDepth, depth);
}

void d912pxy_metrics::TrackReplayItems(UINT64 stackIdx)
{
	iframeMetrics->AddU64(metricReplayItems, stackIdx);
}

#endif

d912pxy_metrics_api_overhead_timer::~d912pxy_metrics_api_overhead_timer()
{
#ifdef ENABLE_METRICS
	UINT64 ela = timer.Elapsed();
	if (d912pxy_s.dev_vtable)
		d912pxy_s.log.metrics.AddOverheadTime(ela, id, fun);
#endif
}

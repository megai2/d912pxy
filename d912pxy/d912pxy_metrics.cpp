/*
MIT License

Copyright(c) 2019 megai2

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
#ifdef ENABLE_METRICS
	FlushIFrameValues();

	for (int i = 0; i != PXY_METRICS_IFRAME_COUNT; ++i)
		delete iframeTime[i];

	dheapMetrics->Release();
	iframeMetrics->Release();
#endif
}

void d912pxy_metrics::Init()
{
	NonCom_Init(L"metrics");

#ifdef ENABLE_METRICS
	LOG_INFO_DTDM("collecting frame and descriptor heap statistics, expect some performance drop");

	IP7_Client *l_pClient = P7_Get_Shared(TM("logger"));

	dheapMetrics = P7_Create_Telemetry(l_pClient, TM("dheap"));
	iframeMetrics = P7_Create_Telemetry(l_pClient, TM("iframe"));

	for (int i = 0; i != PXY_METRICS_IFRAME_COUNT; ++i)
	{
		iframeMetrics->Create(PXY_METRICS_IFRAME_TIME_NAMES[i], 0, 60000, 30000, 1, &metricIFrameTimes[i]);
		iframeTime[i] = new Stopwatch();
	}

	for (int i = 0; i != PXY_INNER_MAX_DSC_HEAPS; ++i)
	{
		dheapMetrics->Create(PXY_METRICS_DHEAP_NAMES[i], 0, d912pxy_dx12_heap_config[i].NumDescriptors, d912pxy_dx12_heap_config[i].NumDescriptors >> 1, 1, &metricDHeapSlots[i]);
	}

	iframeMetrics->Create(TM("counters / draws"), 0, PXY_INNER_MAX_IFRAME_BATCH_COUNT, PXY_INNER_MAX_IFRAME_BATCH_COUNT / 2, 1, &metricIFrameDraws);
	iframeMetrics->Create(TM("counters / cleans"), 0, PXY_INNER_MAX_IFRAME_BATCH_COUNT * 3, PXY_INNER_MAX_IFRAME_BATCH_COUNT, 1, &metricIFrameCleans);
	iframeMetrics->Create(TM("mem / pool / surf"), 0, 1ULL << 12, 1ULL << 12, 1, &metricMemSurf);
	iframeMetrics->Create(TM("mem / pool / ul"), 0, 1ULL << 10, 1ULL << 10, 1, &metricMemUl);
	iframeMetrics->Create(TM("mem / pool / vstream"), 0, 1ULL << 10, 1ULL << 10, 1, &metricMemVStream);
	iframeMetrics->Create(TM("mem / ul / raw / buf"), 0, 1ULL << 10, 1ULL << 10, 1, &metricMemUlFp[0]);
	iframeMetrics->Create(TM("mem / ul / raw / tex"), 0, 1ULL << 10, 1ULL << 10, 1, &metricMemUlFp[1]);
	iframeMetrics->Create(TM("mem / ul / aligned / buf"), 0, 1ULL << 10, 1ULL << 10, 1, &metricMemUlFp[2]);
	iframeMetrics->Create(TM("mem / ul / aligned / tex"), 0, 1ULL << 10, 1ULL << 10, 1, &metricMemUlFp[3]);
	iframeMetrics->Create(TM("mem / heap"), 0, 1024 * 5, 1024 * 15, 1, &metricMemHeap);
	iframeMetrics->Create(TM("mem / VA"), 0, 1024 * 5, 1024 * 15, 1, &metricMemVA);
	iframeMetrics->Create(TM("mem / watched"), 0, 65535, 0, 1, &metricMemWatched);

	iframeMetrics->Create(TM("derived / prep per batch"), 0, 3000, 2000, 1, &metricIFramePerBatchPrep);
	iframeMetrics->Create(TM("derived / overhead per batch"), 0, 3000, 2000, 1, &metricIFramePerBatchOverhead);
	iframeMetrics->Create(TM("derived / app prep"), 0, 10000, 10000, 1, &metricIFrameAppPrep);

	l_pClient->Release();
#endif
}

#ifdef ENABLE_METRICS

void d912pxy_metrics::TrackIFrameTime(UINT start, UINT group)
{
	if (start)
		iframeTime[group]->Reset();
	else 
		iframeTime[group]->Stop();
}

void d912pxy_metrics::TrackDHeapSlots(UINT idx, UINT slots)
{
	dheapMetrics->Add(metricDHeapSlots[idx], slots);
}

void d912pxy_metrics::TrackDrawCount(UINT draws)
{
	iframeMetrics->Add(metricIFrameDraws, draws);
	lastDraws = draws;
}

void d912pxy_metrics::TrackCleanupCount(UINT cleanups)
{
	iframeMetrics->Add(metricIFrameCleans, cleanups);
}

void d912pxy_metrics::TrackUploadMemUsage()
{
	iframeMetrics->Add(metricMemUl, d912pxy_s.pool.upload.GetMemoryInPoolMb());
	iframeMetrics->Add(metricMemUlFp[0], d912pxy_s.thread.bufld.GetMemFootprintMB());
	iframeMetrics->Add(metricMemUlFp[1], d912pxy_s.thread.texld.GetMemFootprintMB());
	iframeMetrics->Add(metricMemUlFp[2], d912pxy_s.thread.bufld.GetMemFootprintAlignedMB());
	iframeMetrics->Add(metricMemUlFp[3], d912pxy_s.thread.texld.GetMemFootprintAlignedMB());
}

void d912pxy_metrics::FlushIFrameValues()
{
	for (int i = 0; i != PXY_METRICS_IFRAME_COUNT; ++i)
	{
		if (i == PXY_METRICS_IFRAME_EXEC)
			iframeMetrics->Add(metricIFrameTimes[i], iframeTime[i]->GetStopTime() - iframeTime[PXY_METRICS_IFRAME_SYNC]->GetStopTime());
		else 
			iframeMetrics->Add(metricIFrameTimes[i], iframeTime[i]->GetStopTime());
	}

//	iframeMetrics->Add(metricIFrameAppPrep, (iframeTime[PXY_METRICS_IFRAME_PREP]->GetStopTime() - apiOverheadTotalTime[PXY_METRICS_API_OVERHEAD_COUNT])*10000 / (iframeTime[PXY_METRICS_IFRAME_PREP]->GetStopTime() + 1));
	iframeMetrics->Add(metricIFramePerBatchPrep, iframeTime[PXY_METRICS_IFRAME_PREP]->GetStopTime() / (lastDraws + 1));
//	iframeMetrics->Add(metricIFramePerBatchOverhead, apiOverheadTotalTime[PXY_METRICS_API_OVERHEAD_COUNT] / (lastDraws + 1));
	iframeMetrics->Add(metricMemHeap, d912pxy_s.mem.GetMemoryUsedMB());
	iframeMetrics->Add(metricMemVA, d912pxy_s.mem.GetVAMemoryUsedMB());
	iframeMetrics->Add(metricMemWatched, d912pxy_s.thread.cleanup.TotalWatchedItems());
	iframeMetrics->Add(metricMemVStream, d912pxy_s.pool.vstream.GetMemoryInPoolMb());
	iframeMetrics->Add(metricMemSurf, d912pxy_s.pool.surface.GetPoolSizeMB());
}

#endif
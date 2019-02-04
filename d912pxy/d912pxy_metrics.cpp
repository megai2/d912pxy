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

d912pxy_metrics::d912pxy_metrics(d912pxy_device * dev) : d912pxy_noncom(dev, L"metrics")
{
	d912pxy_s(metrics) = this;
	m_log->P7_INFO(LGC_DEFAULT, L"collecting frame and descriptor heap statistics, expect some performance drop");

	IP7_Client *l_pClient = P7_Get_Shared(TM("logger"));

	dheapMetrics = P7_Create_Telemetry(l_pClient, TM("dheap"));
	iframeMetrics = P7_Create_Telemetry(l_pClient, TM("iframe"));
	
	for (int i = 0; i != PXY_METRICS_IFRAME_COUNT; ++i)
	{
		iframeMetrics->Create(PXY_METRICS_IFRAME_TIME_NAMES[i], 0, 60000, 30000, 1, &metricIFrameTimes[i]);
		iframeTime[i] = new Stopwatch();
	}

	for (int i = 0; i != PXY_METRICS_API_OVERHEAD_COUNT + 1; ++i)
	{
		iframeMetrics->Create(PXY_METRICS_API_OVERHEAD_NAMES[i], 0, 15000, 5000, 1, &metricIFrameAPIOverhead[i]);
		apiOverheadTime[i] = new Stopwatch();
		apiOverheadTotalTime[i] = 0;
	}

	for (int i = 0; i != PXY_INNER_MAX_DSC_HEAPS; ++i)
	{
		dheapMetrics->Create(PXY_METRICS_DHEAP_NAMES[i], 0, d912pxy_dx12_heap_config[i].NumDescriptors, d912pxy_dx12_heap_config[i].NumDescriptors >> 1, 1, &metricDHeapSlots[i]);
	}

	iframeMetrics->Create(TM("counters / draws"), 0, PXY_INNER_MAX_IFRAME_BATCH_COUNT, PXY_INNER_MAX_IFRAME_BATCH_COUNT / 2, 1, &metricIFrameDraws);
	iframeMetrics->Create(TM("counters / cleans"), 0, PXY_INNER_MAX_IFRAME_BATCH_COUNT*3, PXY_INNER_MAX_IFRAME_BATCH_COUNT, 1, &metricIFrameCleans);

	iframeMetrics->Create(TM("derived / prep per batch"), 0, 3000, 2000, 1, &metricIFramePerBatchPrep);
	iframeMetrics->Create(TM("derived / overhead per batch"), 0, 3000, 2000, 1, &metricIFramePerBatchOverhead);
	iframeMetrics->Create(TM("derived / app prep"), 0, 10000, 10000, 1, &metricIFrameAppPrep);

	l_pClient->Release();

}

d912pxy_metrics::~d912pxy_metrics()
{
	for (int i = 0; i != PXY_METRICS_API_OVERHEAD_COUNT + 1; ++i)
		delete apiOverheadTime[i];

	for (int i = 0; i != PXY_METRICS_IFRAME_COUNT; ++i)
		delete iframeTime[i];

	dheapMetrics->Release();
	iframeMetrics->Release();
}

void d912pxy_metrics::TrackAPIOverheadStart(UINT group)
{
	apiOverheadTime[group]->Reset();
}

void d912pxy_metrics::TrackAPIOverheadEnd(UINT group)
{
	apiOverheadTotalTime[group] += apiOverheadTime[group]->Elapsed().count();
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

void d912pxy_metrics::FlushIFrameValues()
{
	apiOverheadTotalTime[PXY_METRICS_API_OVERHEAD_COUNT] = 0;

	for (int i = 0; i != PXY_METRICS_API_OVERHEAD_COUNT; ++i)
	{
		UINT64 tmp = apiOverheadTotalTime[i];
		apiOverheadTotalTime[i] = 0;
		iframeMetrics->Add(metricIFrameAPIOverhead[i], tmp);
		apiOverheadTotalTime[PXY_METRICS_API_OVERHEAD_COUNT] += tmp;
	}

	iframeMetrics->Add(metricIFrameAPIOverhead[PXY_METRICS_API_OVERHEAD_COUNT], apiOverheadTotalTime[PXY_METRICS_API_OVERHEAD_COUNT]);

	for (int i = 0; i != PXY_METRICS_IFRAME_COUNT; ++i)
	{
		if (i == PXY_METRICS_IFRAME_EXEC)
			iframeMetrics->Add(metricIFrameTimes[i], iframeTime[i]->GetStopTime().count() - iframeTime[PXY_METRICS_IFRAME_SYNC]->GetStopTime().count());
		else 
			iframeMetrics->Add(metricIFrameTimes[i], iframeTime[i]->GetStopTime().count());
	}

	iframeMetrics->Add(metricIFrameAppPrep, (iframeTime[PXY_METRICS_IFRAME_PREP]->GetStopTime().count() - apiOverheadTotalTime[PXY_METRICS_API_OVERHEAD_COUNT])*10000 / (iframeTime[PXY_METRICS_IFRAME_PREP]->GetStopTime().count() + 1));
	iframeMetrics->Add(metricIFramePerBatchPrep, iframeTime[PXY_METRICS_IFRAME_PREP]->GetStopTime().count() / (lastDraws + 1));
	iframeMetrics->Add(metricIFramePerBatchOverhead, apiOverheadTotalTime[PXY_METRICS_API_OVERHEAD_COUNT] / (lastDraws + 1));
}

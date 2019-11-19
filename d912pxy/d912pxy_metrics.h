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
#pragma once
#include "stdafx.h"

#define PXY_METRICS_IFRAME_PREP 0
#define PXY_METRICS_IFRAME_EXEC 1
#define PXY_METRICS_IFRAME_SYNC 2
#define PXY_METRICS_IFRAME_THREAD_TEX 3
#define PXY_METRICS_IFRAME_THREAD_BUF 4
#define PXY_METRICS_IFRAME_THREAD_RP0 5
#define PXY_METRICS_IFRAME_THREAD_RP1 6
#define PXY_METRICS_IFRAME_THREAD_RP2 7
#define PXY_METRICS_IFRAME_THREAD_RP3 8
#define PXY_METRICS_IFRAME_SYNC_WAKE 9
#define PXY_METRICS_IFRAME_RESIDENCY 10
#define PXY_METRICS_IFRAME_COUNT 11

static const wchar_t* PXY_METRICS_IFRAME_TIME_NAMES [] = {
	L"total / prep",
	L"total / exec",
	L"total / sync",
	L"thread / tex",
	L"thread / buf",
	L"thread / rp0",
	L"thread / rp1",
	L"thread / rp2",
	L"thread / rp3",
	L"total / sync wake",
	L"total / residency"	
};

static const wchar_t* PXY_METRICS_DHEAP_NAMES[] = {
	L"slots / rtv",
	L"slots / dsv",
	L"slots / srv",
	L"slots / spl"
};

class d912pxy_metrics_api_overhead_timer {

public:
	d912pxy_metrics_api_overhead_timer(UINT str, const char* funName) { id = str; fun = funName; }
	~d912pxy_metrics_api_overhead_timer();

private:
	Stopwatch timer;
	const char* fun;
	UINT id;

};

#define MAX_OVERHEAD_STR_ID 65535

class d912pxy_metrics : public d912pxy_noncom
{
public:
	d912pxy_metrics();
	~d912pxy_metrics();

	void Init();
	void UnInit();
	   
#ifdef ENABLE_METRICS

	void AddOverheadTime(UINT64 val, UINT id, const char* fun);

	void TrackIFrameTime(UINT start, UINT group);
	void TrackDHeapSlots(UINT idx, UINT slots);

	void TrackGPUTime(UINT64 usTime);
	void TrackDrawCount(UINT draws);
	void TrackCleanupCount(UINT cleanups);
	void TrackUploadMemUsage();

	void FlushIFrameValues();

	void TrackGPUWDepth(UINT64 depth);
	void TrackReplayItems(UINT64 stackIdx);

private:
	IP7_Telemetry* iframeMetrics;	
	tUINT16 metricIFrameTimes[PXY_METRICS_IFRAME_COUNT];	
	tUINT16 metricIFrameDraws;
	tUINT16 metricIFrameCleans;	
	tUINT16 metricIFramePerBatchPrep;
	tUINT16 metricIFramePerBatchOverhead;
	tUINT16 metricIFrameAppPrep;		

	tUINT16 metricMemVA;
	tUINT16 metricMemHeap;
	tUINT16 metricMemSurf;
	tUINT16 metricMemUl;
	tUINT16 metricMemUlFp[4];
	tUINT16 metricMemWatched;
	tUINT16 metricMemVStream;
	tUINT16 metricOverhead;
	tUINT16 metricGPUWDepth;
	tUINT16 metricReplayItems;

	tUINT16 metricVRAM;
	tUINT16 metricVRAMShared;
	tUINT16 metricGPUTime;

	IP7_Telemetry* ohMetrics;
	UINT64 metricOHval[MAX_OVERHEAD_STR_ID];
	tUINT16 metricOH[MAX_OVERHEAD_STR_ID];

	IP7_Telemetry* dheapMetrics;
	tUINT16 metricDHeapSlots[PXY_INNER_MAX_DSC_HEAPS];
	
	Stopwatch* iframeTime[PXY_METRICS_IFRAME_COUNT];	
	
	UINT lastDraws;	
#endif
};


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



class d912pxy_metrics : public d912pxy_noncom
{
public:
	d912pxy_metrics();
	~d912pxy_metrics();

	void Init();
	   
#ifndef DISABLE_P7LIB

	void TrackIFrameTime(UINT start, UINT group);
	void TrackDHeapSlots(UINT idx, UINT slots);

	void TrackDrawCount(UINT draws);
	void TrackCleanupCount(UINT cleanups);
	void TrackUploadMemUsage();

	void FlushIFrameValues();

private:
	IP7_Telemetry* iframeMetrics;	
	tUINT8 metricIFrameTimes[PXY_METRICS_IFRAME_COUNT];	
	tUINT8 metricIFrameDraws;
	tUINT8 metricIFrameCleans;	
	tUINT8 metricIFramePerBatchPrep;
	tUINT8 metricIFramePerBatchOverhead;
	tUINT8 metricIFrameAppPrep;	

	tUINT8 metricMemVA;
	tUINT8 metricMemHeap;
	tUINT8 metricMemSurf;
	tUINT8 metricMemUl;
	tUINT8 metricMemUlFp[4];
	tUINT8 metricMemWatched;
	tUINT8 metricMemVStream;

	IP7_Telemetry* dheapMetrics;
	tUINT8 metricDHeapSlots[PXY_INNER_MAX_DSC_HEAPS];
	
	Stopwatch* iframeTime[PXY_METRICS_IFRAME_COUNT];	
	
	UINT lastDraws;
#endif
};


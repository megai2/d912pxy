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

#define PXY_METRICS_API_OVERHEAD_DEVICE 0
#define PXY_METRICS_API_OVERHEAD_DEVICE_DRAWING 1
#define PXY_METRICS_API_OVERHEAD_DEVICE_CLIPPING 2
#define PXY_METRICS_API_OVERHEAD_DEVICE_CONSTRUCTORS 3
#define PXY_METRICS_API_OVERHEAD_DEVICE_PSO 4
#define PXY_METRICS_API_OVERHEAD_DEVICE_SHADERS 5
#define PXY_METRICS_API_OVERHEAD_DEVICE_STREAMS 6
#define PXY_METRICS_API_OVERHEAD_DEVICE_SURFACE 7
#define PXY_METRICS_API_OVERHEAD_DEVICE_SWAPCHAIN 8
#define PXY_METRICS_API_OVERHEAD_DEVICE_TEXSTATE 9
#define PXY_METRICS_API_OVERHEAD_VSTREAM 10
#define PXY_METRICS_API_OVERHEAD_TEXTURE 11
#define PXY_METRICS_API_OVERHEAD_SURFACE 12
#define PXY_METRICS_API_OVERHEAD_COM 13
#define PXY_METRICS_API_OVERHEAD_DEVICE_DRAWING_UP 14
#define PXY_METRICS_API_OVERHEAD_QUERY_OCCLUSION 15
#define PXY_METRICS_API_OVERHEAD_COUNT 16

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

static const wchar_t* PXY_METRICS_API_OVERHEAD_NAMES[] = {
	L"overhead / dev",
	L"overhead / dev / draw",
	L"overhead / dev / clip",
	L"overhead / dev / ctrs",
	L"overhead / dev / pso",
	L"overhead / dev / shd",
	L"overhead / dev / strm",
	L"overhead / dev / surf",
	L"overhead / dev / swpc",
	L"overhead / dev / texs",
	L"overhead / vstream",
	L"overhead / texture",
	L"overhead / surface",
	L"overhead / com",
	L"overhead / dev / dup",
	L"overhead / query / occlusion",
	L"overhead / total"
};

static const wchar_t* PXY_METRICS_IFRAME_TIME_NAMES [] = {
	L"time / prep",
	L"time / exec",
	L"time / sync",
	L"time / thread / tex",
	L"time / thread / buf",
	L"time / thread / rp0",
	L"time / thread / rp1",
	L"time / thread / rp2",
	L"time / thread / rp3",
	L"time / sync wake",
	L"time / residency"
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
	void TrackAPIOverheadStart(UINT group);
	void TrackAPIOverheadEnd(UINT group);

	void TrackIFrameTime(UINT start, UINT group);
	void TrackDHeapSlots(UINT idx, UINT slots);

	void TrackDrawCount(UINT draws);
	void TrackCleanupCount(UINT cleanups);
	void TrackUploadPoolUsage(UINT64 usage);

	void FlushIFrameValues();

private:
	IP7_Telemetry* iframeMetrics;	
	tUINT8 metricIFrameTimes[PXY_METRICS_IFRAME_COUNT];
	tUINT8 metricIFrameAPIOverhead[PXY_METRICS_API_OVERHEAD_COUNT + 1];
	tUINT8 metricIFrameDraws;
	tUINT8 metricIFrameCleans;
	tUINT8 metricIFrameUploadOffset;
	tUINT8 metricTotalMemUsed;
	tUINT8 metricIFramePerBatchPrep;
	tUINT8 metricIFramePerBatchOverhead;
	tUINT8 metricIFrameAppPrep;	

	IP7_Telemetry* dheapMetrics;
	tUINT8 metricDHeapSlots[PXY_INNER_MAX_DSC_HEAPS];
	
	Stopwatch* iframeTime[PXY_METRICS_IFRAME_COUNT];
	Stopwatch* apiOverheadTime[PXY_METRICS_API_OVERHEAD_COUNT+1];
	UINT64 apiOverheadTotalTime[PXY_METRICS_API_OVERHEAD_COUNT+1];	

	UINT lastDraws;
#endif
};


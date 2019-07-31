/*
MIT License

Copyright(c) 2018-2019 megai2

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

typedef enum d912pxy_replay_item_type {
	DRPL_TRAN = 0,
	DRPL_OMSR = 1,
	DRPL_OMBF,//2
	DRPL_RSVP,//3
	DRPL_RSSR,//4
	DRPL_DIIP,//5
	DRPL_OMRT,//6
	DRPL_IFVB,//7
	DRPL_IFIB,//8
	DRPL_RCLR,//9
	DRPL_DCLR,//10
	DRPL_RPSO,//11
	DRPL_RPSF,//12
	DRPL_CPSO,//13
	DRPL_RECT,//14
	DRPL_GPUW,//15
	DRPL_PRMT,//16
	DRPL_QUMA,//17
	DRPL_COUNT//18
} d912pxy_replay_item_type;

static const wchar_t* d912pxy_replay_item_type_dsc[] = {
	L"DRPL_TRAN",
	L"DRPL_OMSR",
	L"DRPL_OMBF",
	L"DRPL_RSVP",
	L"DRPL_RSSR",
	L"DRPL_DIIP",
	L"DRPL_OMRT",
	L"DRPL_IFVB",
	L"DRPL_IFIB",
	L"DRPL_RCLR",
	L"DRPL_DCLR",
	L"DRPL_RPSO",
	L"DRPL_RPSF",
	L"DRPL_CPSO",
	L"DRPL_RECT",
	L"DRPL_GPUW",
	L"DRPL_PRMT",
	L"DRPL_QUMA"
};

typedef struct d912pxy_replay_state_transit {
	d912pxy_resource* res;
	D3D12_RESOURCE_STATES to;
	D3D12_RESOURCE_STATES from;
} d912pxy_replay_state_transit;

typedef struct d912pxy_replay_pso {
	d912pxy_pso_cache_item* cachedPSO;
} d912pxy_replay_pso;

typedef struct d912pxy_replay_om_sr {
	DWORD dRef;	
} d912pxy_replay_om_sr;

typedef struct d912pxy_replay_om_bf {
	float color[4];
} d912pxy_replay_om_bf;

typedef struct d912pxy_replay_rs_viewscissor {
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissor;
} d912pxy_replay_rs_viewscissor;

typedef struct d912pxy_replay_draw_indexed_instanced {
	UINT IndexCountPerInstance;
	UINT InstanceCount;
	UINT StartIndexLocation;
	INT BaseVertexLocation;
	UINT StartInstanceLocation;
	UINT batchId;
} d912pxy_replay_draw_indexed_instanced;

typedef struct d912pxy_replay_om_render_target {
	d912pxy_surface* rtv;
	d912pxy_surface* dsv;
} d912pxy_replay_om_render_target;

typedef struct d912pxy_replay_vbuf_bind {
	d912pxy_vstream* buf;
	UINT stride;
	UINT slot;
	UINT offset;
} d912pxy_replay_vbuf_bind;

typedef struct d912pxy_replay_ibuf_bind {
	d912pxy_vstream* buf;
} d912pxy_replay_ibuf_bind;

typedef struct d912pxy_replay_clear_rt {
	d912pxy_surface* tgt;
	float clr[4];
	D3D12_RECT clearRect;
} d912pxy_replay_clear_rt;

typedef struct d912pxy_replay_clear_ds {
	d912pxy_surface* tgt;
	float depth;
	UINT8 stencil;
	D3D12_CLEAR_FLAGS flag;
	D3D12_RECT clearRect;
} d912pxy_replay_clear_ds;

typedef struct d912pxy_replay_pso_raw {
	d912pxy_trimmed_dx12_pso rawState;
} d912pxy_replay_pso_raw;

typedef struct d912pxy_replay_pso_raw_feedback {
	d912pxy_trimmed_dx12_pso rawState;
	void** feedbackPtr;
} d912pxy_replay_pso_raw_feedback;

typedef struct d912pxy_replay_rect {
	d912pxy_surface* src;
	d912pxy_surface* dst;
	D3D12_RESOURCE_STATES prevS;
	D3D12_RESOURCE_STATES prevD;
} d912pxy_replay_rect;

typedef struct d912pxy_replay_pso_compiled {
	d912pxy_pso_cache_item* psoItem;
} d912pxy_replay_pso_compiled;

typedef struct d912pxy_replay_gpu_write_control {
	UINT32 streamIdx;
	UINT16 offset;
	UINT16 size;
	UINT16 bn;
} d912pxy_replay_gpu_write_control;

typedef struct d912pxy_replay_primitive_topology {
	UINT8 newTopo;
} d912pxy_replay_primitive_topology;

typedef struct d912pxy_replay_query_mark {
	d912pxy_query* obj;
	UINT8 start;
} d912pxy_replay_query_mark;

typedef struct d912pxy_replay_item {
	d912pxy_replay_item_type type;
	union {
		d912pxy_replay_state_transit transit;
		d912pxy_replay_om_sr omsr;
		d912pxy_replay_om_bf ombf;
		d912pxy_replay_rs_viewscissor rs;
		d912pxy_replay_draw_indexed_instanced dip;
		d912pxy_replay_om_render_target rt;
		d912pxy_replay_vbuf_bind vb;
		d912pxy_replay_ibuf_bind ib;		
		d912pxy_replay_clear_ds clrDs;
		d912pxy_replay_clear_rt clrRt;
		d912pxy_replay_pso_raw rawPso;
		d912pxy_replay_pso_compiled compiledPso;
		d912pxy_replay_pso_raw_feedback rawPsoFeedback;
		d912pxy_replay_rect srect;				
		d912pxy_replay_gpu_write_control gpuw_ctl;
		d912pxy_replay_primitive_topology topo;
		d912pxy_replay_query_mark queryMark;
		UINT64 ptr;
	};
} d912pxy_replay_item;

typedef struct d912pxy_replay_thread_transit_data {
	d912pxy_surface* surfBind[2];
	DWORD srefVal;
	DWORD bfacVal;
	
	d912pxy_device_streamsrc streams[PXY_INNER_MAX_VBUF_STREAMS];
	d912pxy_vstream* indexBuf;

	d912pxy_trimmed_dx12_pso pso;
	d912pxy_pso_cache_item* cpso;

	UINT saved;

	D3D12_VIEWPORT main_viewport;
	D3D12_RECT main_scissor;
	
} d912pxy_replay_thread_transit_data;

class d912pxy_replay;

typedef void (d912pxy_replay::*d912pxy_replay_handler_func)(void*, ID3D12GraphicsCommandList* cl, void**);

class d912pxy_replay : public d912pxy_replay_base	
{
public:
	d912pxy_replay();
	~d912pxy_replay();

	void Init();

	//actions

	UINT StateTransit(d912pxy_resource* res, D3D12_RESOURCE_STATES to);
	void OMStencilRef(DWORD ref);
	void OMBlendFac(float* color);
	void RSViewScissor(D3D12_VIEWPORT viewport, D3D12_RECT scissor);
	
	void PSOCompiled(d912pxy_pso_cache_item* dsc);
	void PSORaw(d912pxy_trimmed_dx12_pso* dsc);
	void PSORawFeedback(d912pxy_trimmed_dx12_pso* dsc, void** ptr);
	void VBbind(d912pxy_vstream* buf, UINT stride,	UINT slot, UINT offset);
	void IBbind(d912pxy_vstream* buf);
	void DIIP(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation, UINT batchId);
	
	void RT(d912pxy_surface* rtv, d912pxy_surface* dsv);
	void RTClear(d912pxy_surface* tgt, float* clr, D3D12_VIEWPORT* currentVWP);
	void DSClear(d912pxy_surface* tgt, float depth, UINT8 stencil, D3D12_CLEAR_FLAGS flag, D3D12_VIEWPORT* currentVWP);
	void StretchRect(d912pxy_surface* src, d912pxy_surface* dst);
	void GPUW(UINT32 si, UINT16 of, UINT16 cnt, UINT16 bn);

	void QueryMark(d912pxy_query* va, UINT start);

	void PrimTopo(D3DPRIMITIVETYPE primType);

	//actual execute code and thread managment

	void PlayId(d912pxy_replay_item* it, ID3D12GraphicsCommandList* cl, void** context);
	void Replay(UINT start, UINT end, ID3D12GraphicsCommandList * cl, d912pxy_replay_thread* thrd);
	UINT WaitForData(UINT idx, UINT maxRI, UINT end, d912pxy_replay_thread* thrd);

	void Finish();
	void Start();
	void IFrameStart();
	void IssueWork(UINT batch);
	void ReRangeThreads(UINT batches);

	void Free();

	UINT GetStackTop();
	void SyncStackTop();

	d912pxy_replay_item* GetItem(UINT id) { return &stack[id]; };
	d912pxy_replay_item* BacktraceItemType(d912pxy_replay_item_type type, UINT depth, UINT base);

	void TransitBacktrace(d912pxy_replay_item_type type, UINT depth, ID3D12GraphicsCommandList* cl, UINT base, void** context);

	void TransitCLState(ID3D12GraphicsCommandList* cl, UINT base, UINT thread, void** context);
	void SaveCLState(UINT thread);

#ifdef _DEBUG
	UINT DbgStackGet();
	void DbgStackIncrement();
	UINT DbgStackIgnore();
#endif
	
private:
	d912pxy_replay_handler_func replay_handlers[DRPL_COUNT];

	void RHA_TRAN(d912pxy_replay_state_transit* it, ID3D12GraphicsCommandList * cl, void** unused);
	void RHA_OMSR(d912pxy_replay_om_sr* it, ID3D12GraphicsCommandList * cl, void** unused);
	void RHA_OMBF(d912pxy_replay_om_bf* it, ID3D12GraphicsCommandList * cl, void** unused);
	void RHA_RSVP(d912pxy_replay_rs_viewscissor* it, ID3D12GraphicsCommandList * cl, void** unused);
	void RHA_RSSR(d912pxy_replay_rs_viewscissor* it, ID3D12GraphicsCommandList * cl, void** unused);
	void RHA_DIIP(d912pxy_replay_draw_indexed_instanced* it, ID3D12GraphicsCommandList * cl, ID3D12PipelineState** context);
	void RHA_OMRT(d912pxy_replay_om_render_target* it, ID3D12GraphicsCommandList * cl, void** unused);
	void RHA_IFVB(d912pxy_replay_vbuf_bind* it, ID3D12GraphicsCommandList * cl, void** unused);
	void RHA_IFIB(d912pxy_replay_ibuf_bind* it, ID3D12GraphicsCommandList * cl, void** unused);
	void RHA_RCLR(d912pxy_replay_clear_rt* it, ID3D12GraphicsCommandList * cl, void** unused);
	void RHA_DCLR(d912pxy_replay_clear_ds* it, ID3D12GraphicsCommandList * cl, void** unused);
	void RHA_RPSO(d912pxy_replay_pso_raw* it, ID3D12GraphicsCommandList * cl, ID3D12PipelineState** context);
	void RHA_CPSO(d912pxy_replay_pso_compiled* it, ID3D12GraphicsCommandList * cl, ID3D12PipelineState** context);
	void RHA_RPSF(d912pxy_replay_pso_raw_feedback* it, ID3D12GraphicsCommandList * cl, void** unused);
	void RHA_RECT(d912pxy_replay_rect* it, ID3D12GraphicsCommandList * cl, void** unused);
	void RHA_GPUW(d912pxy_replay_gpu_write_control* it, ID3D12GraphicsCommandList * cl, void** unused);
	void RHA_GPUW_MT(d912pxy_replay_gpu_write_control* it, ID3D12GraphicsCommandList * cl, void** unused);
	void RHA_PRMT(d912pxy_replay_primitive_topology* it, ID3D12GraphicsCommandList * cl, void** unused);
	void RHA_QUMA(d912pxy_replay_query_mark* it, ID3D12GraphicsCommandList * cl, void** unused);
	
	d912pxy_replay_item* stack;

	UINT stackTop;
	LONG stackTopMT;
	UINT maxReplayItems;
	
	UINT lastSRefStk;
	UINT lastBFactorStk;
		
	UINT switchRange;	
	UINT switchPoint;
	UINT rangeEnds[PXY_INNER_REPLAY_THREADS_MAX];
	UINT cWorker;
	UINT numThreads;

	d912pxy_replay_thread_transit_data transitData[PXY_INNER_REPLAY_THREADS_MAX];

	d912pxy_replay_thread* threads[PXY_INNER_REPLAY_THREADS_MAX];
	LONG stopMarker;

#ifdef _DEBUG
	d912pxy_thread_lock simThreadAcc;
#endif
};


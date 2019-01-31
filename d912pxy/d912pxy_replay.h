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
	DRPL_OMSR,
	DRPL_OMBF,
	DRPL_RSVP,
	DRPL_RSSR,
	DRPL_DIIP,
	DRPL_OMRT,
	DRPL_IFVB,
	DRPL_IFIB,
	DRPL_RCLR,
	DRPL_DCLR,
	DRPL_RPSO,
	DRPL_RPSF,
	DRPL_CPSO,
	DRPL_RECT,
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
	L"DRPL_RECT"
};

typedef struct d912pxy_replay_view_transit {
	d912pxy_surface* res;
	D3D12_RESOURCE_STATES to;
	D3D12_RESOURCE_STATES from;
} d912pxy_replay_view_transit;

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
} d912pxy_replay_draw_indexed_instanced;

typedef struct d912pxy_replay_om_render_target {
	d912pxy_surface* rtv;
	d912pxy_surface* dsv;
} d912pxy_replay_om_render_target;

typedef struct d912pxy_replay_vbuf_bind {
	d912pxy_vbuf* buf;
	UINT stride;
	UINT slot;
	UINT offset;
} d912pxy_replay_vbuf_bind;

typedef struct d912pxy_replay_ibuf_bind {
	d912pxy_ibuf* buf;
} d912pxy_replay_ibuf_bind;

typedef struct d912pxy_replay_rsig {
	UINT64 texAdr;
	UINT64 varAdr;
} d912pxy_replay_rsig;

typedef struct d912pxy_replay_clear_rt {
	d912pxy_surface* tgt;
	float clr[4];
} d912pxy_replay_clear_rt;

typedef struct d912pxy_replay_clear_ds {
	d912pxy_surface* tgt;
	float depth;
	UINT8 stencil;
	D3D12_CLEAR_FLAGS flag;
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
} d912pxy_replay_rect;

typedef struct d912pxy_replay_gpu_data_dlt {
	UINT dstOf;
	intptr_t srcOf;
	UINT size;
} d912pxy_replay_gpu_data_dlt;

typedef struct d912pxy_replay_cs_copy_data {
	intptr_t srcStreamBind;	
	intptr_t parStreamBind;
} d912pxy_replay_cs_copy_data;

typedef struct d912pxy_replay_pso_compiled {
	d912pxy_pso_cache_item* psoItem;
} d912pxy_replay_pso_compiled;

typedef struct d912pxy_replay_item {
	d912pxy_replay_item_type type;
	union {
		d912pxy_replay_view_transit transit;
		d912pxy_replay_om_sr omsr;
		d912pxy_replay_om_bf ombf;
		d912pxy_replay_rs_viewscissor rs;
		d912pxy_replay_draw_indexed_instanced dip;
		d912pxy_replay_om_render_target rt;
		d912pxy_replay_vbuf_bind vb;
		d912pxy_replay_ibuf_bind ib;
		d912pxy_replay_rsig rsig;
		d912pxy_replay_clear_ds clrDs;
		d912pxy_replay_clear_rt clrRt;
		d912pxy_replay_pso_raw rawPso;
		d912pxy_replay_pso_compiled compiledPso;
		d912pxy_replay_pso_raw_feedback rawPsoFeedback;
		d912pxy_replay_rect srect;
		d912pxy_replay_gpu_data_dlt gpuddl;
		d912pxy_replay_cs_copy_data cscp;
	};
} d912pxy_replay_item;

#define D912PXY_REPLAY_HA_DECL(rtype,name, ptname) rtype RHA_##name (ptname* it, ID3D12GraphicsCommandList * cl)
#define D912PXY_REPLAY_HA_IMPL(rtype,name, ptname) rtype d912pxy_replay::RHA_##name (ptname* it, ID3D12GraphicsCommandList * cl)

//#define PXY_REPLAY_THERAD_IMPL_PASSTHRU

class d912pxy_replay :
	public d912pxy_noncom
{
public:
	d912pxy_replay(d912pxy_device* dev);
	~d912pxy_replay();

	UINT ViewTransit(d912pxy_surface* res, D3D12_RESOURCE_STATES to);
	void PSOCompiled(d912pxy_pso_cache_item* dsc);
	void PSORaw(d912pxy_trimmed_dx12_pso* dsc);
	void PSORawFeedback(d912pxy_trimmed_dx12_pso* dsc, void** ptr);
	void OMStencilRef(DWORD ref);
	void OMBlendFac(float* color);
	void RSViewScissor(D3D12_VIEWPORT viewport, D3D12_RECT scissor);
	void DIIP(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
	void RT(d912pxy_surface* rtv, d912pxy_surface* dsv);
	void VBbind(d912pxy_vbuf* buf, UINT stride,	UINT slot, UINT offset);
	void IBbind(d912pxy_ibuf* buf);
	void StretchRect(d912pxy_surface* src, d912pxy_surface* dst);

	void RTClear(d912pxy_surface* tgt, float* clr);
	void DSClear(d912pxy_surface* tgt, float depth, UINT8 stencil, D3D12_CLEAR_FLAGS flag);

	void PlayId(UINT id, ID3D12GraphicsCommandList* cl);

	void Replay(UINT start, UINT end, d912pxy_gpu_cmd_list_group listGrp, d912pxy_replay_thread* thrd);

	void Finish();

	void IssueWork(UINT batch);
	void ReRangeThreads(UINT batches);

	void Start();

	d912pxy_replay_item* GetItem(UINT id) { return &stack[id]; };

	d912pxy_replay_item* BacktraceItemType(d912pxy_replay_item_type type, UINT depth);

private:

#ifdef PXY_REPLAY_THERAD_IMPL_PASSTHRU
	ID3D12GraphicsCommandList * cl;
	ID3D12PipelineState* psoPtr;
#endif


	D912PXY_REPLAY_HA_DECL(void, TRAN, d912pxy_replay_view_transit);	
	D912PXY_REPLAY_HA_DECL(void, PSO, d912pxy_replay_pso);
	D912PXY_REPLAY_HA_DECL(void, OMSR, d912pxy_replay_om_sr);
	D912PXY_REPLAY_HA_DECL(void, OMBF, d912pxy_replay_om_bf);
	D912PXY_REPLAY_HA_DECL(void, RSVP, d912pxy_replay_rs_viewscissor);
	D912PXY_REPLAY_HA_DECL(void, RSSR, d912pxy_replay_rs_viewscissor);
	D912PXY_REPLAY_HA_DECL(void, DIIP, d912pxy_replay_draw_indexed_instanced);
	D912PXY_REPLAY_HA_DECL(void, OMRT, d912pxy_replay_om_render_target);
	D912PXY_REPLAY_HA_DECL(void, IFVB, d912pxy_replay_vbuf_bind);
	D912PXY_REPLAY_HA_DECL(void, IFIB, d912pxy_replay_ibuf_bind);
	D912PXY_REPLAY_HA_DECL(void, RCLR, d912pxy_replay_clear_rt);
	D912PXY_REPLAY_HA_DECL(void, DCLR, d912pxy_replay_clear_ds);
	D912PXY_REPLAY_HA_DECL(ID3D12PipelineState*, RPSO, d912pxy_replay_pso_raw);
	D912PXY_REPLAY_HA_DECL(ID3D12PipelineState*, RPSF, d912pxy_replay_pso_raw_feedback);	
	D912PXY_REPLAY_HA_DECL(void, RECT, d912pxy_replay_rect);

	d912pxy_replay_item stack[PXY_INNER_MAX_IFRAME_BATCH_REPLAY];
	UINT stackTop;

	UINT ignoreDIIP;
	
	UINT switchRange;	
	UINT switchPoint;
	UINT rangeEnds[PXY_INNER_REPLAY_THREADS];
	UINT cWorker;

	d912pxy_replay_thread* threads[PXY_INNER_REPLAY_THREADS];
	LONG stopMarker;
};


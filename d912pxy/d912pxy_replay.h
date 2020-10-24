/*
MIT License

Copyright(c) 2018-2020 megai2

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

struct d912pxy_replay_tracked_state
{
	d912pxy_surface* surfBind[PXY_INNER_MAX_RENDER_TARGETS + 1];
	DWORD srefVal;
	DWORD bfacVal;
	UINT scissorEnabled;

	UINT32 activeStreams;
	d912pxy_device_streamsrc streams[PXY_INNER_MAX_VBUF_STREAMS];
	d912pxy_vstream* indexBuf;

	d912pxy_trimmed_pso_desc pso;
	d912pxy_pso_item* cpso;

	D3D12_VIEWPORT main_viewport;
	D3D12_RECT main_scissor;

	D3DPRIMITIVETYPE primType;

	//not used in thread transits

	d912pxy_shader_pair_hash_type spair;
	float bfacColor[4];
};

typedef struct d912pxy_replay_thread_context {
	ID3D12PipelineState* pso;
	UINT tid;
	D3D12_GPU_VIRTUAL_ADDRESS customBatchPtr;
	d912pxy_replay_tracked_state tracked;
	ID3D12GraphicsCommandList* cl;
} d912pxy_replay_thread_context;

class d912pxy_replay;

typedef void (d912pxy_replay::*d912pxy_replay_handler_func)(void*, ID3D12GraphicsCommandList* cl, d912pxy_replay_thread_context*);

class d912pxy_replay : public d912pxy_noncom
{
public:
	d912pxy_replay();
	~d912pxy_replay();

	void Init();
	void Free();

	//actions

	UINT DoBarrier(d912pxy_resource* res, D3D12_RESOURCE_STATES to);
	void DoOMStencilRef(DWORD ref);
	void DoOMBlendFac(float* color);
	void DoRSViewScissor(D3D12_VIEWPORT viewport, D3D12_RECT scissor);
	void DoPSOCompiled(d912pxy_pso_item* dsc);
	void DoPSORaw(d912pxy_trimmed_pso_desc* dsc);
	void DoPSORawFeedback(d912pxy_trimmed_pso_desc* dsc, void** ptr);
	void DoVBbind(d912pxy_vstream* buf, UINT stride,	UINT slot, UINT offset);
	void DoIBbind(d912pxy_vstream* buf);
	void DoDIIP(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation, UINT batchId);
	void DoRT(d912pxy_surface** rtv, d912pxy_surface* dsv);
	void DoRTClear(d912pxy_surface* tgt, float* clr, D3D12_VIEWPORT* currentVWP);
	void DoDSClear(d912pxy_surface* tgt, float depth, UINT8 stencil, D3D12_CLEAR_FLAGS flag, D3D12_VIEWPORT* currentVWP);
	void DoStretchRect(d912pxy_surface* src, d912pxy_surface* dst);
	void DoGPUW(UINT32 si, UINT16 of, UINT16 cnt, UINT16 bn);
	void DoQueryMark(d912pxy_query* va, UINT8 start);
	void DoPrimTopo(D3DPRIMITIVETYPE primType);
	void DoUseCustomBatchData(d912pxy_custom_batch_data* customDrawParams);

	//actual execute code and thread managment

	void PlayId(d912pxy_replay_item* it, ID3D12GraphicsCommandList* cl, d912pxy_replay_thread_context* context);
	void Replay(UINT items, ID3D12GraphicsCommandList * cl, d912pxy_replay_thread* thrd);
	d912pxy_replay_item* WaitForData(d912pxy_replay_item* from, d912pxy_replay_thread* thrd);

	bool WaitForWake(d912pxy_replay_thread* thrd);

	void Finish();
	void Start();
	void IFrameStart();	
	void IssueWork(UINT batch);

	UINT GetThreadCount() { return numThreads; };
	void ReRangeThreads(UINT batches);

private:
	//thread transit class
	class thread_transit_data
	{
	public:
		thread_transit_data() { Reset(); };
		~thread_transit_data() { };

		void Reset();
		void Gather(d912pxy_replay_item* threadStartingItem, UINT in_tailItems);
		bool Apply(ID3D12GraphicsCommandList* cl, d912pxy_replay_thread_context* context);

		d912pxy_replay_item* GetBaseItem() {
			return startPoint;
		};

		UINT GetTailedItems()
		{
			return tailItems;
		}
		
		bool IsSaved() {
			return saved.GetValue() != 0;
		};

	private:
		d912pxy_thread_lock saved;
		d912pxy_replay_item* startPoint;
		UINT tailItems;

		d912pxy_replay_tracked_state data;
	};

	//handlers
	d912pxy_replay_handler_func replay_handlers[(UINT)d912pxy_replay_item::typeName::_count];

#define RHA_DECL(a,b) \
void RHA_##a(d912pxy_replay_item::dt_##a* it, ID3D12GraphicsCommandList * cl, b); \
void RHA_##a##_extra(d912pxy_replay_item::dt_##a* it, ID3D12GraphicsCommandList * cl, d912pxy_replay_thread_context* context)
	RHA_DECL(barrier, void* unused);
	RHA_DECL(om_stencilref, void* unused);
	RHA_DECL(om_blendfactor, void* unused);
	RHA_DECL(view_scissor, void* unused);
	RHA_DECL(draw_indexed, d912pxy_replay_thread_context* context);
	RHA_DECL(om_render_targets, d912pxy_replay_thread_context* context);
	RHA_DECL(vbuf_bind, void* unused);
	RHA_DECL(ibuf_bind, void* unused);
	RHA_DECL(clear_rt, void* unused);
	RHA_DECL(clear_ds, void* unused);
	RHA_DECL(pso_raw, d912pxy_replay_thread_context* context);
	RHA_DECL(pso_raw_feedback, void* unused);
	RHA_DECL(pso_compiled, d912pxy_replay_thread_context* context);
	RHA_DECL(rect_copy, void* unused);
	RHA_DECL(gpu_write_ctl, d912pxy_replay_thread_context* context);
	RHA_DECL(ia_prim_topo, void* unused);
	RHA_DECL(query_mark, void* unused);
	RHA_DECL(custom_batch_data, d912pxy_replay_thread_context* context);
#undef RHA_DECL		

	d912pxy_replay_buffer buffer;

	//thread ranges and signals
	UINT switchRange;	
	UINT switchPoint;
	UINT rangeEnds[PXY_INNER_REPLAY_THREADS_MAX];
	UINT cWorker;
	UINT numThreads;

	thread_transit_data transitData[PXY_INNER_REPLAY_THREADS_MAX];
	d912pxy_replay_thread* threads[PXY_INNER_REPLAY_THREADS_MAX];
	std::atomic<LONG> stopMarker{ 0 };

public:
	struct ExtraFeatures
	{
		bool enable;

		struct PairTracker
		{
			bool enable;			
			d912pxy_thread_lock lock;
			int writable=0;
			int lastReaded=0;
			d912pxy::Trivial::PushBuffer<d912pxy_shader_pair_hash_type> replayList[2];
			typedef d912pxy::Memtree<d912pxy_shader_pair_hash_type, bool, d912pxy::RawHash<d912pxy_shader_pair_hash_type>> ExclusionStorage;
			ExclusionStorage exclusions;

			void nextFrame();
			void write(d912pxy_shader_pair_hash_type v);
			d912pxy::Trivial::PushBuffer<d912pxy_shader_pair_hash_type>& read();
			void finishRead();
		} pairTracker;

	} extras;
	
};


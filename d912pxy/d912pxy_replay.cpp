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
#include "stdafx.h"

d912pxy_replay::d912pxy_replay() 
{

}

d912pxy_replay::~d912pxy_replay()
{	
}

void d912pxy_replay::Init()
{
	NonCom_Init(L"replay mt");
	buffer.Init();	

	stopMarker = 0;

	numThreads = d912pxy_s.config.GetValueUI32(PXY_CFG_REPLAY_THREADS);

	if (numThreads > PXY_INNER_REPLAY_THREADS_MAX)
	{
		LOG_ERR_DTDM("Max replay threads is %u, you selected %u, forcing max value", PXY_INNER_REPLAY_THREADS_MAX, numThreads);
		numThreads = 4;
	}

	if (d912pxy_s.config.GetValueUI32(PXY_CFG_BATCHING_RAW_GPUW))
	{
		LOG_WARN_DTDM("Raw gpu write is removed from code cuz it is useless, delta batching used");
	}
	
	d912pxy_s.dev.AddActiveThreads(numThreads);

	for (int i = 0; i != numThreads; ++i)
	{
		d912pxy_gpu_cmd_list_group clg = (d912pxy_gpu_cmd_list_group)(CLG_RP1 + i);

		char thrdName[255];
		sprintf(thrdName, "d912pxy replay %u", i);

		d912pxy_s.dx12.que.EnableGID(clg, PXY_INNER_CLG_PRIO_REPLAY + i);

		threads[i] = new d912pxy_replay_thread(clg, thrdName);
	}

	ReRangeThreads(0);

	for (int i = 0; i != numThreads; ++i)
	{
		threads[i]->Resume();
		transitData[i].Reset();
	}

#define RHA_ASSIGN(a) replay_handlers[(UINT)d912pxy_replay_item::typeName::a] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_##a
	RHA_ASSIGN(barrier);
	RHA_ASSIGN(om_stencilref);
	RHA_ASSIGN(om_blendfactor);
	RHA_ASSIGN(view_scissor);
	RHA_ASSIGN(draw_indexed);
	RHA_ASSIGN(om_render_targets);
	RHA_ASSIGN(vbuf_bind);
	RHA_ASSIGN(ibuf_bind);
	RHA_ASSIGN(clear_rt);
	RHA_ASSIGN(clear_ds);
	RHA_ASSIGN(pso_raw);
	RHA_ASSIGN(pso_raw_feedback);
	RHA_ASSIGN(pso_compiled);
	RHA_ASSIGN(rect_copy);
	RHA_ASSIGN(gpu_write_ctl);
	RHA_ASSIGN(ia_prim_topo);
	RHA_ASSIGN(query_mark);
	RHA_ASSIGN(custom_batch_data);
#undef RHA_ASSIGN
	
}

void d912pxy_replay::PlayId(d912pxy_replay_item* it, ID3D12GraphicsCommandList * cl, d912pxy_replay_thread_context* context)
{	
	LOG_DBG_DTDM("RP I: %llX Type: %u Thread: %u", it, (UINT)it->GetTypeName(), context->tid);
	(this->*replay_handlers[(UINT)it->GetTypeName()])(it->GetData<void*>(), cl, context);
}

void d912pxy_replay::Replay(UINT items, ID3D12GraphicsCommandList * cl, d912pxy_replay_thread* thrd)
{
	LOG_DBG_DTDM2("replay %u items", items);
		
	d912pxy_replay_thread_context context;
	context.pso = 0;
	context.tid = thrd->GetId();
	context.customBatchPtr = 0;

	d912pxy_replay_item* item_iter;
	//megai2: wait for actual data
	if (context.tid > 0)
	{		
		auto transit = &transitData[context.tid];

		while (!transit->IsSaved())
		{
			if (!WaitForWake(thrd))
				return;
		}
		transit->Apply(cl, &context);

		item_iter = transit->GetBaseItem();
		items -= transit->GetTailedItems();
	}
	else 
	{
		item_iter = buffer.getBase();
	}

	d912pxy_replay_item* item_end = item_iter;
	
	//execute operations
	while (items)
	{
		while (item_iter < item_end)
		{
			PlayId(item_iter, cl, &context);

			item_iter = item_iter->Next();
			--items;		

			if (!items)
				break;
		}
		
		item_end = WaitForData(item_iter, thrd);			

		if (!item_end)
			break;
	}

	//megai2: unlock thread only when stopMarker is set
	while (WaitForWake(thrd))
		;	

	//megai2: we can't put correct starting item on next thread, so we have to play things to next draw call as switch occuring right in that place
	//but we also have to properly process data that is visible in thread after stop marker is set
	
	bool linkedThread = context.tid != (numThreads - 1);

	item_end = buffer.getCurrentExtern();
	while (item_iter < item_end)
	{
		PlayId(item_iter, cl, &context);

		if (linkedThread)
		{
			if (items)
				--items;

			if ((item_iter->GetTypeName() == d912pxy_replay_item::typeName::draw_indexed) && !items)
				break;

		}

		item_iter = item_iter->Next();
	}
}

d912pxy_replay_item* d912pxy_replay::WaitForData(d912pxy_replay_item* from, d912pxy_replay_thread * thrd)
{	
	d912pxy_replay_item* ret = buffer.getCurrentExtern();
	while (ret <= from)
	{
		if (WaitForWake(thrd))
		{
			ret = buffer.getCurrentExtern();
		}
		else
			return nullptr;
	}

	return ret;
}

bool d912pxy_replay::WaitForWake(d912pxy_replay_thread* thrd)
{
	if (InterlockedAdd(&stopMarker, 0))
		return false;

	thrd->WaitForJob();

	return true;
}

void d912pxy_replay::Finish()
{
	buffer.CheckRange();
	buffer.syncCurrent();

	InterlockedIncrement(&stopMarker);	
	
	for (int i = 0; i != numThreads; ++i)
		threads[i]->Finish();	

	ReRangeThreads(buffer.getIndex());
}

void d912pxy_replay::IssueWork(UINT batch)
{
	if (buffer.getIndex() > switchPoint)
	{	
		buffer.syncCurrent();

		//megai2: skip workers if they don't have DIIP calls and transit state to proper one
		while (buffer.getIndex() > rangeEnds[cWorker + 1])
		{
			//megai2: wake threads so they do their job
			threads[cWorker]->SignalWork();
			switchPoint = rangeEnds[cWorker];
			++cWorker;
		}

		transitData[cWorker + 1].Gather(buffer.getCurrent(), buffer.getIndex() - switchPoint);
		threads[cWorker]->SignalWork();
	
		++cWorker;
		switchPoint = rangeEnds[cWorker];				
		threads[cWorker]->SignalWork();
		
	}
	else if ((batch % PXY_WAKE_FACTOR_REPLAY) == 0) {
		buffer.syncCurrent();
		threads[cWorker]->SignalWork();
	}
}

void d912pxy_replay::ReRangeThreads(UINT maxRange)
{
	switchRange = maxRange / numThreads;

	if (switchRange < 30)
		switchRange = 30;
	 
	for (int i = 0; i != numThreads; ++i)
	{		
		rangeEnds[i] = switchRange * (i + 1);

		if (i == (numThreads - 1))
			rangeEnds[i] = 0xFFFFFFFF;		

		threads[i]->ExecItems(rangeEnds[i] - switchRange * i);
	}

	switchPoint = rangeEnds[0];
	cWorker = 0;
}

void d912pxy_replay::Free()
{
	for (int i = 0; i != numThreads; ++i)
	{
		if (threads[i])
			threads[i]->Stop();
		delete threads[i];
	}

	buffer.UnInit();
	
	d912pxy_noncom::UnInit();
}

void d912pxy_replay::Start()
{
	LOG_DBG_DTDM("RP START");

	buffer.Reset();

	InterlockedDecrement(&stopMarker);
}

void d912pxy_replay::IFrameStart()
{
}

void d912pxy_replay::thread_transit_data::Reset()
{
	saved.SetValue(0);
}

void d912pxy_replay::thread_transit_data::Gather(d912pxy_replay_item* threadStartingItem, UINT in_tailItems)
{
	bfacVal = d912pxy_s.render.state.pso.GetDX9RsValue(D3DRS_BLENDFACTOR);
	srefVal = d912pxy_s.render.state.pso.GetDX9RsValue(D3DRS_STENCILREF);

	surfBind[0] = d912pxy_s.render.iframe.GetBindedSurface(0);
	surfBind[1] = d912pxy_s.render.iframe.GetBindedSurface(1);

	indexBuf = d912pxy_s.render.iframe.GetIBuf();

	activeStreams = d912pxy_s.render.iframe.GetActiveStreamCount();
	scissorEnabled = d912pxy_s.render.state.pso.GetDX9RsValue(D3DRS_SCISSORTESTENABLE);

	for (int i = 0; i != PXY_INNER_MAX_VBUF_STREAMS; ++i)
		streams[i] = d912pxy_s.render.iframe.GetStreamSource(i);

	pso = d912pxy_s.render.state.pso.GetCurrentDesc();

	main_viewport = *d912pxy_s.render.iframe.GetViewport();
	main_scissor = *d912pxy_s.render.iframe.GetScissorRect();

	primType = d912pxy_s.render.iframe.GetCurrentPrimType();
	cpso = d912pxy_s.render.state.pso.GetCurrentCPSO();
	startPoint = threadStartingItem;
	tailItems = in_tailItems;
	
	saved.SetValue(1);
}

bool d912pxy_replay::thread_transit_data::Apply(ID3D12GraphicsCommandList* cl, d912pxy_replay_thread_context* context)
{
	if (!saved.GetValue())
		return false;

	d912pxy_replay_item item;

#define ITEM_TRANSIT(a,b) \
	item.Set(d912pxy_replay_item::dt_##a b); \
	d912pxy_s.render.replay.PlayId(&item, cl, context) \

	ITEM_TRANSIT(om_render_targets, ({ surfBind[1], surfBind[0] }));	

	for (UINT i = 0; i != PXY_INNER_MAX_VBUF_STREAMS; ++i)
	{
		if (!streams[i].buffer)
			continue;

		ITEM_TRANSIT(vbuf_bind, ({ streams[i].buffer, streams[i].stride, i, streams[i].offset }));
	}

	if (indexBuf)
	{
		ITEM_TRANSIT(ibuf_bind, ({ indexBuf }));
	}

	ITEM_TRANSIT(om_stencilref, ({ srefVal }));

	fv4Color bfColor = d912pxy_s.render.state.pso.TransformBlendFactor(bfacVal);
	ITEM_TRANSIT(om_blendfactor, ({ { bfColor.val[0], bfColor.val[1], bfColor.val[2], bfColor.val[3]} }));

	ITEM_TRANSIT(ia_prim_topo, ({ (UINT8)primType }));

	if (cpso)
	{
		ITEM_TRANSIT(pso_compiled, ({ cpso }));
	}
	else {
		ITEM_TRANSIT(pso_raw, ({ pso }));
	}

	cl->RSSetViewports(1, &main_viewport);

	if (scissorEnabled)
		cl->RSSetScissorRects(1, &main_scissor);
	else {
		D3D12_RECT r;
		r.left = (UINT)main_viewport.TopLeftX;
		r.top = (UINT)main_viewport.TopLeftY;
		r.bottom = (UINT)main_viewport.Height + (UINT)main_viewport.TopLeftY;
		r.right = (UINT)main_viewport.Width + (UINT)main_viewport.TopLeftX;

		cl->RSSetScissorRects(1, &r);
	}

	Reset();

	return true;

#undef ITEM_TRANSIT
}


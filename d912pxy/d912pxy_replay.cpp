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
#include "stdafx.h"
#include "d912pxy_replay_base.h"

#ifdef _DEBUG
	#define REPLAY_STACK_GET(x) d912pxy_replay_item* it = &stack[DbgStackGet()]; it->type = x
	#define REPLAY_STACK_INCREMENT DbgStackIncrement()
	#define REPLAY_STACK_IGNORE return DbgStackIgnore()
#else
	#define REPLAY_STACK_GET(x) d912pxy_replay_item* it = &stack[stackTop]; it->type = x
	#define REPLAY_STACK_INCREMENT ++stackTop
	#define REPLAY_STACK_IGNORE return 0
#endif

d912pxy_replay::d912pxy_replay() 
{

}

d912pxy_replay::~d912pxy_replay()
{	
}

void d912pxy_replay::Init()
{
	NonCom_Init(L"replay mt");
	
	stack = 0;
	
	maxReplayItems = PXY_INNER_MAX_IFRAME_BATCH_COUNT * d912pxy_s.config.GetValueUI32(PXY_CFG_REPLAY_ITEMS_PER_BATCH);

	PXY_MALLOC(stack, (sizeof(d912pxy_replay_item)*maxReplayItems), d912pxy_replay_item*);

	stackTop = 0;
	stopMarker = 0;

	numThreads = (UINT)d912pxy_s.config.GetValueUI64(PXY_CFG_REPLAY_THREADS);

	d912pxy_s.dev.AddActiveThreads(numThreads);

	for (int i = 0; i != numThreads; ++i)
	{
		d912pxy_gpu_cmd_list_group clg = (d912pxy_gpu_cmd_list_group)(CLG_RP1 + i);

		char thrdName[255];
		sprintf(thrdName, "d912pxy replay %u", i);

		d912pxy_s.dx12.que.EnableGID(clg, PXY_INNER_CLG_PRIO_REPLAY + i);

		threads[i] = new d912pxy_replay_thread(clg, thrdName);

		transitData[i].saved = 0;
	}

	ReRangeThreads(maxReplayItems);

	for (int i = 0; i != numThreads; ++i)
		threads[i]->Resume();

	replay_handlers[DRPL_TRAN] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_TRAN;
	replay_handlers[DRPL_OMSR] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_OMSR;
	replay_handlers[DRPL_OMBF] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_OMBF;
	replay_handlers[DRPL_RSVP] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_RSVP;
	replay_handlers[DRPL_RSSR] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_RSSR;
	replay_handlers[DRPL_DIIP] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_DIIP;
	replay_handlers[DRPL_OMRT] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_OMRT;
	replay_handlers[DRPL_IFVB] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_IFVB;
	replay_handlers[DRPL_IFIB] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_IFIB;
	replay_handlers[DRPL_RCLR] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_RCLR;
	replay_handlers[DRPL_DCLR] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_DCLR;
	replay_handlers[DRPL_RPSO] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_RPSO;
	replay_handlers[DRPL_RPSF] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_RPSF;
	replay_handlers[DRPL_CPSO] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_CPSO;
	replay_handlers[DRPL_RECT] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_RECT;
	replay_handlers[DRPL_PRMT] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_PRMT;
	replay_handlers[DRPL_QUMA] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_QUMA;

	if (numThreads > 1)
		replay_handlers[DRPL_GPUW] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_GPUW_MT;
	else
		replay_handlers[DRPL_GPUW] = (d912pxy_replay_handler_func)&d912pxy_replay::RHA_GPUW;	
}

UINT d912pxy_replay::StateTransit(d912pxy_resource * res, D3D12_RESOURCE_STATES to)
{
	REPLAY_STACK_GET(DRPL_TRAN);

	D3D12_RESOURCE_STATES cstate = res->GetCurrentState();

	if (to == cstate)
		REPLAY_STACK_IGNORE;

	it->transit.res = res;
	it->transit.to = to;
	it->transit.from = cstate;

	res->ATransit(to);

	REPLAY_STACK_INCREMENT;

	return 1;
}

void d912pxy_replay::PSOCompiled(d912pxy_pso_cache_item * dsc)
{
	REPLAY_STACK_GET(DRPL_CPSO);

	it->compiledPso.psoItem = dsc;

	REPLAY_STACK_INCREMENT;
}

void d912pxy_replay::PSORaw(d912pxy_trimmed_dx12_pso * dsc)
{
	REPLAY_STACK_GET(DRPL_RPSO);

	it->rawPso.rawState = *dsc;

	REPLAY_STACK_INCREMENT;
}

void d912pxy_replay::PSORawFeedback(d912pxy_trimmed_dx12_pso * dsc, void ** ptr)
{
	REPLAY_STACK_GET(DRPL_RPSF);

	it->rawPsoFeedback.rawState = *dsc;
	it->rawPsoFeedback.feedbackPtr = ptr;

	REPLAY_STACK_INCREMENT;
}

void d912pxy_replay::OMStencilRef(DWORD ref)
{
	REPLAY_STACK_GET(DRPL_OMSR);

	it->omsr.dRef = ref;

	REPLAY_STACK_INCREMENT;
}

void d912pxy_replay::OMBlendFac(float * color)
{
	REPLAY_STACK_GET(DRPL_OMBF);

	it->ombf.color[0] = color[0];
	it->ombf.color[1] = color[1];
	it->ombf.color[2] = color[2];
	it->ombf.color[3] = color[3];

	REPLAY_STACK_INCREMENT;
}

void d912pxy_replay::RSViewScissor(D3D12_VIEWPORT viewport, D3D12_RECT scissor)
{
	REPLAY_STACK_GET(DRPL_RSVP);

	it->rs.scissor = scissor;
	it->rs.viewport = viewport;

	REPLAY_STACK_INCREMENT;
}

void d912pxy_replay::DIIP(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation, UINT batchId)
{
	REPLAY_STACK_GET(DRPL_DIIP);

	it->dip.IndexCountPerInstance = IndexCountPerInstance;
	it->dip.InstanceCount = InstanceCount;
	it->dip.StartIndexLocation = StartIndexLocation;
	it->dip.BaseVertexLocation = BaseVertexLocation;
	it->dip.StartInstanceLocation = StartInstanceLocation;
	it->dip.batchId = batchId;

	REPLAY_STACK_INCREMENT;
}

void d912pxy_replay::RT(d912pxy_surface * rtv, d912pxy_surface * dsv)
{
	REPLAY_STACK_GET(DRPL_OMRT);

	it->rt.dsv = dsv;
	it->rt.rtv = rtv;

	REPLAY_STACK_INCREMENT;
}

void d912pxy_replay::VBbind(d912pxy_vstream * buf, UINT stride, UINT slot, UINT offset)
{
	REPLAY_STACK_GET(DRPL_IFVB);

	it->vb.buf = buf;
	it->vb.stride = stride;
	it->vb.slot = slot;
	it->vb.offset = offset;

	REPLAY_STACK_INCREMENT;
}

void d912pxy_replay::IBbind(d912pxy_vstream * buf)
{
	REPLAY_STACK_GET(DRPL_IFIB);

	it->ib.buf = buf;

	REPLAY_STACK_INCREMENT;
}

void d912pxy_replay::StretchRect(d912pxy_surface * src, d912pxy_surface * dst)
{
	REPLAY_STACK_GET(DRPL_RECT);

	it->srect.src = src;
	it->srect.dst = dst;
	it->srect.prevS = src->GetCurrentState();
	it->srect.prevD = dst->GetCurrentState();
	
	REPLAY_STACK_INCREMENT;	
}

void d912pxy_replay::GPUW(UINT32 si, UINT16 of, UINT16 cnt, UINT16 bn)
{
	REPLAY_STACK_GET(DRPL_GPUW);

	it->gpuw_ctl.bn = bn;
	it->gpuw_ctl.size = cnt;
	it->gpuw_ctl.offset = of;
	it->gpuw_ctl.streamIdx = si;

	REPLAY_STACK_INCREMENT;
}

void d912pxy_replay::QueryMark(d912pxy_query * va, UINT start)
{
	REPLAY_STACK_GET(DRPL_QUMA);

	it->queryMark.obj = va;
	it->queryMark.start = start;

	REPLAY_STACK_INCREMENT;
}

void d912pxy_replay::PrimTopo(D3DPRIMITIVETYPE primType)
{
	REPLAY_STACK_GET(DRPL_PRMT);

	it->topo.newTopo = primType;

	REPLAY_STACK_INCREMENT;
}

void d912pxy_replay::RTClear(d912pxy_surface * tgt, float * clr, D3D12_VIEWPORT* currentVWP)
{
	D3D12_RESOURCE_STATES prevState = tgt->GetCurrentState();
	StateTransit(tgt, D3D12_RESOURCE_STATE_RENDER_TARGET);

	REPLAY_STACK_GET(DRPL_RCLR);

	it->clrRt.clr[0] = clr[3];
	it->clrRt.clr[1] = clr[2];
	it->clrRt.clr[2] = clr[1];
	it->clrRt.clr[3] = clr[0];
	it->clrRt.tgt = tgt;

	it->clrRt.clearRect.left = (LONG)currentVWP->TopLeftX;
	it->clrRt.clearRect.top = (LONG)currentVWP->TopLeftY;
	it->clrRt.clearRect.right = (LONG)(currentVWP->TopLeftX + currentVWP->Width);
	it->clrRt.clearRect.bottom = (LONG)(currentVWP->TopLeftY + currentVWP->Height);

	REPLAY_STACK_INCREMENT;

	StateTransit(tgt, prevState);
}

void d912pxy_replay::DSClear(d912pxy_surface * tgt, float depth, UINT8 stencil, D3D12_CLEAR_FLAGS flag, D3D12_VIEWPORT* currentVWP)
{
	D3D12_RESOURCE_STATES prevState = tgt->GetCurrentState();
	StateTransit(tgt, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	REPLAY_STACK_GET(DRPL_DCLR);

	it->clrDs.depth = depth;
	it->clrDs.flag = flag;
	it->clrDs.stencil = stencil;
	it->clrDs.tgt = tgt;

	it->clrDs.clearRect.left = (LONG)currentVWP->TopLeftX;
	it->clrDs.clearRect.top = (LONG)currentVWP->TopLeftY;
	it->clrDs.clearRect.right = (LONG)(currentVWP->TopLeftX + currentVWP->Width);
	it->clrDs.clearRect.bottom = (LONG)(currentVWP->TopLeftY + currentVWP->Height);

	REPLAY_STACK_INCREMENT;

	StateTransit(tgt, prevState);
}

void d912pxy_replay::PlayId(d912pxy_replay_item* it, ID3D12GraphicsCommandList * cl, void** context)
{	
	(this->*replay_handlers[it->type])(&it->ptr, cl, context);
}

void d912pxy_replay::Replay(UINT start, UINT end, ID3D12GraphicsCommandList * cl, d912pxy_replay_thread* thrd)
{
	LOG_DBG_DTDM("replay range [%u , %u, %u]", start, end, stackTop);
	
	UINT i = start;		
	UINT maxRI = 0;

	//megai2: wait for actual stack to be filled	
	maxRI = WaitForData(i, maxRI, end, thrd);

	if (!maxRI)
		return;

	PIXBeginEvent(cl, 0x88888888, "RP%u", thrd->GetId());

	ID3D12PipelineState* context = NULL;

	if (start > 0)
	{		
		PIXBeginEvent(cl, 0x88888888, "RPT%u", thrd->GetId());
		TransitCLState(cl, start, thrd->GetId(), (void**)&context);
		PIXEndEvent(cl);
	}

	PIXBeginEvent(cl, 0x88888888, "OMRT");
	PIXBeginEvent(cl, 0x88888888, "B");
	
	//execute operations
	while (i != end)
	{
		while (i < maxRI)
		{
			LOG_DBG_DTDM("RP TY %u %s", i, d912pxy_replay_item_type_dsc[stack[i].type]);

			PlayId(&stack[i], cl, (void**)&context);

#ifdef _DEBUG
			if (stack[i].type == DRPL_DIIP)
			{
				PIXEndEvent(cl);
				PIXBeginEvent(cl, 0x88888888, "B%u", stack[i].dip.batchId);
			}
			else if (stack[i].type == DRPL_OMRT)
			{
				PIXEndEvent(cl);
				PIXEndEvent(cl);
				PIXBeginEvent(cl, 0x88888888, "OMRT");
				PIXBeginEvent(cl, 0x88888888, "B");
			}
#endif
			++i;
		}
		
		maxRI = WaitForData(i, maxRI, end, thrd);			

		if (!maxRI)
		{
			PIXEndEvent(cl);
			return;
		}
	}

	//megai2: unlock thread only when stopMarker is set
	while (!InterlockedAdd(&stopMarker, 0))	
		thrd->WaitForJob();	

	PIXEndEvent(cl);
}

UINT d912pxy_replay::WaitForData(UINT idx, UINT maxRI, UINT end, d912pxy_replay_thread * thrd)
{	
	while (idx >= maxRI)
	{
		maxRI = GetStackTop();
		if (InterlockedAdd(&stopMarker, 0))
		{			
			//megai2: be sure to have the last one value
			maxRI = GetStackTop();
			if (idx >= maxRI)
				return 0;
			else
				break;
		}		
		PIXBeginEvent(0xAA0000, "rp thread wait");
		thrd->WaitForJob();
		PIXEndEvent();
	}

	if (maxRI > end)
		maxRI = end;

	return maxRI;
}

void d912pxy_replay::Finish()
{
	if (stackTop >= maxReplayItems)
	{
		LOG_ERR_THROW2(-1, "too many replay items");
	}
	
	SyncStackTop();
	InterlockedIncrement(&stopMarker);	
	
	for (int i = 0; i != numThreads; ++i)
		threads[i]->Finish();	

	LOG_DBG_DTDM("FSTK %u", stackTop);

	ReRangeThreads((UINT32)stackTop);
}

void d912pxy_replay::IssueWork(UINT batch)
{
	if (stackTop >= switchPoint)
	{	
		SyncStackTop();

		threads[cWorker]->SignalWork();
		
		++cWorker;
		switchPoint = rangeEnds[cWorker];
		SaveCLState(cWorker);
				
		threads[cWorker]->SignalWork();
		
	}
	else if ((batch % PXY_WAKE_FACTOR_REPLAY) == 0) {
		SyncStackTop();
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
			rangeEnds[i] = maxReplayItems;		

		threads[i]->ExecRange(switchRange * i, rangeEnds[i]);
	}

	//threads[0]->ExecRange(0, rangeEnds[0]);

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

	PXY_FREE(stack);

	this->~d912pxy_replay();
}

UINT d912pxy_replay::GetStackTop()
{
	return InterlockedAdd(&stackTopMT, 0);
}

void d912pxy_replay::SyncStackTop()
{
	InterlockedExchange(&stackTopMT, stackTop);
}

void d912pxy_replay::Start()
{
	LOG_DBG_DTDM("RP START");

	stackTop = 0;
	lastBFactorStk = -1;
	lastSRefStk = -1;

	SyncStackTop();

	InterlockedDecrement(&stopMarker);
}

void d912pxy_replay::IFrameStart()
{
}

d912pxy_replay_item * d912pxy_replay::BacktraceItemType(d912pxy_replay_item_type type, UINT depth, UINT base)
{
	if (stackTop == 0)
		return nullptr;

	for (int i = base - 1; i >= 0; --i)
	{
		if (stack[i].type == type)
		{
			if (!depth)
			{
				return &stack[i];
			}
			else
				--depth;
		}
	}

	return nullptr;
}

void d912pxy_replay::TransitBacktrace(d912pxy_replay_item_type type, UINT depth, ID3D12GraphicsCommandList* cl, UINT base, void** context)
{
	d912pxy_replay_item * it = BacktraceItemType(type, depth, base);

	if (it)
		PlayId(it, cl, context);
}

void d912pxy_replay::SaveCLState(UINT thread)
{
	d912pxy_replay_thread_transit_data* trd = &transitData[thread];

	trd->bfacVal = d912pxy_s.render.db.pso.GetDX9RsValue(D3DRS_BLENDFACTOR);
	trd->srefVal = d912pxy_s.render.db.pso.GetDX9RsValue(D3DRS_STENCILREF);

	trd->surfBind[0] = d912pxy_s.render.iframe.GetBindedSurface(0);
	trd->surfBind[1] = d912pxy_s.render.iframe.GetBindedSurface(1);

	trd->indexBuf = d912pxy_s.render.iframe.GetIBuf();

	for (int i = 0; i!= PXY_INNER_REPLAY_THREADS_MAX;++i)
		trd->streams[i] = d912pxy_s.render.iframe.GetStreamSource(i);

	trd->pso = *d912pxy_s.render.db.pso.GetCurrentDsc();

	trd->main_viewport = *d912pxy_s.render.iframe.GetViewport();
	trd->main_scissor = *d912pxy_s.render.iframe.GetScissorRect();

	trd->saved = 1;

	trd->cpso = d912pxy_s.render.db.pso.GetCurrentCPSO();
}

#ifdef _DEBUG

UINT d912pxy_replay::DbgStackGet()
{
	if (!simThreadAcc.TryHold())
	{
		LOG_ERR_THROW2(-1, "replay stack mt corruption");
	}

	return stackTop;
}

void d912pxy_replay::DbgStackIncrement()
{
	++stackTop;
	simThreadAcc.Release();
}

UINT d912pxy_replay::DbgStackIgnore()
{
	simThreadAcc.Release();
	return 0;
}

#endif 

void d912pxy_replay::TransitCLState(ID3D12GraphicsCommandList * cl, UINT base, UINT thread, void** context)
{
	d912pxy_replay_thread_transit_data* trd = &transitData[thread];

	if (!trd->saved)
		return;

	d912pxy_replay_item surfBind;
	surfBind.type = DRPL_OMRT;
	surfBind.rt.dsv = trd->surfBind[0];
	surfBind.rt.rtv = trd->surfBind[1];

	PlayId(&surfBind, cl, context);

	d912pxy_replay_item streamBind;
	streamBind.type = DRPL_IFVB;

	for (int i = 0; i != PXY_INNER_REPLAY_THREADS_MAX; ++i)
	{		
		if (!trd->streams[i].buffer)
			continue;

		streamBind.vb.buf = trd->streams[i].buffer;
		streamBind.vb.offset = trd->streams[i].offset;
		streamBind.vb.slot = i;
		streamBind.vb.stride = trd->streams[i].stride;

		PlayId(&streamBind, cl, context);
	}

	streamBind.type = DRPL_IFIB;
	streamBind.ib.buf = trd->indexBuf;
	PlayId(&streamBind, cl, context);


	d912pxy_replay_item sbVal;

	sbVal.type = DRPL_OMSR;
	sbVal.omsr.dRef = trd->srefVal;

	PlayId(&sbVal, cl, context);

	sbVal.type = DRPL_OMBF;

	fv4Color bfColor = d912pxy_s.render.db.pso.TransformBlendFactor(trd->bfacVal);
	memcpy(sbVal.ombf.color, bfColor.val, 16);

	PlayId(&sbVal, cl, context);

	d912pxy_replay_item psoSet;

	if (trd->cpso)
	{
		psoSet.type = DRPL_CPSO;
		psoSet.compiledPso.psoItem = trd->cpso;
	}
	else {
		psoSet.type = DRPL_RPSO;
		psoSet.rawPso.rawState = trd->pso;
	}
	PlayId(&psoSet, cl, context);

	cl->RSSetViewports(1, &trd->main_viewport);
	cl->RSSetScissorRects(1, &trd->main_scissor);
	   
	trd->saved = 0;	
}


void d912pxy_replay::RHA_TRAN(d912pxy_replay_state_transit * it, ID3D12GraphicsCommandList * cl, void** unused)
{
	it->res->BTransit(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, it->to, it->from, cl);
}

void d912pxy_replay::RHA_OMSR(d912pxy_replay_om_sr* it, ID3D12GraphicsCommandList * cl, void** unused)
{
	cl->OMSetStencilRef(it->dRef);	
}

void d912pxy_replay::RHA_OMBF(d912pxy_replay_om_bf* it, ID3D12GraphicsCommandList * cl, void** unused)
{
	cl->OMSetBlendFactor(it->color);
}

void d912pxy_replay::RHA_RSVP(d912pxy_replay_rs_viewscissor* it, ID3D12GraphicsCommandList * cl, void** unused)
{
	cl->RSSetViewports(1, &it->viewport);
	cl->RSSetScissorRects(1, &it->scissor);
}

void d912pxy_replay::RHA_RSSR(d912pxy_replay_rs_viewscissor* it, ID3D12GraphicsCommandList * cl, void** unused)
{	
	cl->RSSetScissorRects(1, &it->scissor);	
}

void d912pxy_replay::RHA_DIIP(d912pxy_replay_draw_indexed_instanced* it, ID3D12GraphicsCommandList * cl, ID3D12PipelineState** context)
{
	if (!*context)
		return;

	d912pxy_s.render.batch.PreDIP(cl, it->batchId);

	cl->DrawIndexedInstanced(
		it->IndexCountPerInstance,
		it->InstanceCount,
		it->StartIndexLocation,
		it->BaseVertexLocation,
		it->StartInstanceLocation
	);	
}

void d912pxy_replay::RHA_OMRT(d912pxy_replay_om_render_target* it, ID3D12GraphicsCommandList * cl, void** unused)
{
	D3D12_CPU_DESCRIPTOR_HANDLE bindedSurfacesDH[2];
	D3D12_CPU_DESCRIPTOR_HANDLE* bindedRTV = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE* bindedDSV = 0;

	if (it->rtv)
	{
		bindedRTV = &bindedSurfacesDH[0];
		bindedSurfacesDH[0] = it->rtv->GetDHeapHandle();
	}

	if (it->dsv)
	{
		bindedDSV = &bindedSurfacesDH[1];
		bindedSurfacesDH[1] = it->dsv->GetDHeapHandle();
	}

	if (it->rtv)
	{		
		cl->OMSetRenderTargets(1, bindedRTV, 0, bindedDSV);
	}
	else {		
		cl->OMSetRenderTargets(0, 0, 0, bindedDSV);		
	}
}

void d912pxy_replay::RHA_IFVB(d912pxy_replay_vbuf_bind* it, ID3D12GraphicsCommandList * cl, void** unused)
{
	it->buf->IFrameBindVB(it->stride, it->slot, it->offset, cl);
}

void d912pxy_replay::RHA_IFIB(d912pxy_replay_ibuf_bind* it, ID3D12GraphicsCommandList * cl, void** unused)
{
	it->buf->IFrameBindIB(cl);
}

void d912pxy_replay::RHA_RCLR(d912pxy_replay_clear_rt* it, ID3D12GraphicsCommandList * cl, void** unused)
{
	LOG_DBG_DTDM("RCLR tgt %llX", it->tgt);

	it->tgt->ClearAsRTV(it->clr, cl, &it->clearRect);
}

void d912pxy_replay::RHA_DCLR(d912pxy_replay_clear_ds* it, ID3D12GraphicsCommandList * cl, void** unused)
{
	it->tgt->ClearAsDSV(it->depth, it->stencil, it->flag, cl, &it->clearRect);
}

void d912pxy_replay::RHA_RPSO(d912pxy_replay_pso_raw* it, ID3D12GraphicsCommandList * cl, ID3D12PipelineState** context)
{
	ID3D12PipelineState * pso = d912pxy_s.render.db.pso.UseByDescMT(&it->rawState, 0);
		
	if (pso && (*context != pso))					
		cl->SetPipelineState(pso);	

	*context = pso;
}

void d912pxy_replay::RHA_CPSO(d912pxy_replay_pso_compiled* it, ID3D12GraphicsCommandList * cl, ID3D12PipelineState** context)
{	
	ID3D12PipelineState* pso = it->psoItem->GetPtr();

	if (pso)
		cl->SetPipelineState(pso);

	*context = pso;
}

void d912pxy_replay::RHA_RPSF(d912pxy_replay_pso_raw_feedback* it, ID3D12GraphicsCommandList * cl, void** unused)
{	
	*it->feedbackPtr = d912pxy_s.render.db.pso.GetByDescMT(&it->rawState, 0);	
}

void d912pxy_replay::RHA_RECT(d912pxy_replay_rect* it, ID3D12GraphicsCommandList * cl, void** unused)
{
	d912pxy_surface* sSrc = it->src;
	d912pxy_surface* sDst = it->dst;

	D3DSURFACE_DESC dSrc, dDst;

	dSrc = sSrc->GetDX9DescAtLevel(0);
	dDst = sDst->GetDX9DescAtLevel(0);
	
	if (
		(dSrc.Height == dDst.Height) &&
		(dSrc.Width == dDst.Width) &&
		(dSrc.Format == dDst.Format)
		)
	{
		if (!sSrc->GetD12Obj())
			sSrc->ConstructResource();

		if (!sDst->GetD12Obj())
			sDst->ConstructResource();

		sSrc->BCopyToWStates(sDst, 0x3, cl, it->prevD, it->prevS);
	} else {
		//megai2: TODO allow non similar texture copy via custom pass
		LOG_ERR_DTDM("rha rect with different textures");
	}
}

void d912pxy_replay::RHA_GPUW(d912pxy_replay_gpu_write_control * it, ID3D12GraphicsCommandList * cl, void ** unused)
{
	d912pxy_s.render.batch.GPUWriteControl(it->streamIdx, it->offset, it->size, it->bn);
}

void d912pxy_replay::RHA_GPUW_MT(d912pxy_replay_gpu_write_control * it, ID3D12GraphicsCommandList * cl, void ** unused)
{
	d912pxy_s.render.batch.GPUWriteControlMT(it->streamIdx, it->offset, it->size, it->bn);
}

void d912pxy_replay::RHA_PRMT(d912pxy_replay_primitive_topology * it, ID3D12GraphicsCommandList * cl, void ** unused)
{
	cl->IASetPrimitiveTopology((D3D12_PRIMITIVE_TOPOLOGY)it->newTopo);
}

void d912pxy_replay::RHA_QUMA(d912pxy_replay_query_mark * it, ID3D12GraphicsCommandList * cl, void ** unused)
{
	it->obj->QueryMark(it->start, cl);
}
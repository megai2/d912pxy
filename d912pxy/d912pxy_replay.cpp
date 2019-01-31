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

#define REPLAY_STACK_GET(x) d912pxy_replay_item* it = &stack[stackTop]; it->type = x

d912pxy_replay::d912pxy_replay(d912pxy_device * dev) : d912pxy_noncom(dev, L"replay")
{
	d912pxy_s(CMDReplay) = this;

	stackTop = 0;
	stopMarker = 1;

	for (int i = 0; i != PXY_INNER_REPLAY_THREADS; ++i)
	{
		d912pxy_gpu_cmd_list_group clg = (d912pxy_gpu_cmd_list_group)(CLG_RP1 + i);
		threads[i] = new d912pxy_replay_thread(dev, clg);
	}

	ReRangeThreads(PXY_INNER_MAX_IFRAME_BATCH_REPLAY);

	ignoreDIIP = 0;
}

d912pxy_replay::~d912pxy_replay()
{	
	for (int i = 0; i != PXY_INNER_REPLAY_THREADS; ++i)
	{
		threads[i]->Stop();
		delete threads[i];
	}		
}

UINT d912pxy_replay::ViewTransit(d912pxy_surface * res, D3D12_RESOURCE_STATES to)
{
	REPLAY_STACK_GET(DRPL_TRAN);

	if (to == res->GetCurrentState())
		return 0;

	it->transit.res = res;
	it->transit.to = to;
	it->transit.from = res->GetCurrentState();

	res->IFrameTrans(to);

	++stackTop;

	return 1;
}

void d912pxy_replay::PSOCompiled(d912pxy_pso_cache_item * dsc)
{
	REPLAY_STACK_GET(DRPL_CPSO);

	it->compiledPso.psoItem = dsc;

	++stackTop;
}

void d912pxy_replay::PSORaw(d912pxy_trimmed_dx12_pso * dsc)
{
	REPLAY_STACK_GET(DRPL_RPSO);

	it->rawPso.rawState = *dsc;

	++stackTop;
}

void d912pxy_replay::PSORawFeedback(d912pxy_trimmed_dx12_pso * dsc, void ** ptr)
{
	REPLAY_STACK_GET(DRPL_RPSF);

	it->rawPsoFeedback.rawState = *dsc;
	it->rawPsoFeedback.feedbackPtr = ptr;

	++stackTop;
}

void d912pxy_replay::OMStencilRef(DWORD ref)
{
	REPLAY_STACK_GET(DRPL_OMSR);

	it->omsr.dRef = ref;

	++stackTop;
}

void d912pxy_replay::OMBlendFac(float * color)
{
	REPLAY_STACK_GET(DRPL_OMBF);

	it->ombf.color[0] = color[0];
	it->ombf.color[1] = color[1];
	it->ombf.color[2] = color[2];
	it->ombf.color[3] = color[3];

	++stackTop;
}

void d912pxy_replay::RSViewScissor(D3D12_VIEWPORT viewport, D3D12_RECT scissor)
{
	REPLAY_STACK_GET(DRPL_RSVP);

	it->rs.scissor = scissor;
	it->rs.viewport = viewport;

	++stackTop;
}

void d912pxy_replay::DIIP(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
	REPLAY_STACK_GET(DRPL_DIIP);

	it->dip.IndexCountPerInstance = IndexCountPerInstance;
	it->dip.InstanceCount = InstanceCount;
	it->dip.StartIndexLocation = StartIndexLocation;
	it->dip.BaseVertexLocation = BaseVertexLocation;
	it->dip.StartInstanceLocation = StartInstanceLocation;

	++stackTop;
}

void d912pxy_replay::RT(d912pxy_surface * rtv, d912pxy_surface * dsv)
{
	REPLAY_STACK_GET(DRPL_OMRT);

	it->rt.dsv = dsv;
	it->rt.rtv = rtv;

	++stackTop;
}

void d912pxy_replay::VBbind(d912pxy_vbuf * buf, UINT stride, UINT slot, UINT offset)
{
	REPLAY_STACK_GET(DRPL_IFVB);

	it->vb.buf = buf;
	it->vb.stride = stride;
	it->vb.slot = slot;
	it->vb.offset = offset;

	++stackTop;
}

void d912pxy_replay::IBbind(d912pxy_ibuf * buf)
{
	REPLAY_STACK_GET(DRPL_IFIB);

	it->ib.buf = buf;

	++stackTop;
}

void d912pxy_replay::StretchRect(d912pxy_surface * src, d912pxy_surface * dst)
{
	D3D12_RESOURCE_STATES prevS = src->GetCurrentState();
	D3D12_RESOURCE_STATES prevD = dst->GetCurrentState();

	ViewTransit(src, D3D12_RESOURCE_STATE_COPY_SOURCE);
	ViewTransit(dst, D3D12_RESOURCE_STATE_COPY_DEST);

	REPLAY_STACK_GET(DRPL_RECT);

	it->srect.src = src;
	it->srect.dst = dst;

	++stackTop;

	ViewTransit(src, prevS);
	ViewTransit(dst, prevD);
}

void d912pxy_replay::RTClear(d912pxy_surface * tgt, float * clr)
{
	D3D12_RESOURCE_STATES prevState = tgt->GetCurrentState();
	ViewTransit(tgt, D3D12_RESOURCE_STATE_RENDER_TARGET);

	REPLAY_STACK_GET(DRPL_RCLR);

	it->clrRt.clr[0] = clr[3];
	it->clrRt.clr[1] = clr[2];
	it->clrRt.clr[2] = clr[1];
	it->clrRt.clr[3] = clr[0];
	it->clrRt.tgt = tgt;

	++stackTop;

	ViewTransit(tgt, prevState);
}

void d912pxy_replay::DSClear(d912pxy_surface * tgt, float depth, UINT8 stencil, D3D12_CLEAR_FLAGS flag)
{
	D3D12_RESOURCE_STATES prevState = tgt->GetCurrentState();
	ViewTransit(tgt, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	REPLAY_STACK_GET(DRPL_DCLR);

	it->clrDs.depth = depth;
	it->clrDs.flag = flag;
	it->clrDs.stencil = stencil;
	it->clrDs.tgt = tgt;

	++stackTop;

	ViewTransit(tgt, prevState);
}

D912PXY_REPLAY_HA_IMPL(void, TRAN, d912pxy_replay_view_transit)
{
	it->res->IFrameBarrierTrans2(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, it->to, it->from, cl);
}

D912PXY_REPLAY_HA_IMPL(void, OMSR, d912pxy_replay_om_sr)
{
	cl->OMSetStencilRef(it->dRef);
}

D912PXY_REPLAY_HA_IMPL(void, OMBF, d912pxy_replay_om_bf)
{
	cl->OMSetBlendFactor(it->color);
}

D912PXY_REPLAY_HA_IMPL(void, RSVP, d912pxy_replay_rs_viewscissor)
{
	cl->RSSetViewports(1, &it->viewport);
	cl->RSSetScissorRects(1, &it->scissor);
}

D912PXY_REPLAY_HA_IMPL(void, RSSR, d912pxy_replay_rs_viewscissor)
{
	cl->RSSetScissorRects(1, &it->scissor);
}

D912PXY_REPLAY_HA_IMPL(void, DIIP, d912pxy_replay_draw_indexed_instanced)
{
	//LOG_DBG_DTDM2("DIIP SIL %u ICPI %u IC %u SI %u BV %u", it->dip.StartInstanceLocation, it->dip.IndexCountPerInstance, it->dip.InstanceCount, it->dip.StartIndexLocation, it->dip.BaseVertexLocation);

	d912pxy_s(batch)->PreDIP(cl, it->StartInstanceLocation);

	cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cl->DrawIndexedInstanced(
		it->IndexCountPerInstance,
		it->InstanceCount,
		it->StartIndexLocation,
		it->BaseVertexLocation,
		0
	);

	d912pxy_s(batch)->PostDIP(cl);
}

D912PXY_REPLAY_HA_IMPL(void, OMRT, d912pxy_replay_om_render_target)
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
		cl->OMSetRenderTargets(1, bindedRTV, 0, bindedDSV);
	else
		cl->OMSetRenderTargets(0, 0, 0, bindedDSV);
}

D912PXY_REPLAY_HA_IMPL(void, IFVB, d912pxy_replay_vbuf_bind)
{
	/*if (!it->buf)
	{
		//LOG_ERR_THROW2(-1, "DRPL_IFVB buf == null");
		ignoreDIIP |= 2;
		return;
	} else 
		ignoreDIIP &= ~2;*/

	try {
		it->buf->GetBase()->IFrameBindVB(it->stride, it->slot, it->offset, cl);
	} catch (...)
	{
		try {
			LOG_ERR_THROW2(-1, "DRPL_IFVB error, but trying to continue");
		}
		catch (...)
		{
			;
		}
	}
}

D912PXY_REPLAY_HA_IMPL(void, IFIB, d912pxy_replay_ibuf_bind)
{
/*	if (!it->buf)
	{
		//LOG_ERR_THROW2(-1, "DRPL_IFIB buf == null");
		ignoreDIIP |= 1;
		return;
	}
	else
		ignoreDIIP &= ~1;*/
	try {
		it->buf->GetBase()->IFrameBindIB(cl);
	}
	catch (...)
	{
		try {
			LOG_ERR_THROW2(-1, "DRPL_IFIB error, but trying to continue");
		}
		catch (...)
		{
			;
		}
	}
}

D912PXY_REPLAY_HA_IMPL(void, RCLR, d912pxy_replay_clear_rt)
{
	it->tgt->d912_rtv_clear2(it->clr, cl);
}

D912PXY_REPLAY_HA_IMPL(void, DCLR, d912pxy_replay_clear_ds)
{
	it->tgt->d912_dsv_clear2(it->depth, it->stencil, it->flag, cl);
}

D912PXY_REPLAY_HA_IMPL(ID3D12PipelineState*, RPSO, d912pxy_replay_pso_raw)
{
	ID3D12PipelineState* psoPtr = d912pxy_s(psoCache)->UseByDesc(&it->rawState, 0)->GetPtr();

	if (psoPtr)
		cl->SetPipelineState(psoPtr);

	return psoPtr;
}

D912PXY_REPLAY_HA_IMPL(ID3D12PipelineState*, RPSF, d912pxy_replay_pso_raw_feedback)
{
	*it->feedbackPtr = d912pxy_s(psoCache)->UseByDesc(&it->rawState, 0);

	return NULL;
}

D912PXY_REPLAY_HA_IMPL(void, RECT, d912pxy_replay_rect)
{
	d912pxy_surface* sSrc = it->src;
	d912pxy_surface* sDst = it->dst;

	sSrc->CopyTo2(sDst, cl);
}

void d912pxy_replay::PlayId(UINT id, ID3D12GraphicsCommandList * cl)
{
	d912pxy_replay_item* it = &stack[id];

	switch (it->type)
	{
	case DRPL_TRAN:
	{
		RHA_TRAN(&it->transit, cl);
	}
	break;
	case DRPL_OMSR:
	{
		RHA_OMSR(&it->omsr, cl);
	}
	break;
	case DRPL_OMBF:
	{
		RHA_OMBF(&it->ombf, cl);
	}
	break;
	case DRPL_RSVP:
	{
		RHA_RSVP(&it->rs, cl);
	}
	break;
	case DRPL_RSSR:
	{
		RHA_RSSR(&it->rs, cl);
	}
	break;
	case DRPL_DIIP:
	{
		RHA_DIIP(&it->dip, cl);
	}
	break;
	case DRPL_OMRT:
	{
		RHA_OMRT(&it->rt, cl);
	}
	break;
	case DRPL_IFVB:
	{
		RHA_IFVB(&it->vb, cl);
	}
	break;
	case DRPL_IFIB:
	{
		RHA_IFIB(&it->ib, cl);
	}
	break;
	case DRPL_RCLR:
	{
		RHA_RCLR(&it->clrRt, cl);
	}
	break;
	case DRPL_DCLR:
	{
		RHA_DCLR(&it->clrDs, cl);
	}
	break;
	case DRPL_RECT:
	{
		RHA_RECT(&it->srect, cl);
	}
	break;
	case DRPL_RPSF:
	{
		RHA_RPSF(&it->rawPsoFeedback, cl);
	}
	break;
	default:
	{
		LOG_ERR_THROW2(-1, "replay type error");
	}
	break;
	}
}

void d912pxy_replay::Replay(UINT start, UINT end, d912pxy_gpu_cmd_list_group listGrp, d912pxy_replay_thread* thrd)
{
//	UINT32 startTime = GetTickCount();

	ID3D12GraphicsCommandList * cl = d912pxy_s(GPUcl)->GID(listGrp).Get();

	ID3D12PipelineState* psoPtr = NULL;

	LOG_DBG_DTDM("replay range [%u , %u, %u]", start, end, stackTop);

	d912pxy_s(iframe)->SetRSigOnList(listGrp);	

	for (UINT i = start; i != end; ++i)
	{
		LOG_DBG_DTDM("RP TY %s", d912pxy_replay_item_type_dsc[stack[i].type]);

		if (i >= stackTop)
		{
			if (InterlockedAdd(&stopMarker, 0))
				return;
		sleepAgain:
			thrd->WaitForJob();
			//if we waked this thread just to mark it finished
			if (i >= stackTop)
			{
				if (InterlockedAdd(&stopMarker, 0))
					return;
				else
					goto sleepAgain;
			}			
		}

		switch (stack[i].type) 
		{
		case DRPL_CPSO:
		{
			d912pxy_pso_cache_item* it = stack[i].compiledPso.psoItem;

			psoPtr = it->GetPtr();

			if (psoPtr)
				cl->SetPipelineState(psoPtr);

			break;
		}
		case DRPL_RPSO:
		{
			d912pxy_pso_cache_item* it = d912pxy_s(psoCache)->UseByDesc(&stack[i].rawPso.rawState, 0);
			
			if (it)
			{
				psoPtr = it->GetPtr();

				if (psoPtr)
					cl->SetPipelineState(psoPtr);
			}
			else
				psoPtr = NULL;
		}
		break;
		case DRPL_DIIP:
		{
			if (!psoPtr)
				break;
		}
		default:
			PlayId(i, cl);
			break;
		}
	}
}

void d912pxy_replay::Finish()
{
	if (stackTop >= PXY_INNER_MAX_IFRAME_BATCH_REPLAY)
	{
		LOG_ERR_THROW2(-1, "too many replay items");
	}

	InterlockedIncrement(&stopMarker);	

	for (int i = 0; i != PXY_INNER_REPLAY_THREADS; ++i)
		threads[i]->Finish();	

	ReRangeThreads(stackTop);
}

void d912pxy_replay::IssueWork(UINT batch)
{
	if (stackTop > switchPoint)
	{
		if ((cWorker + 1) == PXY_INNER_REPLAY_THREADS)
		{
			threads[cWorker]->SignalWork();
			switchPoint = PXY_INNER_MAX_IFRAME_BATCH_REPLAY;
			return;
		}
		//this should be executed on replay thread, with somekind new DRPL_ item, but for now we use one thread and this is sufficient, so TODO
		//d912pxy_s(iframe)->TransitStates((d912pxy_gpu_cmd_list_group)(CLG_RP1 + cWorker + 1));
		threads[cWorker]->SignalWork();
		threads[++cWorker]->SignalWork();
		switchPoint += switchRange;
	}
	else if ((batch % 100) == 0)
		threads[cWorker]->SignalWork();
}

void d912pxy_replay::ReRangeThreads(UINT maxRange)
{
	switchRange = maxRange / PXY_INNER_REPLAY_THREADS;
	switchPoint = switchRange;
	cWorker = 0;

	for (int i = 0; i != PXY_INNER_REPLAY_THREADS; ++i)
	{		
		rangeEnds[i] = switchRange * (i + 1);
		if (i == (PXY_INNER_REPLAY_THREADS - 1))
			threads[i]->ExecRange(switchRange * i, PXY_INNER_MAX_IFRAME_BATCH_REPLAY);
		else
			threads[i]->ExecRange(switchRange * i, rangeEnds[i]);
	}
}

void d912pxy_replay::Start()
{
	stackTop = 0;
	InterlockedDecrement(&stopMarker);
}

d912pxy_replay_item * d912pxy_replay::BacktraceItemType(d912pxy_replay_item_type type, UINT depth)
{
	if (stackTop == 0)
		return nullptr;

	for (int i = stackTop - 1; i > 0; --i)
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
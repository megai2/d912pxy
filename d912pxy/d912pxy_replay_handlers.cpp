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

#define RHA_DECL(a,b) void d912pxy_replay::RHA_##a(d912pxy_replay_item::dt_##a* it, ID3D12GraphicsCommandList * cl, b)

RHA_DECL(barrier, void* unused)
{
	it->res->BTransit(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, it->to, it->from, cl);
}

RHA_DECL(om_stencilref, void* unused)
{
	cl->OMSetStencilRef(it->dRef);
}

RHA_DECL(om_blendfactor, void* unused)
{
	cl->OMSetBlendFactor(it->color);
}

RHA_DECL(view_scissor, void* unused)
{
	cl->RSSetViewports(1, &it->viewport);
	cl->RSSetScissorRects(1, &it->scissor);
}

RHA_DECL(draw_indexed, d912pxy_replay_thread_context* context) 
{
	if (!context->pso)
		return;

	if (context->customBatchPtr)
	{
		cl->SetGraphicsRootConstantBufferView(3, context->customBatchPtr);
		context->customBatchPtr = 0;
	} else
		d912pxy_s.render.batch.Bind(cl, it->batchId);

	if (it->BaseVertexLocation < 0)
		cl->DrawInstanced(
			it->IndexCountPerInstance,
			it->InstanceCount,
			it->StartIndexLocation,
			it->StartInstanceLocation
		);
	else 
		cl->DrawIndexedInstanced(
			it->IndexCountPerInstance,
			it->InstanceCount,
			it->StartIndexLocation,
			it->BaseVertexLocation,
			it->StartInstanceLocation
		);
}

RHA_DECL(om_render_targets, void* unused)
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

RHA_DECL(vbuf_bind, void* unused)
{
	it->buf->IFrameBindVB(it->stride, it->slot, it->offset, cl);
}

RHA_DECL(ibuf_bind, void* unused)
{
	it->buf->IFrameBindIB(cl);
}

RHA_DECL(clear_rt, void* unused)
{
	LOG_DBG_DTDM("RCLR tgt %llX", it->tgt);

	if (it->cuState != D3D12_RESOURCE_STATE_RENDER_TARGET)
		it->tgt->BTransit(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_RENDER_TARGET, it->cuState, cl);

	it->tgt->ClearAsRTV(it->clr, cl, &it->clearRect);

	if (it->cuState != D3D12_RESOURCE_STATE_RENDER_TARGET)
		it->tgt->BTransit(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, it->cuState, D3D12_RESOURCE_STATE_RENDER_TARGET, cl);
}

RHA_DECL(clear_ds, void* unused)
{
	if (it->cuState != D3D12_RESOURCE_STATE_DEPTH_WRITE)
		it->tgt->BTransit(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_DEPTH_WRITE, it->cuState, cl);

	it->tgt->ClearAsDSV(it->depth, it->stencil, it->flag, cl, &it->clearRect);

	if (it->cuState != D3D12_RESOURCE_STATE_DEPTH_WRITE)
		it->tgt->BTransit(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, it->cuState, D3D12_RESOURCE_STATE_DEPTH_WRITE, cl);
}

RHA_DECL(pso_raw, d912pxy_replay_thread_context* context)
{
	auto psoItem = d912pxy_s.render.db.pso.GetByDescMT(&it->rawState);
	if (!psoItem)
		return;

	ID3D12PipelineState* pso = psoItem->GetPtr();

	/*
	//megai2: debug mode busy wait for pso compile
	while (!pso)
		pso = d912pxy_s.render.db.pso.UseByDescMT(&it->rawState, 0);
	*/

	if (pso && (context->pso != pso))
		cl->SetPipelineState(pso);

	context->pso = pso;
}

RHA_DECL(pso_raw_feedback, void* unused)
{
	*it->feedbackPtr = d912pxy_s.render.db.pso.GetByDescMT(&it->rawState);

	it->rawState.HoldRefs(false);
}

RHA_DECL(pso_compiled, d912pxy_replay_thread_context* context)
{
	ID3D12PipelineState* pso = it->psoItem->GetPtr();

	if (pso)
		cl->SetPipelineState(pso);

	context->pso = pso;
}

RHA_DECL(rect_copy, void* unused)
{
	d912pxy_surface* sSrc = it->src;
	d912pxy_surface* sDst = it->dst;

	if (!sSrc->GetD12Obj())
		sSrc->ConstructResource();

	if (!sDst->GetD12Obj())
		sDst->ConstructResource();

	sSrc->BCopyToWStates(sDst, 0x3, cl, it->prevD, it->prevS);	
}

RHA_DECL(gpu_write_ctl, d912pxy_replay_thread_context* context)
{
	d912pxy_s.render.batch.ReplayWrite(
		context->tid,
		it->streamIdx,
		d912pxy_batch_buffer::write_info(it->bn, it->offset, it->size)
	);
}

RHA_DECL(ia_prim_topo, void* unused)
{
	cl->IASetPrimitiveTopology((D3D12_PRIMITIVE_TOPOLOGY)it->newTopo);
}

RHA_DECL(query_mark, void* unused)
{
	it->obj->QueryMark(it->start, cl);
}

RHA_DECL(custom_batch_data, d912pxy_replay_thread_context* context)
{
	context->customBatchPtr = it->batchDataPtr;
}

#undef RHA_DECL
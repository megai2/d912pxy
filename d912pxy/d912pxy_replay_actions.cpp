/*
MIT License

Copyright(c) 2019-2020 megai2

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

#define ITEM_PUSH(a) \
	auto it = buffer.PushAction<d912pxy_replay_item::dt_##a>()	

UINT d912pxy_replay::DoBarrier(d912pxy_resource* res, D3D12_RESOURCE_STATES to)
{
	D3D12_RESOURCE_STATES cstate = res->GetCurrentState();

	if (to == cstate)
		return 0;

	ITEM_PUSH(barrier);

	it->res = res;
	it->to = to;
	it->from = cstate;

	res->ATransit(to);

	return 1;
}

void d912pxy_replay::DoPSOCompiled(d912pxy_pso_item* dsc)
{
	ITEM_PUSH(pso_compiled);

	it->psoItem = dsc;
}

void d912pxy_replay::DoPSORaw(d912pxy_trimmed_pso_desc* dsc)
{
	ITEM_PUSH(pso_raw);

	it->rawState = *dsc;
}

void d912pxy_replay::DoPSORawFeedback(d912pxy_trimmed_pso_desc* dsc, void** ptr)
{
	ITEM_PUSH(pso_raw_feedback);

	it->rawState = *dsc;
	it->feedbackPtr = ptr;
}

void d912pxy_replay::DoOMStencilRef(DWORD ref)
{
	ITEM_PUSH(om_stencilref);

	it->dRef = ref;
}

void d912pxy_replay::DoOMBlendFac(float* color)
{
	ITEM_PUSH(om_blendfactor);

	memcpy(it->color, color, sizeof(it->color));
}

void d912pxy_replay::DoRSViewScissor(D3D12_VIEWPORT viewport, D3D12_RECT scissor)
{
	ITEM_PUSH(view_scissor);

	it->viewport = viewport;
	it->scissor = scissor;
}

void d912pxy_replay::DoDIIP(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation, UINT batchId)
{
	ITEM_PUSH(draw_indexed);

	it->IndexCountPerInstance = IndexCountPerInstance;
	it->InstanceCount = InstanceCount;
	it->StartIndexLocation = StartIndexLocation;
	it->BaseVertexLocation = BaseVertexLocation;
	it->StartInstanceLocation = StartInstanceLocation;
	it->batchId = batchId;
}

void d912pxy_replay::DoRT(d912pxy_surface** rtv, d912pxy_surface* dsv)
{
	ITEM_PUSH(om_render_targets);

	it->dsv = dsv;
	for (int i = 0;i<PXY_INNER_MAX_RENDER_TARGETS;++i)
		it->rtv[i] = rtv[i];
}

void d912pxy_replay::DoVBbind(d912pxy_vstream* buf, UINT stride, UINT slot, UINT offset)
{
	ITEM_PUSH(vbuf_bind);

	it->buf = buf;
	it->stride = stride;
	it->slot = slot;
	it->offset = offset;
}

void d912pxy_replay::DoIBbind(d912pxy_vstream* buf)
{
	ITEM_PUSH(ibuf_bind);

	it->buf = buf;
}

void d912pxy_replay::DoStretchRect(d912pxy_surface* src, d912pxy_surface* dst)
{
	ITEM_PUSH(rect_copy);

	it->src = src;
	it->dst = dst;
	it->prevS = src->GetCurrentState();
	it->prevD = dst->GetCurrentState();
}

void d912pxy_replay::DoGPUW(UINT32 si, UINT16 of, UINT16 cnt, UINT16 bn)
{
	ITEM_PUSH(gpu_write_ctl);

	it->streamIdx = si;
	it->offset = of;
	it->size = cnt;
	it->bn = bn;
}

void d912pxy_replay::DoQueryMark(d912pxy_query* va, UINT8 start)
{
	ITEM_PUSH(query_mark);

	it->obj = va;
	it->start = start;
}

void d912pxy_replay::DoPrimTopo(D3DPRIMITIVETYPE primType)
{
	ITEM_PUSH(ia_prim_topo);

	it->newTopo = (UINT8)primType;
}

void d912pxy_replay::DoRTClear(d912pxy_surface* tgt, float* clr, D3D12_VIEWPORT* currentVWP)
{
	ITEM_PUSH(clear_rt);

	it->tgt = tgt;
	it->clr[0] = clr[3];
	it->clr[1] = clr[2];
	it->clr[2] = clr[1];
	it->clr[3] = clr[0];
	it->clearRect = {
				(LONG)currentVWP->TopLeftX,
				(LONG)currentVWP->TopLeftY,
				(LONG)(currentVWP->TopLeftX + currentVWP->Width),
				(LONG)(currentVWP->TopLeftY + currentVWP->Height)
	};
	it->cuState = tgt->GetCurrentState();
}

void d912pxy_replay::DoDSClear(d912pxy_surface* tgt, float depth, UINT8 stencil, D3D12_CLEAR_FLAGS flag, D3D12_VIEWPORT* currentVWP)
{
	ITEM_PUSH(clear_ds);

	it->tgt = tgt;
	it->depth = depth;
	it->stencil = stencil;
	it->flag = flag;
	it->clearRect = {
				(LONG)currentVWP->TopLeftX,
				(LONG)currentVWP->TopLeftY,
				(LONG)(currentVWP->TopLeftX + currentVWP->Width),
				(LONG)(currentVWP->TopLeftY + currentVWP->Height)
	};
	it->cuState = tgt->GetCurrentState();
}

void d912pxy_replay::DoUseCustomBatchData(d912pxy_custom_batch_data* customDrawParams)
{
	ITEM_PUSH(custom_batch_data);
	
	d912pxy_vstream* cbuf = PXY_COM_LOOKUP(customDrawParams->buffer, vstream);
	it->batchDataPtr = cbuf->GetVA_GPU() + sizeof(d912pxy_batch_buffer_element) * customDrawParams->index;
}
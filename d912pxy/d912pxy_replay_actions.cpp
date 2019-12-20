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

#define ITEM_PUSH(a,b) \
	buffer.PushAction( \
		d912pxy_replay_item::typeName::a, \
		d912pxy_replay_item::dt_##a b  \
	) 

UINT d912pxy_replay::DoBarrier(d912pxy_resource* res, D3D12_RESOURCE_STATES to)
{
	D3D12_RESOURCE_STATES cstate = res->GetCurrentState();

	if (to == cstate)
		return 0;

	ITEM_PUSH(barrier, ({ res, to, cstate }));

	res->ATransit(to);

	return 1;
}

void d912pxy_replay::DoPSOCompiled(d912pxy_pso_cache_item* dsc)
{
	ITEM_PUSH(pso_compiled, ({ dsc }));
}

void d912pxy_replay::DoPSORaw(d912pxy_trimmed_dx12_pso* dsc)
{
	ITEM_PUSH(pso_raw, ({ *dsc }));
}

void d912pxy_replay::DoPSORawFeedback(d912pxy_trimmed_dx12_pso* dsc, void** ptr)
{
	ITEM_PUSH(pso_raw_feedback, ({ *dsc, ptr }));
}

void d912pxy_replay::DoOMStencilRef(DWORD ref)
{
	ITEM_PUSH(om_stencilref, ({ref}));
}

void d912pxy_replay::DoOMBlendFac(float* color)
{
	ITEM_PUSH(om_blendfactor, ({ {color[0], color[1], color[2], color[3]} }));
}

void d912pxy_replay::DoRSViewScissor(D3D12_VIEWPORT viewport, D3D12_RECT scissor)
{
	ITEM_PUSH(view_scissor, ({ viewport, scissor }));
}

void d912pxy_replay::DoDIIP(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation, UINT batchId)
{
	ITEM_PUSH(
		draw_indexed, 
		({ 
			IndexCountPerInstance, 
			InstanceCount, 
			StartIndexLocation, 
			BaseVertexLocation, 
			StartInstanceLocation, 
			batchId
		})
	);
}

void d912pxy_replay::DoRT(d912pxy_surface* rtv, d912pxy_surface* dsv)
{
	ITEM_PUSH(om_render_targets, ({ rtv, dsv }));
}

void d912pxy_replay::DoVBbind(d912pxy_vstream* buf, UINT stride, UINT slot, UINT offset)
{
	ITEM_PUSH(vbuf_bind, ({ buf, stride, slot, offset }));
}

void d912pxy_replay::DoIBbind(d912pxy_vstream* buf)
{
	ITEM_PUSH(ibuf_bind, ({ buf }));
}

void d912pxy_replay::DoStretchRect(d912pxy_surface* src, d912pxy_surface* dst)
{
	ITEM_PUSH(rect_copy, ({ src, dst, src->GetCurrentState(), dst->GetCurrentState() }));
}

void d912pxy_replay::DoGPUW(UINT32 si, UINT16 of, UINT16 cnt, UINT16 bn)
{
	ITEM_PUSH(gpu_write_ctl, ({ si, of, cnt, bn }));
}

void d912pxy_replay::DoQueryMark(d912pxy_query* va, UINT8 start)
{
	ITEM_PUSH(query_mark, ({ va, start }));
}

void d912pxy_replay::DoPrimTopo(D3DPRIMITIVETYPE primType)
{
	ITEM_PUSH(ia_prim_topo, ({ (UINT8)primType }));
}

void d912pxy_replay::DoRTClear(d912pxy_surface* tgt, float* clr, D3D12_VIEWPORT* currentVWP)
{
	ITEM_PUSH(
		clear_rt, 
		({ 
			tgt, 
			{ clr[3], clr[2], clr[1], clr[0] }, 
			{ 
				(LONG)currentVWP->TopLeftX, 
				(LONG)currentVWP->TopLeftY, 
				(LONG)(currentVWP->TopLeftX + currentVWP->Width), 
				(LONG)(currentVWP->TopLeftY + currentVWP->Height)
			},
			tgt->GetCurrentState()
		})
	);
}

void d912pxy_replay::DoDSClear(d912pxy_surface* tgt, float depth, UINT8 stencil, D3D12_CLEAR_FLAGS flag, D3D12_VIEWPORT* currentVWP)
{
	ITEM_PUSH(
		clear_ds,
		({
			tgt,
			depth,
			stencil,
			flag,
			{
				(LONG)currentVWP->TopLeftX,
				(LONG)currentVWP->TopLeftY,
				(LONG)(currentVWP->TopLeftX + currentVWP->Width),
				(LONG)(currentVWP->TopLeftY + currentVWP->Height)
			},
			tgt->GetCurrentState()
		})
	);
}
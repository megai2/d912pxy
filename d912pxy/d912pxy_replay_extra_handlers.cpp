/*
MIT License

Copyright(c) 2020 megai2

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

#define RHA_DECL(a,b) void d912pxy_replay::RHA_##a##_extra(d912pxy_replay_item::dt_##a* it, ID3D12GraphicsCommandList * cl, b)
#define RHA_BASE(a,b) RHA_##a(it, cl, b);

RHA_DECL(barrier, void* unused)
{
	RHA_BASE(barrier, unused);
}

RHA_DECL(om_stencilref, void* unused)
{
	RHA_BASE(om_stencilref, unused);
}

RHA_DECL(om_blendfactor, void* unused)
{
	RHA_BASE(om_blendfactor, unused);
}

RHA_DECL(view_scissor, void* unused)
{
	RHA_BASE(view_scissor, unused);
}

RHA_DECL(draw_indexed, d912pxy_replay_thread_context* context)
{
	d912pxy_s.iframeMods.RP_PreDraw(it, context);
	if (extras.pairTracker.enable)
	{		
		extras.pairTracker.write(context->tracked.spair);		
		{
			d912pxy::mt::containter::Ref<ExtraFeatures::PairTracker::ExclusionStorage> excRef(extras.pairTracker.exclusions, context->tracked.spair);
			if (excRef.val)
				return;
		}
	}

	RHA_BASE(draw_indexed, context);

	d912pxy_s.iframeMods.RP_PostDraw(it, context);
}

RHA_DECL(om_render_targets, d912pxy_replay_thread_context* context)
{
	if ((context->tracked.dsv != it->dsv) || (context->tracked.rtv != it->rtv))
	{
		context->tracked.dsv = it->dsv;
		context->tracked.rtv = it->rtv;
	}

	d912pxy_s.iframeMods.RP_RTDSChange(it, context);

	if (extras.pairTracker.enable)
	{
		//add 2 markers for pass end & pass start on rt changes
		extras.pairTracker.write(0);
		int passId = (it->dsv ? 1 : 0) + (it->rtv ? 2 : 0);
		extras.pairTracker.write(passId);
	}

	RHA_BASE(om_render_targets, context);
}

RHA_DECL(vbuf_bind, void* unused)
{
	RHA_BASE(vbuf_bind, unused);
}

RHA_DECL(ibuf_bind, void* unused)
{
	RHA_BASE(ibuf_bind, unused);
}

RHA_DECL(clear_rt, void* unused)
{
	RHA_BASE(clear_rt, unused);
}

RHA_DECL(clear_ds, void* unused)
{
	RHA_BASE(clear_ds, unused);
}

RHA_DECL(pso_raw, d912pxy_replay_thread_context* context)
{
	d912pxy_trimmed_pso_desc& targetPso = it->rawState;
	context->tracked.spair = targetPso.GetShaderPairUID();

	d912pxy_s.iframeMods.RP_PSO_Change(it, context);
	
	RHA_BASE(pso_raw, context);
}

RHA_DECL(pso_raw_feedback, void* unused)
{
	RHA_BASE(pso_raw_feedback, unused);
}

RHA_DECL(pso_compiled, d912pxy_replay_thread_context* context)
{
	RHA_BASE(pso_compiled, context);
}

RHA_DECL(rect_copy, void* unused)
{
	RHA_BASE(rect_copy, unused);
}

RHA_DECL(gpu_write_ctl, d912pxy_replay_thread_context* context)
{
	RHA_BASE(gpu_write_ctl, context);
}

RHA_DECL(ia_prim_topo, void* unused)
{
	RHA_BASE(ia_prim_topo, unused);
}

RHA_DECL(query_mark, void* unused)
{
	RHA_BASE(query_mark, unused);
}

RHA_DECL(custom_batch_data, d912pxy_replay_thread_context* context)
{
	RHA_BASE(custom_batch_data, context);
}

#undef RHA_DECL
#undef RHA_BASE
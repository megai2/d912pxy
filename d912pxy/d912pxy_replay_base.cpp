#include "stdafx.h"

d912pxy_replay_base::d912pxy_replay_base()
{

}

d912pxy_replay_base::~d912pxy_replay_base()
{
}

UINT d912pxy_replay_base::StateTransit(d912pxy_resource * res, D3D12_RESOURCE_STATES to)
{
	return 0;
}

void d912pxy_replay_base::OMStencilRef(DWORD ref)
{
}

void d912pxy_replay_base::OMBlendFac(float * color)
{
}

void d912pxy_replay_base::RSViewScissor(D3D12_VIEWPORT viewport, D3D12_RECT scissor)
{
}

void d912pxy_replay_base::PSOCompiled(d912pxy_pso_cache_item * dsc)
{
}

void d912pxy_replay_base::PSORaw(d912pxy_trimmed_dx12_pso * dsc)
{
}

void d912pxy_replay_base::PSORawFeedback(d912pxy_trimmed_dx12_pso * dsc, void ** ptr)
{
}

void d912pxy_replay_base::VBbind(d912pxy_vstream * buf, UINT stride, UINT slot, UINT offset)
{
}

void d912pxy_replay_base::IBbind(d912pxy_vstream * buf)
{
}

void d912pxy_replay_base::DIIP(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation, UINT batchId)
{
}

void d912pxy_replay_base::RT(d912pxy_surface * rtv, d912pxy_surface * dsv)
{
}

void d912pxy_replay_base::RTClear(d912pxy_surface * tgt, float * clr, D3D12_VIEWPORT * currentVWP)
{
}

void d912pxy_replay_base::DSClear(d912pxy_surface * tgt, float depth, UINT8 stencil, D3D12_CLEAR_FLAGS flag, D3D12_VIEWPORT * currentVWP)
{
}

void d912pxy_replay_base::StretchRect(d912pxy_surface * src, d912pxy_surface * dst)
{
}

void d912pxy_replay_base::GPUW(UINT32 si, UINT16 of, UINT16 cnt, UINT16 bn)
{
}

void d912pxy_replay_base::QueryMark(d912pxy_query * va, UINT start)
{
}

void d912pxy_replay_base::PrimTopo(D3DPRIMITIVETYPE primType)
{
}

void d912pxy_replay_base::Finish()
{
}

void d912pxy_replay_base::Start()
{
}

void d912pxy_replay_base::IFrameStart()
{
}

void d912pxy_replay_base::IssueWork(UINT batch)
{
}

void d912pxy_replay_base::Replay(UINT start, UINT end, ID3D12GraphicsCommandList * cl, d912pxy_replay_thread * thrd)
{
}

void d912pxy_replay_base::Free()
{
}

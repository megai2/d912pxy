#include "stdafx.h"

d912pxy_replay_passthru::d912pxy_replay_passthru(d912pxy_device * dev) : d912pxy_replay_base(dev)
{
	d912pxy_s(CMDReplay) = this;

	d912pxy_s(GPUque)->EnableGID(CLG_RP1, PXY_INNER_CLG_PRIO_REPLAY);

	cl = d912pxy_s(GPUcl)->GID(CLG_RP1);
}

d912pxy_replay_passthru::~d912pxy_replay_passthru()
{
}

UINT d912pxy_replay_passthru::StateTransit(d912pxy_resource * res, D3D12_RESOURCE_STATES to)
{
	if (res->GetCurrentState() != to)
	{
		res->BTransitTo(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, to, cl);
		return 1;
	}
	else
		return 0;

}

void d912pxy_replay_passthru::OMStencilRef(DWORD ref)
{
	cl->OMSetStencilRef(ref);
}

void d912pxy_replay_passthru::OMBlendFac(float * color)
{
	cl->OMSetBlendFactor(color);
}

void d912pxy_replay_passthru::RSViewScissor(D3D12_VIEWPORT viewport, D3D12_RECT scissor)
{
	cl->RSSetViewports(1, &viewport);
	cl->RSSetScissorRects(1, &scissor);
}

void d912pxy_replay_passthru::PSOCompiled(d912pxy_pso_cache_item * dsc)
{
	psoPtr = dsc->GetPtr();

	if (psoPtr)
		cl->SetPipelineState(psoPtr);
}

void d912pxy_replay_passthru::PSORaw(d912pxy_trimmed_dx12_pso * dsc)
{
	psoPtr = d912pxy_s(psoCache)->UseByDesc(dsc, 0)->GetPtr();

	if (psoPtr)
		cl->SetPipelineState(psoPtr);
}

void d912pxy_replay_passthru::PSORawFeedback(d912pxy_trimmed_dx12_pso * dsc, void ** ptr)
{
	*ptr = (void*)d912pxy_s(psoCache)->UseByDesc(dsc, 0);
}

void d912pxy_replay_passthru::VBbind(d912pxy_vstream * buf, UINT stride, UINT slot, UINT offset)
{
	buf->IFrameBindVB(stride, slot, offset, cl);
}

void d912pxy_replay_passthru::IBbind(d912pxy_vstream * buf)
{
	buf->IFrameBindIB(cl);
}

void d912pxy_replay_passthru::DIIP(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
	if (!psoPtr)
		return;

	d912pxy_s(batch)->PreDIP(cl, StartInstanceLocation);

	//cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cl->DrawIndexedInstanced(
		IndexCountPerInstance,
		InstanceCount,
		StartIndexLocation,
		BaseVertexLocation,
		0
	);
}

void d912pxy_replay_passthru::RT(d912pxy_surface * rtv, d912pxy_surface * dsv)
{
	D3D12_CPU_DESCRIPTOR_HANDLE bindedSurfacesDH[2];
	D3D12_CPU_DESCRIPTOR_HANDLE* bindedRTV = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE* bindedDSV = 0;

	if (rtv)
	{
		bindedRTV = &bindedSurfacesDH[0];
		bindedSurfacesDH[0] = rtv->GetDHeapHandle();
	}

	if (dsv)
	{
		bindedDSV = &bindedSurfacesDH[1];
		bindedSurfacesDH[1] = dsv->GetDHeapHandle();
	}

	if (rtv)
		cl->OMSetRenderTargets(1, bindedRTV, 0, bindedDSV);
	else
		cl->OMSetRenderTargets(0, 0, 0, bindedDSV);
}

void d912pxy_replay_passthru::RTClear(d912pxy_surface * tgt, float * clr)
{
	D3D12_RESOURCE_STATES prevState = tgt->GetCurrentState();
	StateTransit(tgt, D3D12_RESOURCE_STATE_RENDER_TARGET);

	tgt->ClearAsRTV(clr, cl);

	StateTransit(tgt, prevState);
}

void d912pxy_replay_passthru::DSClear(d912pxy_surface * tgt, float depth, UINT8 stencil, D3D12_CLEAR_FLAGS flag)
{
	D3D12_RESOURCE_STATES prevState = tgt->GetCurrentState();
	StateTransit(tgt, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	tgt->ClearAsDSV(depth, stencil, flag, cl);

	StateTransit(tgt, prevState);
}

void d912pxy_replay_passthru::StretchRect(d912pxy_surface * src, d912pxy_surface * dst)
{
	src->BCopyTo(dst, 3, cl);
}

void d912pxy_replay_passthru::Finish()
{
}

void d912pxy_replay_passthru::Start()
{
	psoPtr = NULL;

	d912pxy_s(iframe)->SetRSigOnList(CLG_RP1);

	cl = d912pxy_s(GPUcl)->GID(CLG_RP1);
}

void d912pxy_replay_passthru::IssueWork(UINT batch)
{

}

void d912pxy_replay_passthru::Replay(UINT start, UINT end, ID3D12GraphicsCommandList * cl, d912pxy_replay_thread * thrd)
{
}

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

#ifdef FORCE_REPLAY_MT_SAFETY
	#define REPLAY_SYNC_START drawSync.Hold()
	#define REPLAY_SYNC_STOP drawSync.Release()
	#define REPLAY_SYNC_RETURN(x) drawSync.Release(); return x
#else
	#define REPLAY_SYNC_START
	#define REPLAY_SYNC_STOP 
	#define REPLAY_SYNC_RETURN(x) return x
#endif

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
	REPLAY_SYNC_START;

	if (res->GetCurrentState() != to)
	{
		res->BTransitTo(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, to, cl);
		REPLAY_SYNC_RETURN(1);
	}
	else
		REPLAY_SYNC_RETURN(0);
}

void d912pxy_replay_passthru::OMStencilRef(DWORD ref)
{
	REPLAY_SYNC_START;

	cl->OMSetStencilRef(ref);

	REPLAY_SYNC_STOP;
}

void d912pxy_replay_passthru::OMBlendFac(float * color)
{
	REPLAY_SYNC_START;

	cl->OMSetBlendFactor(color);

	REPLAY_SYNC_STOP;
}

void d912pxy_replay_passthru::RSViewScissor(D3D12_VIEWPORT viewport, D3D12_RECT scissor)
{
	REPLAY_SYNC_START;

	cl->RSSetViewports(1, &viewport);
	cl->RSSetScissorRects(1, &scissor);

	REPLAY_SYNC_STOP;
}

void d912pxy_replay_passthru::PSOCompiled(d912pxy_pso_cache_item * dsc)
{
	psoPtr = dsc->GetPtr();

	if (psoPtr)
	{
		REPLAY_SYNC_START;

		cl->SetPipelineState(psoPtr);

		REPLAY_SYNC_STOP;
	}
}

void d912pxy_replay_passthru::PSORaw(d912pxy_trimmed_dx12_pso * dsc)
{
	psoPtr = d912pxy_s(psoCache)->UseByDesc(dsc, 0)->GetPtr();

	if (psoPtr)
	{
		REPLAY_SYNC_START;

		cl->SetPipelineState(psoPtr);

		REPLAY_SYNC_STOP;
	}
}

void d912pxy_replay_passthru::PSORawFeedback(d912pxy_trimmed_dx12_pso * dsc, void ** ptr)
{
	*ptr = (void*)d912pxy_s(psoCache)->UseByDesc(dsc, 0);
}

void d912pxy_replay_passthru::VBbind(d912pxy_vstream * buf, UINT stride, UINT slot, UINT offset)
{
	REPLAY_SYNC_START;

	buf->IFrameBindVB(stride, slot, offset, cl);

	REPLAY_SYNC_STOP;
}

void d912pxy_replay_passthru::IBbind(d912pxy_vstream * buf)
{
	REPLAY_SYNC_START;

	buf->IFrameBindIB(cl);

	REPLAY_SYNC_STOP;
}

void d912pxy_replay_passthru::DIIP(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation, UINT batchId)
{
	if (!psoPtr)
		return;

	REPLAY_SYNC_START;

	d912pxy_s(batch)->PreDIP(cl, batchId & 0xFFFF);

	cl->IASetPrimitiveTopology((D3D12_PRIMITIVE_TOPOLOGY)(batchId >> 16));
	cl->DrawIndexedInstanced(
		IndexCountPerInstance,
		InstanceCount,
		StartIndexLocation,
		BaseVertexLocation,
		StartInstanceLocation
	);

	REPLAY_SYNC_STOP;
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

	REPLAY_SYNC_START;

	if (rtv)
		cl->OMSetRenderTargets(1, bindedRTV, 0, bindedDSV);
	else
		cl->OMSetRenderTargets(0, 0, 0, bindedDSV);

	REPLAY_SYNC_STOP;
}

void d912pxy_replay_passthru::RTClear(d912pxy_surface * tgt, float * clr, D3D12_VIEWPORT* currentVWP)
{
	REPLAY_SYNC_START;

	D3D12_RESOURCE_STATES prevState = tgt->GetCurrentState();
	StateTransit(tgt, D3D12_RESOURCE_STATE_RENDER_TARGET);

	float clrRemap[4] = { clr[3], clr[2], clr[1], clr[0] };

	D3D12_RECT vwpRect;

	vwpRect.left = (LONG)currentVWP->TopLeftX;
	vwpRect.top = (LONG)currentVWP->TopLeftY;
	vwpRect.right = (LONG)(currentVWP->TopLeftX + currentVWP->Width);
	vwpRect.bottom = (LONG)(currentVWP->TopLeftY + currentVWP->Height);

	tgt->ClearAsRTV(clrRemap, cl, &vwpRect);

	StateTransit(tgt, prevState);

	REPLAY_SYNC_STOP;
}

void d912pxy_replay_passthru::DSClear(d912pxy_surface * tgt, float depth, UINT8 stencil, D3D12_CLEAR_FLAGS flag, D3D12_VIEWPORT* currentVWP)
{
	REPLAY_SYNC_START;

	D3D12_RESOURCE_STATES prevState = tgt->GetCurrentState();
	StateTransit(tgt, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	D3D12_RECT vwpRect;
	
	vwpRect.left = (LONG)currentVWP->TopLeftX;
	vwpRect.top = (LONG)currentVWP->TopLeftY;
	vwpRect.right = (LONG)(currentVWP->TopLeftX + currentVWP->Width);
	vwpRect.bottom = (LONG)(currentVWP->TopLeftY + currentVWP->Height);

	tgt->ClearAsDSV(depth, stencil, flag, cl, &vwpRect);

	StateTransit(tgt, prevState);

	REPLAY_SYNC_STOP;
}

void d912pxy_replay_passthru::StretchRect(d912pxy_surface * src, d912pxy_surface * dst)
{
	REPLAY_SYNC_START;

	src->BCopyTo(dst, 3, cl);

	REPLAY_SYNC_STOP;
}

void d912pxy_replay_passthru::GPUW(UINT32 si, UINT16 of, UINT16 cnt, UINT16 bn)
{
	d912pxy_s(batch)->GPUWriteControl(si, of, cnt, bn);
}

void d912pxy_replay_passthru::Finish()
{
}

void d912pxy_replay_passthru::Start()
{
}

void d912pxy_replay_passthru::IFrameStart()
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

void d912pxy_replay_passthru::Free()
{
	delete this;
}

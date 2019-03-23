#pragma once
#include "stdafx.h"

class d912pxy_replay_passthru :
	public d912pxy_replay_base
{
public:
	d912pxy_replay_passthru(d912pxy_device* dev);
	~d912pxy_replay_passthru();

	//actions

	UINT StateTransit(d912pxy_resource* res, D3D12_RESOURCE_STATES to);
	void OMStencilRef(DWORD ref);
	void OMBlendFac(float* color);
	void RSViewScissor(D3D12_VIEWPORT viewport, D3D12_RECT scissor);

	void PSOCompiled(d912pxy_pso_cache_item* dsc);
	void PSORaw(d912pxy_trimmed_dx12_pso* dsc);
	void PSORawFeedback(d912pxy_trimmed_dx12_pso* dsc, void** ptr);
	void VBbind(d912pxy_vstream* buf, UINT stride, UINT slot, UINT offset);
	void IBbind(d912pxy_vstream* buf);
	void DIIP(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);

	void RT(d912pxy_surface* rtv, d912pxy_surface* dsv);
	void RTClear(d912pxy_surface* tgt, float* clr);
	void DSClear(d912pxy_surface* tgt, float depth, UINT8 stencil, D3D12_CLEAR_FLAGS flag);
	void StretchRect(d912pxy_surface* src, d912pxy_surface* dst);

	//actual execute code and thread managment

	void Finish();
	void Start();
	void IssueWork(UINT batch);

	void Replay(UINT start, UINT end, ID3D12GraphicsCommandList * cl, d912pxy_replay_thread* thrd) ;

private:
	ID3D12GraphicsCommandList* cl;
	ID3D12PipelineState* psoPtr;
};


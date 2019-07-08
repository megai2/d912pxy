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
#pragma once
#include "stdafx.h"

class d912pxy_replay_thread;

class d912pxy_replay_base :
	public d912pxy_noncom
{
public:
	d912pxy_replay_base();
	~d912pxy_replay_base();

	//actions

	virtual UINT StateTransit(d912pxy_resource* res, D3D12_RESOURCE_STATES to);
	virtual void OMStencilRef(DWORD ref);
	virtual void OMBlendFac(float* color);
	virtual void RSViewScissor(D3D12_VIEWPORT viewport, D3D12_RECT scissor);

	virtual void PSOCompiled(d912pxy_pso_cache_item* dsc);
	virtual void PSORaw(d912pxy_trimmed_dx12_pso* dsc);
	virtual void PSORawFeedback(d912pxy_trimmed_dx12_pso* dsc, void** ptr);
	virtual void VBbind(d912pxy_vstream* buf, UINT stride, UINT slot, UINT offset);
	virtual void IBbind(d912pxy_vstream* buf);
	virtual void DIIP(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation, UINT batchId);

	virtual void RT(d912pxy_surface* rtv, d912pxy_surface* dsv);
	virtual void RTClear(d912pxy_surface* tgt, float* clr, D3D12_VIEWPORT* currentVWP);
	virtual void DSClear(d912pxy_surface* tgt, float depth, UINT8 stencil, D3D12_CLEAR_FLAGS flag, D3D12_VIEWPORT* currentVWP);
	virtual void StretchRect(d912pxy_surface* src, d912pxy_surface* dst);
	virtual void GPUW(UINT32 si, UINT16 of, UINT16 cnt, UINT16 bn);

	virtual void QueryMark(d912pxy_query* va, UINT start);

	virtual void PrimTopo(D3DPRIMITIVETYPE primType);

	//actual execute code and thread managment

	virtual void Finish();
	virtual void Start();
	virtual void IFrameStart();
	virtual void IssueWork(UINT batch);

	virtual void Replay(UINT start, UINT end, ID3D12GraphicsCommandList * cl, d912pxy_replay_thread* thrd);

	virtual void Free();
};
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
	void RTClear(d912pxy_surface* tgt, float* clr, D3D12_VIEWPORT* currentVWP);
	void DSClear(d912pxy_surface* tgt, float depth, UINT8 stencil, D3D12_CLEAR_FLAGS flag, D3D12_VIEWPORT* currentVWP);
	void StretchRect(d912pxy_surface* src, d912pxy_surface* dst);

	//actual execute code and thread managment

	void Finish();
	void Start();
	void IssueWork(UINT batch);

	void Replay(UINT start, UINT end, ID3D12GraphicsCommandList * cl, d912pxy_replay_thread* thrd);

	void Free();

private:
	ID3D12GraphicsCommandList* cl;
	ID3D12PipelineState* psoPtr;
};


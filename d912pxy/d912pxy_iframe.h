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

#define PXY_INNER_RBSTACK_SIZE 65535

class d912pxy_iframe : public d912pxy_noncom
{
public:
	d912pxy_iframe(d912pxy_device* dev, d912pxy_dheap** heaps);
	~d912pxy_iframe();

	void RBarrierImm(D3D12_RESOURCE_BARRIER* bar);
	void RBarrierStk(UINT cnt, D3D12_RESOURCE_BARRIER* bar);

	void SetStreamFreq(UINT StreamNumber, UINT Divider);
	void SetVBuf(d912pxy_vstream* vb, UINT StreamNumber, UINT OffsetInBytes, UINT Stride);
	void SetIBuf(d912pxy_vstream* ib);

	d912pxy_vstream* GetIBuf();
	d912pxy_device_streamsrc GetStreamSource(UINT StreamNumber);
		
	void CommitBatch(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);

	void TransitZBufferRW(int write);
	void BindSurface(UINT index, d912pxy_surface* obj);
	void ClearBindedSurfaces();
	d912pxy_surface* GetBindedSurface(UINT index) { return bindedSurfaces[index]; };

	void InstancedVDecl(d912pxy_vdecl* src);
	
	UINT GetInstanceCount() { return instanceCount; };

	void Start();
	void End();

	//non pso states
	void EndSceneReset();
	void SetViewport(D3D12_VIEWPORT* pViewport);
	void SetScissors(D3D12_RECT* pRect);

	D3D12_VIEWPORT* GetViewport() { return &main_viewport; };
	D3D12_RECT* GetScissorRect() { return &main_scissor; };

	void RestoreScissor();
	void IgnoreScissor();

	UINT GetBatchCount() { return batchesIssued; };

	void SetSwapper(d912pxy_swapchain* iSwp) {
		mSwapChain = iSwp;
		d912pxy_s(GPUque)->SetPresenter(iSwp);
	}

	void SetRSigOnList(d912pxy_gpu_cmd_list_group lstID);

	void NoteBindedSurfaceTransit(d912pxy_surface* surf, UINT slot);

	void StateSafeFlush();

	void ForceStateRebind();

	UINT ForceActiveStreams(UINT forceValue);

	UINT GetIndexCount(UINT PrimitiveCount, D3DPRIMITIVETYPE PrimitiveType);

private:	

	void InitRootSignature();

	/*d912pxy_gpu_que* mGPUque;
	d912pxy_gpu_cmd_list* mGPUcl;

	d912pxy_pso_cache* mPSOcache;
	d912pxy_texstage_cache* mTSTcache;
	d912pxy_sampler_cache* mSSTcache;*/

	d912pxy_dheap** mHeaps;
	ID3D12DescriptorHeap* mSetHeapArr[PXY_INNER_MAX_DSC_HEAPS];
	UINT mSetHeapArrCnt;
	d912pxy_swapchain* mSwapChain;
	
	ComPtr<ID3D12RootSignature> mRootSignature;
	UINT batchesIssued;
	
	UINT mRBarrierStkPointer;
	D3D12_RESOURCE_BARRIER mRBarrierStkData[PXY_INNER_RBSTACK_SIZE];

	d912pxy_surface* bindedSurfaces[1+PXY_INNER_MAX_RENDER_TARGETS];
	
	D3D12_CPU_DESCRIPTOR_HANDLE bindedSurfacesDH[1 + PXY_INNER_MAX_RENDER_TARGETS];
	D3D12_CPU_DESCRIPTOR_HANDLE* bindedRTV;
	UINT bindedRTVcount;
	D3D12_CPU_DESCRIPTOR_HANDLE* bindedDSV;

	UINT streamsActive;
	UINT batchCommisionDF;
	UINT instanceCount;
	D3DPRIMITIVETYPE cuPrimType;
	d912pxy_device_streamsrc streamBinds[PXY_INNER_MAX_VBUF_STREAMS];
	d912pxy_vstream* indexBind;

	UINT mCurrentFrameIndex;

	D3D12_VIEWPORT main_viewport;
	D3D12_RECT main_scissor;
	
};


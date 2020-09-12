/*
MIT License

Copyright(c) 2018-2020 megai2

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
	d912pxy_iframe();
	~d912pxy_iframe();

	void Init(d912pxy_dheap** heaps);
	void UnInit();

	void SetStreamFreq(UINT StreamNumber, UINT Divider);
	void SetVBuf(d912pxy_vstream* vb, UINT StreamNumber, UINT OffsetInBytes, UINT Stride);
	void SetIBuf(d912pxy_vstream* ib);

	void SetIBufIfChanged(d912pxy_vstream* ib);
	void SetVBufIfChanged(d912pxy_vstream* vb, UINT StreamNumber, UINT OffsetInBytes, UINT Stride);

	void UpdateActiveStreams(d912pxy_vstream* vb, UINT StreamNumber);

	d912pxy_vstream* GetIBuf();
	d912pxy_device_streamsrc GetStreamSource(UINT StreamNumber);
			
	void CommitBatch(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
	void CommitBatch2(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
	void CommitBatchTailProc(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
	bool CommitBatchHeadProc(D3DPRIMITIVETYPE PrimitiveType);
	void ExtractInstanceCount();
	void ExtractInstanceCountExtra();
	void ExtractBatchDataFromStream(int stream);
	UINT CommitBatchPreCheck(D3DPRIMITIVETYPE PrimitiveType);

	void TransitZBufferRW(int write);
	void BindSurface(UINT index, d912pxy_surface* obj);
	void ClearBindedSurfaces();
	d912pxy_surface* GetBindedSurface(UINT index) { return bindedSurfaces[index]; };

	UINT GetInstanceCount() { return batchCommitData.instanceCount; };

	void Start();
	void End();

	//non pso states
	void EndSceneReset();
	void SetViewport(D3D12_VIEWPORT* pViewport);
	void SetScissors(D3D12_RECT* pRect);
	void SetViewportIfChanged(D3D12_VIEWPORT* pViewport);
	void SetScissorsIfChanged(D3D12_RECT* pRect);

	D3D12_VIEWPORT* GetViewport() { return &main_viewport; };
	D3D12_RECT* GetScissorRect() { return &main_scissor; };

	void RestoreScissor();
	void IgnoreScissor();

	void SetSwapper(d912pxy_swapchain* iSwp);

	void SetRSigOnList(d912pxy_gpu_cmd_list_group lstID);

	void NoteBindedSurfaceTransit(d912pxy_surface* surf, UINT slot);

	void StateSafeFlush(UINT fullFlush);

	void ForceStateRebind();

	UINT ForceActiveStreams(UINT forceValue);

	UINT GetIndexCount(UINT PrimitiveCount, D3DPRIMITIVETYPE PrimitiveType);

	void OptimizeZeroWriteRT(UINT value);

	void ProcessSurfaceBinds(UINT psoOnly);

	D3DPRIMITIVETYPE GetCurrentPrimType() { return cuPrimType; };

	UINT GetActiveStreamCount() { return streamsActive; };

	struct StreamBinds
	{
		d912pxy_device_streamsrc vertex[PXY_INNER_MAX_VBUF_STREAMS];
		d912pxy_vstream* index;
	};

	class StreamBindsHolder
	{
	public:
		StreamBindsHolder();
		~StreamBindsHolder();

	private:
		StreamBinds data;
		d912pxy_iframe& iframe;
	};

	struct BatchCommitData
	{
		UINT batchDF;
		UINT32 instancedModMask;
		UINT instanceCount;
	};

	void OverrideRootSignature(ID3D12RootSignature* newRS);
	void FillPrimaryRSDescriptorRanges(D3D12_DESCRIPTOR_RANGE* ranges);
	void FillPrimaryRSParameters(D3D12_ROOT_PARAMETER* rootParameters, D3D12_DESCRIPTOR_RANGE* ranges);
	void FillPrimaryRSstaticPCFSampler(D3D12_STATIC_SAMPLER_DESC& staticPCF);

private:	
	void InitRootSignature();

	d912pxy_dheap** mHeaps;
	ID3D12DescriptorHeap* mSetHeapArr[PXY_INNER_MAX_DSC_HEAPS];
	UINT mSetHeapArrCnt;
	d912pxy_swapchain* mSwapChain;
	
	ID3D12RootSignature* mRootSignature;
	
	d912pxy_surface* bindedSurfaces[1+PXY_INNER_MAX_RENDER_TARGETS];
	d912pxy_surface* zeroWriteRT;
	
	D3D12_CPU_DESCRIPTOR_HANDLE bindedSurfacesDH[1 + PXY_INNER_MAX_RENDER_TARGETS];
	UINT bindedRTVcount;

	UINT streamsActive;
	UINT batchCommisionDF;	
	D3DPRIMITIVETYPE cuPrimType;
	BatchCommitData batchCommitData;

	StreamBinds streamBinds;

	UINT mCurrentFrameIndex;
	UINT batchLimit;

	D3D12_VIEWPORT main_viewport;
	D3D12_RECT main_scissor;
	
};


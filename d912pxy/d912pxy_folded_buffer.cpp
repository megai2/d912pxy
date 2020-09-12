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

template<class base_element, class sub_element>
void d912pxy_folded_buffer<base_element, sub_element>::Init(UINT maxElements, UINT maxWrites, UINT threads)
{
	ctl.Init(maxWrites, L"folded control");
	data.Init(maxWrites, L"folded data");
	gpuCtl = new d912pxy_cbuffer(sizeof(d912pxy_folded_buffer_control_entry)*maxWrites, false, L"folded gpu control");
	gpuData = new d912pxy_cbuffer(sizeof(sub_element)*maxWrites, false, L"folded gpu data");

	unfolded = new d912pxy_cbuffer(sizeof(base_element)*maxElements, false, L"unfolded buffer");

	unfoldedDevPtrBase = unfolded->DevPtr();
	
	activeThreads = threads;
	lastElements = 0;

	for (UINT i = 0; i != activeThreads; ++i)
		unfoldRanges[i].Init(&ctl);

	InitUnfoldCS();

	maxControlWrites = maxWrites;
	maxUnfoldedElements = maxElements;
}

template<class base_element, class sub_element>
void d912pxy_folded_buffer<base_element, sub_element>::UnInit()
{
	ctl.UnInit();
	data.UnInit();

	delete gpuCtl;
	delete gpuData;
	delete unfolded;

	for (UINT i = 0; i != activeThreads; ++i)
		unfoldRanges[i].UnInit();

	unfoldPSO->Release();
	unfoldRS->Release();
}

template<class base_element, class sub_element>
void d912pxy_folded_buffer<base_element, sub_element>::AddWrite(write_info info, sub_element* src)
{
	data.Push(src, info.i_subCount);

	d912pxy_s.render.replay.DoGPUW(curStreamIdx, info.i_subOffset, info.i_subCount, info.i_element);

	curStreamIdx += info.i_subCount;
}

template<class base_element, class sub_element>
void d912pxy_folded_buffer<base_element, sub_element>::ReplayWrite(UINT thread, UINT streamIndex, write_info info)
{
	for (UINT i = info.i_subOffset; i != (info.i_subOffset + info.i_subCount); ++i)
		unfoldRanges[thread].StartUnfold(streamIndex++, info.i_element, i);
}

template<class base_element, class sub_element>
D3D12_GPU_VIRTUAL_ADDRESS d912pxy_folded_buffer<base_element, sub_element>::GetDevicePtr(UINT element)
{
	return unfoldedDevPtrBase + sizeof(base_element) * element;
}

template<class base_element, class sub_element>
void d912pxy_folded_buffer<base_element, sub_element>::StartWrite()
{
	unfoldRanges[0].NoteGPUTransit(transitSubElements);
	curStreamIdx = transitSubElements;
}

template<class base_element, class sub_element>
void d912pxy_folded_buffer<base_element, sub_element>::EndWrite(UINT elements)
{
	if (elements >= maxUnfoldedElements)
		LOG_ERR_THROW2(-1, "Too much unfolded elements are writed into folded buffer!");

	if (curStreamIdx >= maxControlWrites)
		LOG_ERR_THROW2(-1, "Too much writes performed into folded buffer!");

	for (UINT i = 1; i != activeThreads; ++i)
		unfoldRanges[0].Join(&unfoldRanges[i]);

	UINT gpuGroups = unfoldRanges[0].Finish(elements+1, &curStreamIdx);
		
	ID3D12GraphicsCommandList* cl = d912pxy_s.dx12.cl->GID(CLG_TOP);
	
	CopyDataToGPU(cl);
	DispatchUnfold(cl, gpuGroups);

	lastElements = elements;
}

template<class base_element, class sub_element>
void d912pxy_folded_buffer<base_element, sub_element>::CopyDataToGPU(ID3D12GraphicsCommandList* cl)
{
	gpuCtl->BTransitTo(0, D3D12_RESOURCE_STATE_COPY_DEST, cl);
	gpuData->BTransitTo(0, D3D12_RESOURCE_STATE_COPY_DEST, cl);
	unfolded->BTransitTo(0, D3D12_RESOURCE_STATE_COPY_SOURCE, cl);

	cl->CopyBufferRegion(
		gpuData->GetD12Obj(),
		0,
		unfolded->GetD12Obj(),
		sizeof(base_element) * lastElements,
		sizeof(sub_element) * transitSubElements
	);

	UINT joinOffset[2] = {
		0,
		sizeof(sub_element) * transitSubElements
	};

	joinOffset[0] = ctl.UploadToTarget(cl, gpuCtl, joinOffset[0], curStreamIdx);
	joinOffset[1] = data.UploadToTarget(cl, gpuData, joinOffset[1], curStreamIdx - transitSubElements);
}

template<class base_element, class sub_element>
void d912pxy_folded_buffer<base_element, sub_element>::DispatchUnfold(ID3D12GraphicsCommandList* cl, UINT gpuGroups)
{
	gpuCtl->BTransitTo(0, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, cl);
	gpuData->BTransitTo(0, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, cl);
	unfolded->BTransitTo(0, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, cl);

	cl->SetComputeRootSignature(unfoldRS);
	cl->SetPipelineState(unfoldPSO);
	cl->SetComputeRootUnorderedAccessView(0, unfolded->DevPtr());
	cl->SetComputeRootUnorderedAccessView(1, gpuData->DevPtr());
	cl->SetComputeRootUnorderedAccessView(2, gpuCtl->DevPtr());
	cl->Dispatch(gpuGroups, 1, 1);

	unfolded->BTransitTo(0, D3D12_RESOURCE_STATE_GENERIC_READ, cl);
}

template<class base_element, class sub_element>
void d912pxy_folded_buffer<base_element, sub_element>::InitUnfoldCS()
{
	//copy cs Root signature
	D3D12_ROOT_PARAMETER rootParameters[3] = {
		{ D3D12_ROOT_PARAMETER_TYPE_UAV, {0}, D3D12_SHADER_VISIBILITY_ALL},
		{ D3D12_ROOT_PARAMETER_TYPE_UAV, {0}, D3D12_SHADER_VISIBILITY_ALL},
		{ D3D12_ROOT_PARAMETER_TYPE_UAV, {0}, D3D12_SHADER_VISIBILITY_ALL}
	};

	rootParameters[0].Descriptor = { 0, 0 };
	rootParameters[1].Descriptor = { 1, 0 };
	rootParameters[2].Descriptor = { 2, 0 };

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = { 3, rootParameters, 0, 0, D3D12_ROOT_SIGNATURE_FLAG_NONE };

	unfoldRS = d912pxy_s.dev.ConstructRootSignature(&rootSignatureDesc);

	//copy cs hlsl code
	auto CScodec = d912pxy_shader_replacer(0, 0, 2, 0);
	d912pxy_shader_code CScode = CScodec.GetCodeCS();

	//copy cs PSO
	D3D12_COMPUTE_PIPELINE_STATE_DESC dsc = {
		unfoldRS,
		{CScode.code, CScode.sz},
		0,
		{NULL, 0},
		D3D12_PIPELINE_STATE_FLAG_NONE
	};

	LOG_ERR_THROW2(d912pxy_s.dx12.dev->CreateComputePipelineState(&dsc, IID_PPV_ARGS(&unfoldPSO)), "CS pso creation err");

	//cleanup
	if (!CScode.blob)
		PXY_FREE(CScode.code);
}

template<class base_element, class sub_element>
inline void d912pxy_folded_buffer_unfold_ranges<base_element, sub_element>::UnInit()
{
	PXY_FREE(dltRefs);
	PXY_FREE(dltRefFirstElement);
}

template class d912pxy_folded_buffer<d912pxy_batch_buffer_element, d912pxy_batch_buffer_sub_element>;

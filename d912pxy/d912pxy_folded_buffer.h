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
#pragma once

#pragma pack(push, 1)

typedef struct d912pxy_folded_buffer_control_entry {
	UINT32 subElement;
	union {
		struct {
			UINT32 elementStart;
			UINT32 elementEnd;
		};
		UINT64 elementNums;
	};
	UINT32 unused;
} d912pxy_folded_buffer_control_entry;

#pragma pack(pop)

template<class bufElement>
class d912pxy_folded_buffer_stream {

public:
	d912pxy_folded_buffer_stream() : dataPtr(0) { };
	~d912pxy_folded_buffer_stream() { };

	void Init(UINT eCount, const wchar_t* name)
	{
		UINT size = eCount * sizeof(bufElement);
		cData.Add(new d912pxy_cbuffer(size, true, name));
		cData.Add(new d912pxy_cbuffer(size, true, name));

		dataPtr = cData->HostPtr();
	}

	void UnInit()
	{
		cData.Cleanup();
	}
	
	bufElement* Write(UINT index, bufElement* data)
	{
		bufElement* dst = (bufElement*)(dataPtr + index * sizeof(bufElement));

		memcpy(dst, data, sizeof(bufElement));		

		return dst;
	}

	void Push(bufElement* data, UINT amount)
	{
		UINT sz = amount * sizeof(bufElement);
		memcpy((void*)dataPtr, data, sz);

		dataPtr += sz;
	}

	UINT UploadToTarget(ID3D12GraphicsCommandList* cl, d912pxy_cbuffer* target, UINT targetOffset, UINT eCount)
	{		
		UINT uploadSz = eCount * sizeof(bufElement);
		cData->UploadTarget(cl, target, targetOffset, uploadSz);

		cData.Next();
		dataPtr = cData->HostPtr();

		return targetOffset + uploadSz;
	}

private:	
	d912pxy_swap_list<d912pxy_cbuffer> cData;
	
	intptr_t dataPtr;	
};

template<class base_element, class sub_element>
class d912pxy_folded_buffer_unfold_ranges {
public:
	d912pxy_folded_buffer_unfold_ranges() : i_subElementsCount(sizeof(base_element) / sizeof(sub_element)) {};
	~d912pxy_folded_buffer_unfold_ranges() {};

	void Init(d912pxy_folded_buffer_stream<d912pxy_folded_buffer_control_entry>* controlStream)
	{
		ctlStream = controlStream;
		d912pxy_mem_block::allocZero(&dltRefs, i_subElementsCount);
		d912pxy_mem_block::allocZero(&dltRefFirstElement, i_subElementsCount);
	}

	void UnInit();

	UINT Finish(UINT element, UINT* pStreamIdx)
	{
		for (int i = 0; i != i_subElementsCount; ++i)
		{			
			FinishUnfold(element, i);
			dltRefs[i] = nullptr;
		}		

		UINT streamIdx = *pStreamIdx;

		if (streamIdx & gpuGroupsMask)
		{
			d912pxy_folded_buffer_control_entry ctlEntry;
			ctlEntry.elementStart = 0;
			ctlEntry.elementEnd = 0;

			UINT npb = (streamIdx & ~gpuGroupsMask) + gpuGroupsFix;

			for (UINT i = streamIdx; i != npb; ++i)
			{
				ctlStream->Write(i, &ctlEntry);				
			}

			streamIdx = npb;
		}

		*pStreamIdx = streamIdx;

		return streamIdx >> gpuGroupsShift;
	}
	
	void NoteGPUTransit(UINT elementsTransited)
	{
		for (UINT i = 0; i != elementsTransited;++i)
		{
			StartUnfold(i, 0, i);
		}
	}
	
	void StartUnfold(UINT streamIdx, UINT element, UINT subElement)
	{
		FinishUnfold(element, subElement);
	
		d912pxy_folded_buffer_control_entry ctlEntry;
		ctlEntry.elementStart = element;
		ctlEntry.subElement = subElement;

		dltRefs[subElement] = ctlStream->Write(streamIdx, &ctlEntry);
	}

	void FinishUnfold(UINT element, UINT subElement)
	{
		if (dltRefs[subElement] != nullptr)
		{
			dltRefs[subElement]->elementEnd = element;
		}
		else
			dltRefFirstElement[subElement] = element;
	}

	void Join(d912pxy_folded_buffer_unfold_ranges* obj)
	{
		for (int i = 0; i != i_subElementsCount; ++i)
		{
			if (obj->dltRefs[i] != nullptr)
			{
				FinishUnfold(obj->dltRefFirstElement[i], i);

				dltRefs[i] = obj->dltRefs[i];
				obj->dltRefs[i] = nullptr;
			}
		}		
	}

	const UINT gpuGroupsShift = 5;
	const UINT gpuGroupsFix = 1 << gpuGroupsShift;
	const UINT gpuGroupsMask = gpuGroupsFix - 1;

private:
	const UINT32 i_subElementsCount;

	d912pxy_folded_buffer_stream<d912pxy_folded_buffer_control_entry>* ctlStream;
	d912pxy_folded_buffer_control_entry** dltRefs;
	UINT32* dltRefFirstElement;
};

template<class base_element, class sub_element>
class d912pxy_folded_buffer : public d912pxy_noncom {
public:
	d912pxy_folded_buffer(const UINT subElementsToTransit) : transitSubElements(subElementsToTransit) { };
	~d912pxy_folded_buffer() { };

	class write_info {
		friend d912pxy_folded_buffer;
	public:
		write_info(UINT element, UINT subOffset, UINT subCount) : i_element(element), i_subOffset(subOffset), i_subCount(subCount) { };
		~write_info() { };

	private:
		UINT16 i_element;
		UINT16 i_subOffset;
		UINT16 i_subCount;
	};
	
	void Init(UINT maxElements, UINT maxWrites, UINT threads);
	void UnInit();
					
	void AddWrite(write_info info, sub_element* src);
	void ReplayWrite(UINT thread, UINT streamIndex, write_info info);

	D3D12_GPU_VIRTUAL_ADDRESS GetDevicePtr(UINT element);

	void StartWrite();
	void EndWrite(UINT elements);

private:
	void InitUnfoldCS();
	void CopyDataToGPU(ID3D12GraphicsCommandList* cl);
	void DispatchUnfold(ID3D12GraphicsCommandList* cl, UINT gpuGroups);
	
	ID3D12PipelineState* unfoldPSO;
	ID3D12RootSignature* unfoldRS;

	d912pxy_folded_buffer_stream<d912pxy_folded_buffer_control_entry> ctl;
	d912pxy_folded_buffer_stream<sub_element> data;

	d912pxy_folded_buffer_unfold_ranges<base_element, sub_element> unfoldRanges[PXY_INNER_REPLAY_THREADS_MAX];

	d912pxy_cbuffer* gpuCtl;
	d912pxy_cbuffer* gpuData;
	d912pxy_cbuffer* unfolded;

	UINT curStreamIdx;
	UINT activeThreads;
	UINT lastElements;

	UINT maxControlWrites;
	UINT maxUnfoldedElements;

	D3D12_GPU_VIRTUAL_ADDRESS unfoldedDevPtrBase;

	const UINT transitSubElements;
};

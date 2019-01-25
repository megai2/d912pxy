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

class d912pxy_sampler_cache : public d912pxy_noncom
{
public:
	d912pxy_sampler_cache(d912pxy_device* dev, d912pxy_dheap* sviewHeap, UINT maxCacheNodes);
	~d912pxy_sampler_cache();

	UINT GetDirtyDHeapId(UINT stage);
	UINT GetDHeapId(UINT stage);
	UINT GetDHeapId2(D3D12_SAMPLER_DESC* dsc);

	void ModSampler(UINT stage, D3DSAMPLERSTATETYPE state, DWORD value);

	UINT32 IsDirty();

	D3D12_SAMPLER_DESC* Head() { return &samplers[0]; }

private:
	UINT CreateNewSampler(D3D12_SAMPLER_DESC* cDsc);

	d912pxy_dheap* samplerHeap;
	d912pxy_memtree2* mtree;
	
	UINT32 gDirty;
	UINT32 lastDHeapId[PXY_INNER_MAX_API_SAMPLERS];	
	D3D12_SAMPLER_DESC samplers[PXY_INNER_MAX_API_SAMPLERS];
};


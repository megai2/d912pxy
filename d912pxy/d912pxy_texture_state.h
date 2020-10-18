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

#pragma pack(push, 1)

typedef struct d912pxy_device_texture_state {
	UINT64 dirtyTexRefs;
	d912pxy_basetexture* texRefs[PXY_INNER_MAX_TEXTURE_STAGES];

	UINT64 dirty;	
	UINT texHeapID[PXY_INNER_MAX_TEXTURE_STAGES];
	UINT splHeapID[PXY_INNER_MAX_SHADER_SAMPLERS];
} d912pxy_device_texture_state;

typedef struct d912pxy_trimmed_sampler_dsc {
	UINT16 Dsc0;
	UINT16 Dsc1;
	INT16 MipLODBias;
	UINT16 MinLOD;	
	//UINT32 borderColor;
} d912pxy_trimmed_sampler_dsc;

#pragma pack(pop)

class d912pxy_texture_state : public d912pxy_noncom
{
public:
	d912pxy_texture_state();
	~d912pxy_texture_state();

	void Init();
	void UnInit();

	void SetTextureSrvId(UINT stage, UINT srv);
	void SetTexture(UINT stage, d912pxy_basetexture* texRef);
	void ModStageByMask(UINT stage, UINT srv, UINT mask);
	void ModStageBit(UINT stage, UINT bit, UINT set);
	void ModSamplerTracked(UINT stage, D3DSAMPLERSTATETYPE state, DWORD value);
	void ModSampler(UINT stage, D3DSAMPLERSTATETYPE state, DWORD value);

	UINT Use();
	
	UINT GetTexStage(UINT stage) { return current.texHeapID[stage]; };

	void AddDirtyFlag(DWORD val);

	d912pxy_device_texture_state* GetCurrent() { return &current; };

	void ClearActiveTextures();

	constexpr DWORD MakeDirtyFlagBit(const int stage, const bool sampler)
	{
		if (sampler)
			return (1 << stage) << 8;
		else
			return 1 << (stage >> 2);
	}

	//this function is not thread safe
	UINT LookupSamplerId(const d912pxy_trimmed_sampler_dsc& trimmedDsc);

private:
	UINT LookupSamplerId(UINT stage);

	void UpdateFullSplDsc(const d912pxy_trimmed_sampler_dsc& trimmedSpl);

	UINT CreateNewSampler();

	d912pxy_device_texture_state current;	

	d912pxy_dheap* samplerHeap;	

	typedef d912pxy::Memtree<d912pxy_trimmed_sampler_dsc, uint32_t, d912pxy::Hash32> LookupTable;
	LookupTable splLookup;

	D3D12_SAMPLER_DESC splDsc;
	d912pxy_trimmed_sampler_dsc trimmedSpl[PXY_INNER_MAX_TEXTURE_STAGES];
	
	DWORD DX9SSTValues[D3DSAMP_DMAPOFFSET+1];
};


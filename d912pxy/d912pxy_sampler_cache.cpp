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
#include "stdafx.h"

d912pxy_sampler_cache::d912pxy_sampler_cache(d912pxy_device* dev, d912pxy_dheap* sviewHeap, UINT maxCacheNodes) : d912pxy_noncom(dev, L"sampler bulk data")
{
	d912pxy_s(samplerState) = this;

	samplerHeap = sviewHeap;

	mtree = new d912pxy_memtree2(sizeof(D3D12_SAMPLER_DESC), maxCacheNodes, 0);

	for (int i = 0; i != PXY_INNER_MAX_API_SAMPLERS; ++i)
	{
		samplers[i].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		samplers[i].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].MaxLOD = 1e5;
	}

	gDirty = 0xFFFFFFFF;
}

d912pxy_sampler_cache::~d912pxy_sampler_cache()
{
	mtree->Begin();

	while (!mtree->IterEnd())
	{
		UINT32 cid = mtree->CurrentCID() & 0xFFFFFFFF;
		if (cid)
			samplerHeap->FreeSlot(cid-1);

		mtree->Next();
	}

	delete mtree;
}

UINT d912pxy_sampler_cache::GetDirtyDHeapId(UINT stage)
{
	UINT ret = 0;
	D3D12_SAMPLER_DESC* cDsc = &samplers[stage];
	
	ret = (UINT32)mtree->PointAt32(cDsc);

	if (ret != 0)
	{
		gDirty &= ~(1 << stage);		
		return ret - 1;
	}

	ret = CreateNewSampler(&samplers[stage]);

	gDirty &= ~(1 << stage);

	return ret;
}

UINT d912pxy_sampler_cache::GetDHeapId(UINT stage)
{
	UINT ret = 0;
	D3D12_SAMPLER_DESC* cDsc = &samplers[stage];

	if (gDirty & (1 << stage))
		ret = (UINT32)mtree->PointAt32(cDsc);
	else 
	{
		ret = lastDHeapId[stage];
		if (ret)
			return ret - 1;
	}

	if (ret != 0)
	{
		gDirty &= ~(1 << stage);	
		return ret - 1;
	}

	ret = CreateNewSampler(&samplers[stage]);

	lastDHeapId[stage] = ret+1;
	
	gDirty &= ~(1 << stage);

	return ret;
}

UINT d912pxy_sampler_cache::GetDHeapId2(D3D12_SAMPLER_DESC * dsc)
{
	UINT ret = 0;
	D3D12_SAMPLER_DESC* cDsc = dsc;

	ret = (UINT32)mtree->PointAt32(cDsc);

	if (ret != 0)
	{
		return ret - 1;
	}

	return CreateNewSampler(dsc);
}

void d912pxy_sampler_cache::ModSampler(UINT stage, D3DSAMPLERSTATETYPE state, DWORD value)
{
	stage = (stage >= D3DDMAPSAMPLER) * 16 + (stage & 0xF);

	if (stage >= PXY_INNER_MAX_API_SAMPLERS)
	{
		LOG_ERR_THROW2(-1, "stage >= PXY_INNER_MAX_API_SAMPLERS");
		return;
	}

	D3D12_SAMPLER_DESC* cDesc = &samplers[stage];
	UINT8 dFlag = (gDirty >> stage) & 1;

	switch (state)
	{
		case D3DSAMP_ADDRESSU:
			dFlag |= (cDesc->AddressU != (D3D12_TEXTURE_ADDRESS_MODE)value);
			cDesc->AddressU = (D3D12_TEXTURE_ADDRESS_MODE)value;
			break;
		case D3DSAMP_ADDRESSV:
			dFlag |= (cDesc->AddressV != (D3D12_TEXTURE_ADDRESS_MODE)value);
			cDesc->AddressV = (D3D12_TEXTURE_ADDRESS_MODE)value;
			break;
		case D3DSAMP_ADDRESSW:
			dFlag |= (cDesc->AddressW != (D3D12_TEXTURE_ADDRESS_MODE)value);
			cDesc->AddressW = (D3D12_TEXTURE_ADDRESS_MODE)value;
			break;

		case D3DSAMP_MAXMIPLEVEL:
			dFlag |= (value * 1.0f) != cDesc->MaxLOD;
			cDesc->MinLOD = value * 1.0f;
			break;
		case D3DSAMP_MIPMAPLODBIAS:
			dFlag |= (value * 1.0f) != cDesc->MipLODBias;
			cDesc->MipLODBias = value * 1.0f;
			break;
		case D3DSAMP_SRGBTEXTURE:
			d912pxy_s(textureState)->SetTexStageBit(30, stage, value);
			break;
		case D3DSAMP_BORDERCOLOR:
		{			
			DWORD Color = value;

			float bTmp = 0;
			
			for (int i = 0; i != 4; ++i)
			{
				bTmp = ((Color >> (i << 3)) & 0xFF) / 255.0f;
				dFlag |= cDesc->BorderColor[i] != bTmp;
				cDesc->BorderColor[i] = bTmp;
			}
		}
		break;
		case D3DSAMP_MAGFILTER: //5
		case D3DSAMP_MINFILTER: //6
		case D3DSAMP_MIPFILTER: //7
		{
			
			D3D12_FILTER nFilter = (D3D12_FILTER)((cDesc->Filter & (~(0x7 << ((state - 5) * 3)))  )| (value << ((state - 5) * 3)) );//megai2: ignore dx12 filter type for now and deal with it later

			dFlag |= (nFilter != cDesc->Filter);
			cDesc->Filter = nFilter;
			break;
		}
		case D3DSAMP_MAXANISOTROPY:
			dFlag |= cDesc->MaxAnisotropy != value;
			cDesc->MaxAnisotropy = value;
		break;
		default:
			;
	}

	gDirty |= (dFlag) << stage;
}

UINT32 d912pxy_sampler_cache::IsDirty()
{
	return gDirty;//maybe mask it with PXY_INNER_MAX_API_SAMPLERS bits?
}

UINT d912pxy_sampler_cache::CreateNewSampler(D3D12_SAMPLER_DESC* cDsc)
{
	UINT ret;

	{
		LOG_DBG_DTDM("new sampler f %08lX lmi %.2f lma %.2f lbi %.2f U %u V %u W %u A %u B0 %.3f B1 %.3f B2 %.3f B3 %.3f",
			cDsc->Filter, cDsc->MinLOD, cDsc->MaxLOD, cDsc->MipLODBias, cDsc->AddressU, cDsc->AddressV, cDsc->AddressW, cDsc->MaxAnisotropy,
			cDsc->BorderColor[0], cDsc->BorderColor[1], cDsc->BorderColor[2], cDsc->BorderColor[3]);

		//megai2: handle filter type for real

		DWORD dx9FilterName = cDsc->Filter;

		D3D12_FILTER dx12Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

		if (
			((dx9FilterName & 0x3) == 0x3) |
			((dx9FilterName & 0x18) == 0x18) |
			((dx9FilterName & 0xC0) == 0xC0)   //3 for aniso
			)
		{
			dx12Filter = D3D12_ENCODE_ANISOTROPIC_FILTER(D3D12_FILTER_REDUCTION_TYPE_STANDARD);
		}
		else {
			//0 1 is POINT and LINEAR
			//but in dx9 1 is point and 2 is linear

			UINT minF = ((0x2 & dx9FilterName) != 0);
			UINT magF = ((0x10 & dx9FilterName) != 0);
			UINT mipF = (((0x40 & dx9FilterName) != 0));

			//special hack for PCF filter
			if ((0x7 & dx9FilterName) == 0)
			{
				dx12Filter = D3D12_ENCODE_BASIC_FILTER(
					D3D12_FILTER_TYPE_LINEAR,
					D3D12_FILTER_TYPE_LINEAR,
					D3D12_FILTER_TYPE_LINEAR,
					D3D12_FILTER_REDUCTION_TYPE_COMPARISON
				);
			}
			else {

				dx12Filter = D3D12_ENCODE_BASIC_FILTER(
					minF,
					magF,
					mipF,
					D3D12_FILTER_REDUCTION_TYPE_STANDARD
				);
			}
		}

		cDsc->Filter = dx12Filter;

		ret = samplerHeap->CreateSampler(cDsc);
		++ret;

		cDsc->Filter = (D3D12_FILTER)dx9FilterName;

		mtree->SetValue(ret);
	}

	return ret - 1 ;
}

/*UINT d912pxy_sampler_cache::SaveSampler(UINT64 samplerBits)
{
	d912pxy_samplers_tree* root = &rootInd;
	UINT bitInd = 0;
	UINT fflags = 0;

	while (bitInd != 8)
	{
		UINT64 arrIdx = (samplerBits >> (bitInd << 3));

		if (arrIdx == 0)
			fflags |= 2;

		arrIdx = arrIdx & 0xFF;

		if (root->childs[arrIdx] == 0)
		{
			root->childs[arrIdx] = (d912pxy_samplers_tree*)malloc(sizeof(d912pxy_samplers_tree));
			root->childs[arrIdx]->self = 0;
			root = root->childs[arrIdx];
			fflags |= 1;

			for (int i = 0; i != 256; ++i)
				root->childs[i] = 0;
		}
		else
			root = root->childs[arrIdx];

		if (fflags & 2)
			break;

		++bitInd;
	}

	if (fflags & 1)
	{
		D3D12_SAMPLER_DESC desc;
		desc = DescFromBits(samplerBits);

		dheapIds[mAllocatedSamplers] = heap->CreateSampler(&desc);
		root->self = dheapIds[mAllocatedSamplers] + 1;

		++mAllocatedSamplers;

		LOG_ERR_THROW2(-1 * (mAllocatedSamplers >= PXY_INNER_MAX_API_SAMPLERS), "too many samplers");

		return root->self - 1;
	} else 
		return root->self - 1;
}

UINT d912pxy_sampler_cache::GetDHeapId(UINT64 samplerBits)
{
	d912pxy_samplers_tree* root = &rootInd;
	UINT bitInd = 0;
	UINT fflags = 0;

	while (bitInd != 8)
	{
		UINT64 arrIdx = (samplerBits >> (bitInd << 3));

		if (arrIdx == 0)
			fflags = 2;

		arrIdx = arrIdx & 0xFF;

		if (root->childs[arrIdx] == 0)
			break;
		else
			root = root->childs[arrIdx];

		if (fflags & 2)
			break;

		++bitInd;
	}

	if (fflags & 2)		
		return root->self - 1;
	else 
		return SaveSampler(samplerBits);
		
}

D3D12_SAMPLER_DESC d912pxy_sampler_cache::DescFromBits(UINT64 samplerBits)
{
	D3D12_SAMPLER_DESC ret;

	//0  3 AdrU
	//3  3 AdrV
	//6  3 AdrW
	//9  2 FilterMag
	//11 2 FilterMin
	//13 2 FilterMip
	//15 5 Aniso
	//20 

	//TODO: redo this code
	/*DWORD fshifts[3] = { D3D12_MAG_FILTER_SHIFT, D3D12_MIN_FILTER_SHIFT , D3D12_MIP_FILTER_SHIFT };
	DWORD fshift = fshifts[Type - D3DSAMP_MAGFILTER];

	//megai2: this will reset filter to point type
	samplers[Sampler].Filter = (D3D12_FILTER)(samplers[Sampler].Filter & ~((D3D12_FILTER_TYPE_MASK) << fshift));

	switch (Value)
	{
	case D3DTEXF_POINT:
	samplers[Sampler].Filter = (D3D12_FILTER)(samplers[Sampler].Filter | (((D3D12_FILTER_TYPE_POINT)& D3D12_FILTER_TYPE_MASK) << fshift));
	break;
	case D3DTEXF_LINEAR:
	samplers[Sampler].Filter = (D3D12_FILTER)(samplers[Sampler].Filter | (((D3D12_FILTER_TYPE_LINEAR)& D3D12_FILTER_TYPE_MASK) << fshift));
	break;
	case D3DTEXF_ANISOTROPIC:
	samplers[Sampler].Filter = D3D12_ENCODE_ANISOTROPIC_FILTER(0);
	break;
	default:
	;//megai2: leave it to point type;
	}		

	ret.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

	UINT fMag = ((samplerBits >> 9) & 0x1);
	UINT fMin = ((samplerBits >> 11) & 0x1);
	UINT fMip = ((samplerBits >> 13) & 0x1);

	ret.Filter = D3D12_ENCODE_BASIC_FILTER(fMin, fMag, fMip, 0);

	ret.AddressU = (D3D12_TEXTURE_ADDRESS_MODE)((samplerBits >> 0) & 0x7);
	ret.AddressV = (D3D12_TEXTURE_ADDRESS_MODE)((samplerBits >> 3) & 0x7);
	ret.AddressW = (D3D12_TEXTURE_ADDRESS_MODE)((samplerBits >> 6) & 0x7);
	ret.MipLODBias = 0;
	ret.MaxAnisotropy = (D3D12_TEXTURE_ADDRESS_MODE)((samplerBits >> 15) & 0x1F);
	ret.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	ret.BorderColor[0] = 0.0f;
	ret.BorderColor[1] = 0.0f;
	ret.BorderColor[2] = 0.0f;
	ret.BorderColor[3] = 0.0f;
	ret.MinLOD = 0;
	ret.MaxLOD = 0;

	return ret;
}

UINT64 d912pxy_sampler_cache::ModSamplerBits(UINT64 inBits, D3DSAMPLERSTATETYPE state, DWORD value)
{
	//0  3 AdrU
	//3  3 AdrV
	//6  3 AdrW
	//9  2 FilterMag
	//11 2 FilterMin
	//13 2 FilterMip
	//15 5 Aniso
	//20 

	switch (state)
	{
	case D3DSAMP_ADDRESSU:
	case D3DSAMP_ADDRESSV:
	case D3DSAMP_ADDRESSW:
		inBits = (inBits & ~(0x7 << (state - 1)*3)) | ((value & 0x7) << (state - 1)*3);
		break;

	case D3DSAMP_BORDERCOLOR:
	{
		value = value;
		/*DWORD Color = Value;

		float fvColor[4] =
		{
		((Color >> 24) & 0xFF) / 255.0f,
		((Color >> 16) & 0xFF) / 255.0f,
		((Color >> 8) & 0xFF) / 255.0f,
		((Color >> 0) & 0xFF) / 255.0f
		};

		for (int i = 0; i != 4; ++i)
		samplers[Sampler].BorderColor[i] = fvColor[i];

		;
	}
	break;

	case D3DSAMP_MAGFILTER:
	case D3DSAMP_MINFILTER:
	case D3DSAMP_MIPFILTER:
		inBits = (inBits & ~(0x3 << (9 + (state - 5)*2))) | ((value & 0x3) << (9 + (state - 5)*2));
		break;
	case D3DSAMP_MAXANISOTROPY:
		inBits = (inBits & ~(0x1F << 15)) | ((value & 0x1F) << 15);
		break;
	default:
		;
	}

	return inBits;
}
*/
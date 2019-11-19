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
#include "d912pxy_vstream_pool.h"

d912pxy_vstream_pool::d912pxy_vstream_pool() : d912pxy_pool_memcat<d912pxy_vstream*, d912pxy_vstream_pool*>()
{	 
}

d912pxy_vstream_pool::~d912pxy_vstream_pool()
{
}

void d912pxy_vstream_pool::Init()
{
	memPoolSize = d912pxy_s.config.GetValueUI32(PXY_CFG_POOLING_VSTREAM_ALLOC_STEP);
	memPoolHeapType = D3D12_HEAP_TYPE_DEFAULT;
	memPoolHeapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;

	d912pxy_pool_memcat<d912pxy_vstream*, d912pxy_vstream_pool*>::Init(
		PXY_INNDER_VSTREAM_POOL_BITIGNORE,
		PXY_INNDER_VSTREAM_POOL_BITLIMIT,
		PXY_CFG_POOLING_VSTREAM_LIMITS
	);
}

void d912pxy_vstream_pool::UnInit()
{
	pRunning = 0;

	for (int i = 0; i != PXY_INNDER_VSTREAM_POOL_BITCNT; ++i)
	{
		UINT tsz = 0;

		while (memTable[i]->HaveElements())
		{
			memTable[i]->GetElement()->Release();
			memTable[i]->Next();
			++tsz;
		}

		LOG_DBG_DTDM3("vs pool tbl[%u] = %u", i, tsz);

		delete memTable[i];
	}
	d912pxy_pool_memcat<d912pxy_vstream*, d912pxy_vstream_pool*>::UnInit();
}

d912pxy_vstream * d912pxy_vstream_pool::GetVStreamObject(UINT size, UINT fmt, UINT isIB)
{
	d912pxy_vstream* ret = NULL;
	UINT mc = MemCatFromSize(size);

	LOG_DBG_DTDM("mc %02X sz %08lX szc %08lX mcs %08lX", mc, size, MemCatFromSize(MemCatToSize(mc)), MemCatToSize(mc));

	PoolRW(mc, &ret, 0);

	if (!ret)
	{
		LOG_DBG_DTDM2("vstream pool miss: %u %u %u", size, fmt, isIB);
		PoolRW(mc, &ret, 1);
	}
	else {
		ret->PooledAction(1);
	}
	
	ret->NoteFormatChange(fmt, isIB);

	return ret;
}

d912pxy_vstream * d912pxy_vstream_pool::AllocProc(UINT32 cat)
{
	return d912pxy_vstream::d912pxy_vstream_com(MemCatToSize(cat) , 0, 0, 0);
}

ID3D12Resource * d912pxy_vstream_pool::GetPlacedVStream(UINT32 size)
{
	ID3D12Resource* ret = NULL;

	if (!memPool || (size >= memPoolSize))
	{
	fallback:
		d912pxy_resource* dxBuffer = new d912pxy_resource(RTID_UL_BUF, PXY_COM_OBJ_RESOURCE, L"vstream data");
		dxBuffer->d12res_buffer(size, D3D12_HEAP_TYPE_DEFAULT);
		dxBuffer->Release();

		ret = dxBuffer->GetD12Obj();
		ret->AddRef();
	}
	else {

		D3D12_RESOURCE_DESC rsDesc = {
			D3D12_RESOURCE_DIMENSION_BUFFER,
			0,
			size,
			1,
			1,
			1,
			DXGI_FORMAT_UNKNOWN,
			{1, 0},
			D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
			D3D12_RESOURCE_FLAG_NONE
		};

		ret = CreatePlacedResource(size, &rsDesc, D3D12_RESOURCE_STATE_GENERIC_READ);

		if (!ret)
		{
			LOG_ERR_DTDM("CreatePlacedResource failed with po %llX ps %llX", memPoolOffset, memPoolSize);
			goto fallback;
		}

	}

	return ret;
}

void d912pxy_vstream_pool::EarlyInitProc()
{
}

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

d912pxy_upload_pool::d912pxy_upload_pool(d912pxy_device * dev) : d912pxy_pool_memcat<d912pxy_upload_item*>(dev, PXY_INNDER_UPLOAD_POOL_BITIGNORE, PXY_INNDER_UPLOAD_POOL_BITLIMIT)
{
	d912pxy_s(pool_upload) = this;

	dxBuffer = new d912pxy_resource(m_dev, RTID_UL_BUF, L"upload pool data");
}

d912pxy_upload_pool::~d912pxy_upload_pool()
{
	d912pxy_s(pool_upload) = NULL;

	dxBuffer->Release();	

	for (int i = 0; i != PXY_INNDER_UPLOAD_POOL_BITCNT; ++i)
	{
		UINT tsz = 0;

		while (memTable[i]->HaveElements())
		{
			memTable[i]->GetElement()->Release();
			memTable[i]->Next();
			++tsz;
		}

		LOG_DBG_DTDM3("ul pool tbl[%u] = %u", i, tsz);

		delete memTable[i];		
	}

}

d912pxy_upload_item * d912pxy_upload_pool::GetUploadObject(UINT size)
{
	d912pxy_upload_item * ret = NULL;
	UINT mc = MemCatFromSize(size);
	PoolRW(mc, &ret, 0);

	if (!ret)
	{
		PoolRW(mc, &ret, 1);
	}
	else {
		LOG_DBG_DTDM2("upload obj reuse %u-%u", mc, size);
		ret->PooledAction(1);			
	}

	return ret;
}

d912pxy_upload_item * d912pxy_upload_pool::AllocProc(UINT32 cat)
{
	d912pxy_upload_item * ret;

	EnterCriticalSection(&allocMutex);

	ret = new d912pxy_upload_item(m_dev, cat);

	LeaveCriticalSection(&allocMutex);

	return ret;
}


ID3D12Resource * d912pxy_upload_pool::MakeUploadBuffer(UINT maxSize)
{
	maxSize = MemCatToSize(maxSize);
	dxBuffer->d12res_buffer(maxSize, D3D12_HEAP_TYPE_UPLOAD);
	dxBuffer->GetD12Obj().Get()->AddRef();
	return dxBuffer->GetD12Obj().Get();
}

d912pxy_upload_item::d912pxy_upload_item(d912pxy_device * dev, UINT8 icat) : d912pxy_comhandler(dev, L"upload item")
{
	cat = icat;
	recon = 0;
	mRes = d912pxy_s(pool_upload)->MakeUploadBuffer(cat);
	LOG_ERR_THROW2(mRes->Map(0, 0, (void**)&mappedMemWofs), "upload pool memory map error on creation");
}

d912pxy_upload_item::~d912pxy_upload_item()
{
	if (recon)
		free(recon);
}

void d912pxy_upload_item::UploadTarget(ID3D12Resource * res, UINT64 dofs, UINT64 sz, ID3D12GraphicsCommandList * cl)
{
	cl->CopyBufferRegion(res, dofs, mRes, dofs, sz);
}

void * d912pxy_upload_item::MapDPtr()
{
	//d912pxy_s(DXDev)->MakeResident(1, (ID3D12Pageable**)&mRes);

	return (void*)(mappedMemWofs);
}

void * d912pxy_upload_item::MapRPtr()
{
	if (!recon)
		recon = malloc(d912pxy_s(pool_upload)->MemCatToSize(cat));

	return recon;
}

void d912pxy_upload_item::Reconstruct(void* mem, UINT64 rowPitch, UINT64 height, UINT64 size, const D3D12_RANGE * wofs)
{
	intptr_t bufferRef = (intptr_t)MapDPtr();
	intptr_t srcm = (intptr_t)mem;
		
	//megai2: well..
	for (int i = 0; i != height; ++i)
	{
		memcpy((void*)bufferRef, (void*)srcm, size);

		bufferRef += rowPitch;
		srcm = srcm + size;
	}
}

UINT d912pxy_upload_item::FinalReleaseCB()
{
	if (d912pxy_s(pool_upload))
	{
	//	d912pxy_s(DXDev)->Evict(1, (ID3D12Pageable**)&mRes);

		d912pxy_upload_item* tv = this;
		d912pxy_s(pool_upload)->PoolRW(cat, &tv, 1);
		return 0;
	}
	else {
		if (mRes)
			mRes->Release();		
		return 1;
	}
}

UINT32 d912pxy_upload_item::PooledAction(UINT32 use)
{
	d912pxy_s(pool_upload)->PooledActionLock();

	if (!d912pxy_comhandler::PooledAction(use))
	{
		d912pxy_s(pool_upload)->PooledActionUnLock();

		return 0;
	}

	if (use)
	{
		mRes = d912pxy_s(pool_upload)->MakeUploadBuffer(cat);
		LOG_ERR_THROW2(mRes->Map(0, 0, (void**)&mappedMemWofs), "upload pool memory map error");
	}
	else {

		if (recon)
		{
			free(recon);
			recon = 0;
		}

		mRes->Release();
		mRes = NULL;

		mappedMemWofs = NULL;
	}

	d912pxy_s(pool_upload)->PooledActionUnLock();

	return 0;
}

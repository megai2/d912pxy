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

class d912pxy_upload_pool;

class d912pxy_upload_item : public d912pxy_comhandler {
public:
	d912pxy_upload_item(UINT8 icat);
	~d912pxy_upload_item();

	void UploadBlockWrite(UINT64 dst_offset, UINT64 upload_offset, UINT64 sz, void* src);
	void UploadBlock(ID3D12Resource * res, UINT64 dst_offset, UINT64 upload_offset, UINT64 sz, void* src, ID3D12GraphicsCommandList* cl);

	void UploadTargetWithOffset(ID3D12Resource * res, UINT64 sofs, UINT64 dofs, UINT64 sz, void* src, ID3D12GraphicsCommandList* cl);
	void UploadTarget(ID3D12Resource* res, UINT64 dofs, UINT64 sz, void* src, ID3D12GraphicsCommandList* cl);
	
	void Reconstruct(void* mem, UINT64 rowPitch, UINT64 height, UINT64 size, UINT64 upload_offset, const D3D12_RANGE* wofs);

	ID3D12Resource* GetResourcePtr() { return mRes; };

	UINT FinalReleaseCB();

	UINT32 PooledAction(UINT32 use);

	UINT HaveFreeSpace(UINT32 size);

	intptr_t GetSize() { return space; };

	intptr_t GetCurrentOffset() { return usedSpace; };

	void AddSpaceUsed(intptr_t amount) { usedSpace += amount; };
	   
private:	
	intptr_t DPtr();

	intptr_t DPtrOffset(UINT64 offset);

	intptr_t mappedMemWofs;

	intptr_t usedSpace;
	intptr_t space;

	ID3D12Resource* mRes;

	UINT8 cat;
};

//start with 2^20 (1mB) end with 2^28(256mB)
//this will allow creating buffers with size up to 256mB
#define PXY_INNDER_UPLOAD_POOL_BITIGNORE 20
#define PXY_INNDER_UPLOAD_POOL_BITLIMIT 28
#define PXY_INNDER_UPLOAD_POOL_BITCNT (PXY_INNDER_UPLOAD_POOL_BITLIMIT - PXY_INNDER_UPLOAD_POOL_BITIGNORE)


class d912pxy_upload_pool : public d912pxy_pool_memcat<d912pxy_upload_item*, d912pxy_upload_pool*>
{
public:
	d912pxy_upload_pool();
	~d912pxy_upload_pool();

	void Init();
	void UnInit();

	d912pxy_upload_item* GetUploadObject(UINT size);
	ID3D12Resource* MakeUploadBuffer(UINT maxSize);

	d912pxy_upload_item* AllocProc(UINT32 cat);	

	void EarlyInitProc();	
};


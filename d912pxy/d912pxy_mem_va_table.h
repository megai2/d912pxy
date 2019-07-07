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
#pragma once
#include "stdafx.h"

#define d912pxy_mem_va_table_obj_id UINT32
#define d912pxy_mem_va_table_ref_counter UINT32

typedef struct d912pxy_mem_va_table_entry
{
	UINT64 itemSize;
	d912pxy_mem_va_table_obj_id* stackBase;
	d912pxy_mem_va_table_obj_id* stackPtr;
	d912pxy_mem_va_table_obj_id* stackLimit;
	d912pxy_mem_va_table_ref_counter* refBase;
	UINT64 refCount;
	UINT64 allocGrain;	
} d912pxy_mem_va_table_entry;

#define PXY_INNER_MAX_VA_TABLE_ENTRYS 32

class d912pxy_mem_va_table : public d912pxy_noncom
{
public:
	d912pxy_mem_va_table();
	~d912pxy_mem_va_table();

	void Init(UINT64* objSizes, UINT64 allocBitSize, UINT64 entryCount);
	void DeInit();

	void* AllocateObj(UINT64 type);
	void DeAllocateObj(void* obj);

	UINT64 TypeFromAdr(void* obj);
	d912pxy_mem_va_table_obj_id ObjIdFromAdr(void* obj);
	d912pxy_mem_va_table_obj_id ObjIdFromAdr2(void* obj, UINT64 type);

private:
	intptr_t baseAdr;
	UINT64 entryCount;
	UINT64 entryShift;
	UINT64 entryBase;
	UINT64 baseMask;
	UINT64 objShift;
	UINT64 objMask;

	d912pxy_mem_va_table_entry table[PXY_INNER_MAX_VA_TABLE_ENTRYS];
	d912pxy_thread_lock lock[PXY_INNER_MAX_VA_TABLE_ENTRYS];
};

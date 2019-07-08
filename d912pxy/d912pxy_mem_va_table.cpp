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
#include "d912pxy_mem_va_table.h"

d912pxy_mem_va_table::d912pxy_mem_va_table() : d912pxy_noncom()
{
}

d912pxy_mem_va_table::~d912pxy_mem_va_table()
{
}

void d912pxy_mem_va_table::Init(UINT64* objSizes, UINT64 allocBitSize, UINT64 entryCount)
{
	NonCom_Init(L"mem_va_table");

	if (allocBitSize <= 23)
	{
		LOG_ERR_THROW2(-1, "VA table allocation size is too small");
	}

	UINT64 pageSz = d912pxy_s.mem.GetPageSize();

	entryShift = d912pxy_helper::GetClosestPow2(entryCount);
	entryBase = 1ULL << (allocBitSize - entryShift);
	baseMask = (1ULL << allocBitSize) - 1;
	objShift = allocBitSize - entryShift;
	objMask = (1ULL << objShift) - 1;

	LOG_DBG_DTDM3("Marking up %u Mb by %u Mb groups", 1ULL << (allocBitSize - 20), entryBase >> 20);

	UINT64 manageRegionSz = 0;

	for (int i = 0; i != entryCount; ++i)
	{
		table[i].itemSize = objSizes[i];

		table[i].allocGrain = pageSz * table[i].itemSize;

		table[i].refCount = entryBase / table[i].allocGrain;

		manageRegionSz += table[i].refCount * (pageSz * sizeof(d912pxy_mem_va_table_obj_id) + sizeof(d912pxy_mem_va_table_ref_counter));		

		LOG_DBG_DTDM3("table[%u] allocGrain = %X Kb | refCount = %X | totalObjects = %X ", i, table[i].allocGrain >> 10, table[i].refCount, entryBase / table[i].itemSize);
	}

	LOG_INFO_DTDM("reserving %u mb VA space (%u mb managment)", ((1ULL << allocBitSize) + manageRegionSz) >> 20, manageRegionSz >> 20);

	baseAdr = d912pxy_s.mem.ReserveVARangeAligned(allocBitSize, manageRegionSz);

	intptr_t manageBase = baseAdr + (1ULL << allocBitSize);

	d912pxy_s.mem.CommitVARange(manageBase, manageRegionSz);

	for (int i = 0; i != entryCount; ++i)
	{
		table[i].stackBase = (d912pxy_mem_va_table_obj_id*)manageBase;
		table[i].stackPtr = table[i].stackBase;

		d912pxy_mem_va_table_obj_id objId = (d912pxy_mem_va_table_obj_id)(table[i].refCount * pageSz);
		manageBase += objId * sizeof(d912pxy_mem_va_table_obj_id);
		table[i].stackLimit = (d912pxy_mem_va_table_obj_id*)manageBase;		

		while (objId != 0)
		{
			--objId;
			table[i].stackPtr[objId] = objId;						
		}			
	}

	for (int i = 0; i != entryCount; ++i)
	{
		table[i].refBase = (d912pxy_mem_va_table_ref_counter*)manageBase;

		manageBase += table[i].refCount * sizeof(d912pxy_mem_va_table_ref_counter);		
	}
}

void d912pxy_mem_va_table::DeInit()
{
	d912pxy_s.mem.ReleaseReservedVARange(baseAdr);
}

void * d912pxy_mem_va_table::AllocateObj(UINT64 type)
{	
	UINT64 groupBase = baseAdr + type * entryBase;

	lock[type].Hold();

	if (table[type].stackPtr == table[type].stackLimit)
	{
		LOG_ERR_DTDM("Type %u is full (%u items max)", type, table[type].refCount * table[type].allocGrain / table[type].itemSize);
		LOG_ERR_THROW2(-1, "Can't allocate VA table based object");
	}

	d912pxy_mem_va_table_obj_id objId = table[type].stackPtr[0];
	++table[type].stackPtr;

	intptr_t ret = objId * table[type].itemSize;

	UINT64 refId = ret / table[type].allocGrain;

	d912pxy_mem_va_table_ref_counter refs = ++table[type].refBase[refId];

	if (refs == 1)
		d912pxy_s.mem.CommitVARange(groupBase + refId * table[type].allocGrain, table[type].allocGrain);	

	lock[type].Release();

	ret += groupBase;
	return (void*)ret;	
}

void d912pxy_mem_va_table::DeAllocateObj(void * obj)
{
	UINT64 type = TypeFromAdr(obj);
	UINT64 objAdr = (intptr_t)obj & objMask;
	d912pxy_mem_va_table_obj_id objID = (UINT32)(objAdr / table[type].itemSize);	
	UINT64 refId = objAdr / table[type].allocGrain;

	lock[type].Hold();

	--table[type].stackPtr;
	table[type].stackPtr[0] = objID;

	if (!--table[type].refBase[refId])	
		d912pxy_s.mem.DeCommitVARange(refId * table[type].allocGrain + baseAdr + type * entryBase, table[type].allocGrain);

	lock[type].Release();
}

UINT64 d912pxy_mem_va_table::TypeFromAdr(void * obj)
{
	return ((intptr_t)obj & baseMask) >> objShift;
}

d912pxy_mem_va_table_obj_id d912pxy_mem_va_table::ObjIdFromAdr(void * obj)
{
	return ObjIdFromAdr2(obj, TypeFromAdr(obj));
}

d912pxy_mem_va_table_obj_id d912pxy_mem_va_table::ObjIdFromAdr2(void * obj, UINT64 type)
{
	return (d912pxy_mem_va_table_obj_id)(((intptr_t)obj & objMask) / table[type].itemSize);
}

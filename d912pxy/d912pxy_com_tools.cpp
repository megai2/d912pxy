#include "stdafx.h"

typedef struct comObjVTable {
	void* functions[1024];
} comObjVTable;

typedef struct comObj {
	comObjVTable* vtbl;
	UINT64 other;
} comObj;

void d912pxy_com_set_method(void * objPtr, UINT32 methodIdx, void* newMethod)
{
	comObj* obj = (comObj*)objPtr;

	DWORD oldVP;
	VirtualProtect(&obj->vtbl->functions[methodIdx], 0x8, PAGE_EXECUTE_READWRITE, &oldVP);

	obj->vtbl->functions[methodIdx] = newMethod;

	VirtualProtect(&obj->vtbl->functions[methodIdx], 0x8, oldVP, &oldVP);
}

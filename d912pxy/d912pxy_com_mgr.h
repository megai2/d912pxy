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

typedef enum d912pxy_com_obj_typeid {	
	PXY_COM_OBJ_VSTREAM = 0,	
	PXY_COM_OBJ_SURFACE = 1,
	PXY_COM_OBJ_QUERY = 2,
	PXY_COM_OBJ_QUERY_OCC = 3,
	PXY_COM_OBJ_TEXTURE = 4,		
	PXY_COM_OBJ_TEXTURE_RTDS = 5,	
	PXY_COM_OBJ_VDECL = 6,
	PXY_COM_OBJ_SHADER = 7,
	PXY_COM_OBJ_SWAPCHAIN = 8,
	PXY_COM_OBJ_SURFACE_LAYER = 9,
	PXY_COM_OBJ_SBLOCK = 10,
	PXY_COM_OBJ_PSO_ITEM = 11,
	PXY_COM_OBJ_COUNT = 12,
	PXY_COM_OBJ_NOVTABLE = 13,
	PXY_COM_OBJ_STATIC = 14,
	PXY_COM_OBJ_RESOURCE = 15,
} d912pxy_com_obj_typeid;

#define PXY_COM_OBJ_SIGNATURE_SURFACE_LAYER 0x80000000
#define PXY_COM_OBJ_SIGNATURE_TEXTURE_RTDS 0x10000000

#define PXY_COM_OBJ_UNMANAGED PXY_COM_OBJ_COUNT

#define PXY_INNER_COM_MGR_VA_MASK_BITS 32

class d912pxy_com_mgr : public d912pxy_noncom
{
public:
	d912pxy_com_mgr();
	~d912pxy_com_mgr();

	void Init();
	void DeInit();

	d912pxy_com_object* AllocateComObj(d912pxy_com_obj_typeid type);
	void DeAllocateComObj(d912pxy_com_object* obj);

#if _WIN64
	d912pxy_com_object* GetComObject(d912pxy_com_obj_typeid type, d912pxy_mem_va_table_obj_id id);
	d912pxy_com_object* GetComObjectByLowAdr(UINT32 lowAdr);
	d912pxy_com_obj_typeid GetTypeIdFromAdr(d912pxy_com_object* obj);
#endif

private:
	d912pxy_mem_va_table table;
};


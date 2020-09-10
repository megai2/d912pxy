/*
MIT License

Copyright(c) 2019-2020 megai2

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

d912pxy_com_mgr::d912pxy_com_mgr()
{
}


d912pxy_com_mgr::~d912pxy_com_mgr()
{

}

void d912pxy_com_mgr::Init()
{
	NonCom_Init(L"com mgr");

	d912pxy_com_route_init_default();

	UINT64 addedSize = 8;

	UINT64 va_objSizes[] = {
		sizeof(d912pxy_vstream) + addedSize,
		sizeof(d912pxy_surface) + addedSize,
		sizeof(d912pxy_query) + addedSize,
		sizeof(d912pxy_query_occlusion) + addedSize,
		sizeof(d912pxy_ctexture) + addedSize,
		sizeof(d912pxy_ctexture) + addedSize,
		sizeof(d912pxy_vdecl) + addedSize,
		sizeof(d912pxy_shader) + addedSize,
		sizeof(d912pxy_swapchain) + addedSize,
		sizeof(d912pxy_surface_layer) + addedSize,
		sizeof(d912pxy_sblock) + addedSize,
		sizeof(d912pxy_pso_item) + addedSize
	};

	table.Init(va_objSizes, PXY_INNER_COM_MGR_VA_MASK_BITS, PXY_COM_OBJ_COUNT);
}

void d912pxy_com_mgr::DeInit()
{
	table.DeInit();
	this->~d912pxy_com_mgr();
}

d912pxy_com_object * d912pxy_com_mgr::AllocateComObj(d912pxy_com_obj_typeid type)
{	
	return (d912pxy_com_object*)table.AllocateObj(type);
}

void d912pxy_com_mgr::DeAllocateComObj(d912pxy_com_object * obj)
{
	table.DeAllocateObj(obj);
}

#if _WIN64

d912pxy_com_object * d912pxy_com_mgr::GetComObject(d912pxy_com_obj_typeid type, d912pxy_mem_va_table_obj_id id)
{
	return (d912pxy_com_object*)table.GetObj(type, id);
}

d912pxy_com_object * d912pxy_com_mgr::GetComObjectByLowAdr(UINT32 lowAdr)
{
	return (d912pxy_com_object*)((intptr_t)table.GetBaseAdr() | lowAdr);
}

d912pxy_com_obj_typeid d912pxy_com_mgr::GetTypeIdFromAdr(d912pxy_com_object * obj)
{
	return (d912pxy_com_obj_typeid)table.TypeFromAdr(obj);
}

#endif

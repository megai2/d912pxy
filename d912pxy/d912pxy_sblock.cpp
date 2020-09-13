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

d912pxy_sblock::d912pxy_sblock(D3DSTATEBLOCKTYPE Type)
	: d912pxy_comhandler(PXY_COM_OBJ_SBLOCK, L"state block")
	, m_type(Type)
{
}

d912pxy_sblock * d912pxy_sblock::d912pxy_sblock_com(D3DSTATEBLOCKTYPE Type)
{
	d912pxy_com_object* ret = d912pxy_s.com.AllocateComObj(PXY_COM_OBJ_SBLOCK);
	
	new (&ret->sblock)d912pxy_sblock(Type);
	ret->vtable = d912pxy_com_route_get_vtable(PXY_COM_ROUTE_SBLOCK);

	return &ret->sblock;
}

d912pxy_sblock::~d912pxy_sblock()
{

}

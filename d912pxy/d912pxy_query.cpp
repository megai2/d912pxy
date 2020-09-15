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

d912pxy_query::d912pxy_query(D3DQUERYTYPE Type) : d912pxy_comhandler(PXY_COM_OBJ_QUERY,L"query"), m_type(Type)
{ }


d912pxy_query_non_derived * d912pxy_query::d912pxy_query_com(D3DQUERYTYPE Type)
{
	d912pxy_com_object* ret = d912pxy_s.com.AllocateComObj(PXY_COM_OBJ_QUERY);
	
	new (&ret->query)d912pxy_query_non_derived(Type);
	ret->vtable = d912pxy_com_route_get_vtable(PXY_COM_ROUTE_QUERY);

	return &ret->query;
}

d912pxy_query::~d912pxy_query()
{

}

#define D912PXY_METHOD_IMPL_CN d912pxy_query

D912PXY_METHOD_IMPL_NC_(D3DQUERYTYPE, GetType)(THIS)
{
	return m_type;
}

D912PXY_METHOD_IMPL_NC_(DWORD, GetDataSize)(THIS)
{
	LOG_DBG_DTDM(__FUNCTION__);

	return 4;
}

D912PXY_METHOD_IMPL_NC(Issue)(THIS_ DWORD dwIssueFlags)
{
	LOG_DBG_DTDM(__FUNCTION__);

	return D3D_OK;
}

D912PXY_METHOD_IMPL_NC(GetData)(THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags)
{
	LOG_DBG_DTDM(__FUNCTION__);

	switch (m_type)
	{
	case D3DQUERYTYPE_TIMESTAMPDISJOINT:
		if (dwSize == 4)
			((DWORD*)pData)[0] = 0;
		break;
	case D3DQUERYTYPE_TIMESTAMPFREQ:
		if (dwSize == 8)
			d912pxy_s.dx12.que.GetDXQue()->GetTimestampFrequency((UINT64*)pData);
		break;
	case D3DQUERYTYPE_TIMESTAMP:
		QueryPerformanceCounter((LARGE_INTEGER*)pData);
		break;
	default:
		if (dwSize == 4)
			((DWORD*)pData)[0] = 1;
	}

	return S_OK;
}

D912PXY_METHOD_IMPL_NC(GetDataZeroOverride)(THIS_ void * pData, DWORD dwSize, DWORD dwGetDataFlags)
{
	if (dwSize == 4)
		((DWORD*)pData)[0] = 0;

	return S_OK;
}

D912PXY_METHOD_IMPL_NC(GetDataOneOverride)(THIS_ void * pData, DWORD dwSize, DWORD dwGetDataFlags)
{
	if (dwSize == 4)
		((DWORD*)pData)[0] = 1;

	return S_OK;
}

D912PXY_METHOD_IMPL_NC(IssueNOP)(THIS_ DWORD dwIssueFlags)
{
	return D3D_OK;
}

#undef D912PXY_METHOD_IMPL_CN

void d912pxy_query::QueryMark(UINT start, ID3D12GraphicsCommandList* cl)
{
}

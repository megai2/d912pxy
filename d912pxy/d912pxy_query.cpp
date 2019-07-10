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

d912pxy_query::d912pxy_query(d912pxy_device* dev, D3DQUERYTYPE Type) : d912pxy_comhandler(L"query")
{
	m_dev = dev;
	m_type = Type;
}


d912pxy_query::~d912pxy_query()
{

}

#define D912PXY_METHOD_IMPL_CN d912pxy_query

D912PXY_IUNK_IMPL

/*** IDirect3DQuery9 methods ***/
D912PXY_METHOD_IMPL(GetDevice)(THIS_ IDirect3DDevice9** ppDevice)
{
	*ppDevice = (IDirect3DDevice9*)m_dev;

	return D3D_OK;
}

D912PXY_METHOD_IMPL_(D3DQUERYTYPE, GetType)(THIS)
{
	return m_type;
}

D912PXY_METHOD_IMPL_(DWORD, GetDataSize)(THIS)
{
	LOG_DBG_DTDM(__FUNCTION__);

	return 4;
}

D912PXY_METHOD_IMPL(Issue)(THIS_ DWORD dwIssueFlags)
{
	LOG_DBG_DTDM(__FUNCTION__);

	return D3D_OK;
}

D912PXY_METHOD_IMPL(GetData)(THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags)
{
	LOG_DBG_DTDM(__FUNCTION__);

	if (dwSize == 4)
		((DWORD*)pData)[0] = 1;

	return S_OK;
}

#undef D912PXY_METHOD_IMPL_CN

void d912pxy_query::QueryMark(UINT start, ID3D12GraphicsCommandList* cl)
{
}

D912PXY_METHOD(d912pxy_query::GetDataZeroOverride)(IDirect3DQuery9 * self, void * pData, DWORD dwSize, DWORD dwGetDataFlags)
{
	if (dwSize == 4)
		((DWORD*)pData)[0] = 0;
	
	return S_OK;
}

D912PXY_METHOD(d912pxy_query::GetDataOneOverride)(IDirect3DQuery9 * self, void * pData, DWORD dwSize, DWORD dwGetDataFlags)
{
	if (dwSize == 4)
		((DWORD*)pData)[0] = 1;

	return S_OK;
}

D912PXY_METHOD(d912pxy_query::IssueNOP)(IDirect3DQuery9 * self, DWORD dwIssueFlags)
{
	return D3D_OK;
}

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

#define D3DISSUE_FORCED 0x4

class d912pxy_query_non_derived;

class d912pxy_query : public d912pxy_comhandler
{
public:	
	static d912pxy_query_non_derived* d912pxy_query_com(D3DQUERYTYPE Type);
	d912pxy_query(D3DQUERYTYPE Type);
	~d912pxy_query();

	D912PXY_METHOD_(D3DQUERYTYPE, GetType)(PXY_THIS);
	D912PXY_METHOD_(DWORD, GetDataSize)(PXY_THIS);
	D912PXY_METHOD(Issue)(PXY_THIS_ DWORD dwIssueFlags);
	D912PXY_METHOD(GetData)(PXY_THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags);	

	D912PXY_METHOD(GetDataZeroOverride)(PXY_THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags);
	D912PXY_METHOD(GetDataOneOverride)(PXY_THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags);
	D912PXY_METHOD(IssueNOP)(PXY_THIS_ DWORD dwIssueFlags);

	D912PXY_METHOD_NC_(D3DQUERYTYPE, GetType)(THIS);
	D912PXY_METHOD_NC_(DWORD, GetDataSize)(THIS);
	D912PXY_METHOD_NC(Issue)(THIS_ DWORD dwIssueFlags);
	D912PXY_METHOD_NC(GetData)(THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags);

	D912PXY_METHOD_NC(GetDataZeroOverride)(THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags);
	D912PXY_METHOD_NC(GetDataOneOverride)(THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags);
	D912PXY_METHOD_NC(IssueNOP)(THIS_ DWORD dwIssueFlags);

	virtual void QueryMark(UINT start, ID3D12GraphicsCommandList* cl);

private:		

	D3DQUERYTYPE m_type;
};

class d912pxy_query_non_derived : public d912pxy_vtable, public d912pxy_query
{
public:
	d912pxy_query_non_derived(D3DQUERYTYPE Type) : d912pxy_query(Type) { ; };
};
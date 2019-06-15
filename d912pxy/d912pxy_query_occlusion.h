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

class d912pxy_query_occlusion : public d912pxy_query
{
public:
	d912pxy_query_occlusion(d912pxy_device* dev, D3DQUERYTYPE Type);
	~d912pxy_query_occlusion();

	/*** IUnknown methods ***/
	D912PXY_METHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	D912PXY_METHOD_(ULONG, AddRef)(THIS);
	D912PXY_METHOD_(ULONG, Release)(THIS);

	/*** IDirect3DQuery9 methods ***/
	D912PXY_METHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice);
	D912PXY_METHOD_(D3DQUERYTYPE, GetType)(THIS);
	D912PXY_METHOD_(DWORD, GetDataSize)(THIS);
	D912PXY_METHOD(Issue)(THIS_ DWORD dwIssueFlags);
	D912PXY_METHOD(GetData)(THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags);	

	void QueryMark(UINT start, ID3D12GraphicsCommandList* cl);

	void FlushQueryStack();

	static UINT InitOccQueryEmulation();
	static void DeInitOccQueryEmulation();	


private:
	void SetQueryResult(UINT32 v);

	UINT32 queryResult;
	UINT32 queryFinished;
	UINT32 frameIdx;
};


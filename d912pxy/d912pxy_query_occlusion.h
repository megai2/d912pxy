/*
MIT License

Copyright(c) 2018-2020 megai2

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

class d912pxy_query_occlusion : public d912pxy_vtable, public d912pxy_query
{
public:
	static d912pxy_query_occlusion* d912pxy_query_occlusion_com(D3DQUERYTYPE Type);
	~d912pxy_query_occlusion();

	D912PXY_METHOD(occ_Issue)(PXY_THIS_ DWORD dwIssueFlags);
	D912PXY_METHOD(occ_GetData)(PXY_THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags);

	D912PXY_METHOD_NC(occ_Issue)(THIS_ DWORD dwIssueFlags);
	D912PXY_METHOD_NC(occ_GetData)(THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags);

	void QueryMark(UINT start, ID3D12GraphicsCommandList* cl);

	static void FlushQueryStack();
	static void OnIFrameEnd();
	static void OnIFrameStart();

	void ForceClose();

	static UINT InitOccQueryEmulation();
	static void FreePendingQueryObjects();
	static void DeInitOccQueryEmulation();	
	static UINT32 bufferedReadback;

private:
	d912pxy_query_occlusion(D3DQUERYTYPE Type);

	void SetQueryResult(UINT32 v);

	UINT32 queryResult;
	UINT32 queryFinished;
	UINT32 frameIdx;
	UINT32 queryOpened;
};

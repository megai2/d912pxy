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

UINT32 d912pxy_query_occlusion::forcedRet = 0;
ID3D12QueryHeap* g_occQueryHeap = 0;
UINT32 g_occQueryFrameIdx = 0;
d912pxy_resource* g_occReadbackBuffer;
UINT64* g_occReadbackPtr;
d912pxy_query_occlusion* g_occWriteStack[PXY_INNER_MAX_OCCLUSION_QUERY_COUNT_PER_FRAME];

#define PXY_OCCLUSION_TYPE D3D12_QUERY_TYPE_OCCLUSION
#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_QUERY_OCCLUSION

d912pxy_query_occlusion::d912pxy_query_occlusion(d912pxy_device* dev, D3DQUERYTYPE Type) : d912pxy_query(dev, Type)
{
}


d912pxy_query_occlusion::~d912pxy_query_occlusion()
{
}

#define D912PXY_METHOD_IMPL_CN d912pxy_query_occlusion

D912PXY_IUNK_IMPL

/*** IDirect3DQuery9 methods ***/
D912PXY_METHOD_IMPL(GetDevice)(THIS_ IDirect3DDevice9** ppDevice)
{
	return d912pxy_query::GetDevice(ppDevice);
}

D912PXY_METHOD_IMPL_(D3DQUERYTYPE, GetType)(THIS)
{
	return d912pxy_query::GetType();
}

D912PXY_METHOD_IMPL_(DWORD, GetDataSize)(THIS)
{
	return d912pxy_query::GetDataSize();
}

D912PXY_METHOD_IMPL(Issue)(THIS_ DWORD dwIssueFlags)
{
	API_OVERHEAD_TRACK_START(0)

	if (dwIssueFlags & D3DISSUE_BEGIN)
	{
		if (!g_occQueryFrameIdx)
			d912pxy_s(iframe)->StateSafeFlush(0);

		if (g_occQueryFrameIdx >= PXY_INNER_MAX_OCCLUSION_QUERY_COUNT_PER_FRAME)
			FlushQueryStack();			

		queryFinished = 0;
		frameIdx = g_occQueryFrameIdx;
		d912pxy_s(CMDReplay)->QueryMark(this, 1);		
		g_occWriteStack[g_occQueryFrameIdx] = this;
		++g_occQueryFrameIdx;
	}
	else {
		d912pxy_s(CMDReplay)->QueryMark(this, 0);		
	}

	API_OVERHEAD_TRACK_END(0)

	return d912pxy_query::Issue(dwIssueFlags);
}

D912PXY_METHOD_IMPL(GetData)(THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	if (forcedRet)
	{
		if (dwSize == 4)		
			((DWORD*)pData)[0] = forcedRet-1;							
	}
	else {

		if (g_occQueryFrameIdx && !queryFinished)		
			FlushQueryStack();		

		((DWORD*)pData)[0] = queryResult;				
	}

	API_OVERHEAD_TRACK_END(0)

	return S_OK;
}

#undef D912PXY_METHOD_IMPL_CN

void d912pxy_query_occlusion::QueryMark(UINT start, ID3D12GraphicsCommandList * cl)
{
	switch (start)
	{
	case 1:
		cl->BeginQuery(g_occQueryHeap, PXY_OCCLUSION_TYPE, frameIdx);
		break;
	case 0:
		cl->EndQuery(g_occQueryHeap, PXY_OCCLUSION_TYPE, frameIdx);
		//cl->ResolveQueryData(g_occQueryHeap, PXY_OCCLUSION_TYPE, frameIdx, 1, g_occReadbackBuffer->GetD12Obj(), frameIdx*8);
		break;
	case 2:
		cl->ResolveQueryData(g_occQueryHeap, PXY_OCCLUSION_TYPE, 0, g_occQueryFrameIdx, g_occReadbackBuffer->GetD12Obj(), 0);
		break;
	default:
		LOG_ERR_THROW2(-1, "wrong occ query mark");
	}
}

void d912pxy_query_occlusion::FlushQueryStack()
{
	ID3D12GraphicsCommandList* cl = d912pxy_s(GPUcl)->GID(CLG_SEQ);
	
	cl->ResolveQueryData(g_occQueryHeap, PXY_OCCLUSION_TYPE, 0, g_occQueryFrameIdx, g_occReadbackBuffer->GetD12Obj(), 0);
	
	d912pxy_s(iframe)->StateSafeFlush(1);

	LOG_ERR_THROW2(g_occReadbackBuffer->GetD12Obj()->Map(0, 0, (void**)&g_occReadbackPtr), "occ query flush map failed");

	for (int i = 0; i != g_occQueryFrameIdx; ++i)
	{		
		g_occWriteStack[i]->SetQueryResult((UINT32)g_occReadbackPtr[i]);
	}

	g_occReadbackBuffer->GetD12Obj()->Unmap(0, 0);

	g_occQueryFrameIdx = 0;	
}

UINT d912pxy_query_occlusion::InitOccQueryEmulation()
{
	D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
	queryHeapDesc.Count = PXY_INNER_MAX_OCCLUSION_QUERY_COUNT_PER_FRAME;
	queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
	if (FAILED(d912pxy_s(DXDev)->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&g_occQueryHeap))))
		return 1;

	g_occReadbackBuffer = new d912pxy_resource(d912pxy_s(dev), RTID_RB_BUF, L"readback buffer");
	g_occReadbackBuffer->d12res_readback_buffer(8*PXY_INNER_MAX_OCCLUSION_QUERY_COUNT_PER_FRAME);

	return 0;
}

void d912pxy_query_occlusion::DeInitOccQueryEmulation()
{
	if (g_occQueryHeap)
	{
		g_occQueryHeap->Release();
		g_occReadbackBuffer->Release();
	}
}

void d912pxy_query_occlusion::SetQueryResult(UINT32 v)
{
	queryResult = v;
	queryFinished = 1;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE
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

typedef struct d912pxy_query_occlusion_gpu_stack {
	d912pxy_resource* readbackBuffer;
	d912pxy_query_occlusion* stack[PXY_INNER_MAX_OCCLUSION_QUERY_COUNT_PER_FRAME];
	UINT32 count;
} d912pxy_query_occlusion_gpu_stack;

ID3D12QueryHeap* g_occQueryHeap = 0;
d912pxy_query_occlusion_gpu_stack g_gpuStack[2] = { 0 };
UINT32 g_writeStack = 0;

#define PXY_OCCLUSION_TYPE D3D12_QUERY_TYPE_OCCLUSION
#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_QUERY_OCCLUSION

d912pxy_query_occlusion::d912pxy_query_occlusion(D3DQUERYTYPE Type) 
	: d912pxy_query(Type)
	, queryResult(0)
	, queryFinished(0)
	, frameIdx(0)
	, queryOpened(0)
{ }


d912pxy_query_occlusion * d912pxy_query_occlusion::d912pxy_query_occlusion_com(D3DQUERYTYPE Type)
{
	d912pxy_com_object* ret = d912pxy_s.com.AllocateComObj(PXY_COM_OBJ_QUERY_OCC);
	
	new (&ret->query_occ)d912pxy_query_occlusion(Type);
	ret->vtable = d912pxy_com_route_get_vtable(PXY_COM_ROUTE_QUERY_OCC);

	return &ret->query_occ;
}

d912pxy_query_occlusion::~d912pxy_query_occlusion()
{
}

#define D912PXY_METHOD_IMPL_CN d912pxy_query_occlusion

D912PXY_METHOD_IMPL_NC(occ_Issue)(THIS_ DWORD dwIssueFlags)
{	
	if (dwIssueFlags & D3DISSUE_BEGIN)
	{
		if (g_gpuStack[g_writeStack].count >= PXY_INNER_MAX_OCCLUSION_QUERY_COUNT_PER_FRAME)
		{
			LOG_ERR_DTDM("Too many occlusion queries per frame, performance will degrade ( > %u )", PXY_INNER_MAX_OCCLUSION_QUERY_COUNT_PER_FRAME);
			FlushQueryStack();
		}

		//megai2: as we use addition on query result read due to inter-cl force close/open thingy, we need to clear result on fully open condition
		if (!(dwIssueFlags & D3DISSUE_FORCED))
			queryResult = 0;
		else if (!queryOpened)
			//if force open called when query is closed normally, ignore it
			return D3D_OK;

		queryFinished++;
		frameIdx = g_gpuStack[g_writeStack].count;
		d912pxy_s.render.replay.DoQueryMark(this, 1);
		g_gpuStack[g_writeStack].stack[frameIdx] = this;
		ThreadRef(1);
		queryOpened = 1;
		++g_gpuStack[g_writeStack].count;
	}
	else {
		queryOpened = 0;
		d912pxy_s.render.replay.DoQueryMark(this, 0);
	}
	
	return D3D_OK;
}

D912PXY_METHOD_IMPL_NC(occ_GetData)(THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags)
{
	LOG_DBG_DTDM(__FUNCTION__);
		
	if (queryFinished)
	{
		//flush only wheny app asks for flush and we are not executing work on gpu already
		if ((D3DGETDATA_FLUSH & dwGetDataFlags) && (d912pxy_s.dx12.que.IsWorkCompleted() == 0))
			FlushQueryStack();

		return S_FALSE;
	}

	((DWORD*)pData)[0] = queryResult;
	
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
		//cl->ResolveQueryData(g_occQueryHeap, PXY_OCCLUSION_TYPE, 0, g_occQueryFrameIdx, g_occReadbackBuffer->GetD12Obj(), 0);
		break;
	default:
		LOG_ERR_THROW2(-1, "wrong occ query mark");
	}
}

void d912pxy_query_occlusion::FlushQueryStack()
{	
	d912pxy_s.render.iframe.StateSafeFlush(0);
}


void d912pxy_query_occlusion::OnIFrameEnd()
{
	d912pxy_query_occlusion_gpu_stack* writeStack = &g_gpuStack[g_writeStack];

	if (writeStack->count)
	{
		ID3D12GraphicsCommandList* cl = d912pxy_s.dx12.cl->GID(CLG_SEQ);

		for (int i = 0; i != writeStack->count; ++i)
			writeStack->stack[i]->ForceClose();

		cl->ResolveQueryData(g_occQueryHeap, PXY_OCCLUSION_TYPE, 0, writeStack->count, writeStack->readbackBuffer->GetD12Obj(), 0);
	}
}

void d912pxy_query_occlusion::OnIFrameStart()
{
	d912pxy_query_occlusion_gpu_stack* writeStack = &g_gpuStack[g_writeStack];

	//megai2: open query after force close
	if (writeStack->count)
	{
		for (int i = 0; i != writeStack->count; ++i)
			writeStack->stack[i]->Issue(D3DISSUE_BEGIN | D3DISSUE_FORCED);
	}

	d912pxy_query_occlusion_gpu_stack* readStack = &g_gpuStack[!g_writeStack];

	if (readStack->count)
	{
		UINT64* readbackPtr;

		if (!FAILED(readStack->readbackBuffer->GetD12Obj()->Map(0, 0, (void**)&readbackPtr)))
		{
			for (int i = 0; i != readStack->count; ++i)
			{
				readStack->stack[i]->SetQueryResult((UINT32)readbackPtr[i]);
			}

			readStack->readbackBuffer->GetD12Obj()->Unmap(0, 0);
		}

		readStack->count = 0;
	}
	
	g_writeStack = !g_writeStack;
}

void d912pxy_query_occlusion::ForceClose()
{
	if (queryOpened)
	{
		d912pxy_s.render.replay.DoQueryMark(this, 0);
	}
}

UINT d912pxy_query_occlusion::InitOccQueryEmulation()
{
	D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
	queryHeapDesc.Count = PXY_INNER_MAX_OCCLUSION_QUERY_COUNT_PER_FRAME;
	queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
	if (FAILED(d912pxy_s.dx12.dev->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&g_occQueryHeap))))
		return 1;

	for (int i = 0; i != 2; ++i)
	{
		g_gpuStack[i].readbackBuffer = new d912pxy_resource(RTID_RB_BUF, PXY_COM_OBJ_NOVTABLE, L"query readback buffer");
		g_gpuStack[i].readbackBuffer->d12res_readback_buffer(8 * PXY_INNER_MAX_OCCLUSION_QUERY_COUNT_PER_FRAME);
		g_gpuStack[i].count = 0;		
	}

	g_writeStack = 0;

	return 0;
}

void d912pxy_query_occlusion::FreePendingQueryObjects()
{
	for (int i = 0; i != 2; ++i)
	{
		UINT32 objCount = g_gpuStack[i].count;

		if (!objCount)
			continue;

		UINT32 unused;

		for (int j = 0; j != objCount; ++j)
			g_gpuStack[i].stack[j]->occ_GetData(&unused, 4, 0);

	}
}

void d912pxy_query_occlusion::DeInitOccQueryEmulation()
{
	//finish outstanding queres so thery are properly freed on exit
	d912pxy_query_occlusion_gpu_stack* readStack = &g_gpuStack[!g_writeStack];
	if (readStack->count)
	{
		for (int i = 0; i != readStack->count; ++i)	
			readStack->stack[i]->SetQueryResult(0);

		readStack->count = 0;
	}

	if (!g_occQueryHeap)
		return;

	FreePendingQueryObjects();

	g_occQueryHeap->Release();

	for (int i = 0; i != 2; ++i)		
	{
		g_gpuStack[i].readbackBuffer->Release();
	}
}

void d912pxy_query_occlusion::SetQueryResult(UINT32 v)
{	
	queryResult += v;
	queryFinished--;
	ThreadRef(-1);
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE
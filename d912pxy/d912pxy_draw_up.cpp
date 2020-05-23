#include "stdafx.h"

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_DEVICE_DRAWING_UP

d912pxy_draw_up::d912pxy_draw_up()
{
}


d912pxy_draw_up::~d912pxy_draw_up()
{
}

void d912pxy_draw_up::Init()
{
	NonCom_Init(L"draw_up");

	for (int i = 0; i != PXY_DUP_COUNT*2; ++i)
	{
		//megai2: use 64kb buffers at start
		UINT32 tmpUPbufSpace = 0xFFFF;

		AllocateBuffer((d912pxy_draw_up_buffer_name)i, tmpUPbufSpace);
		LockBuffer((d912pxy_draw_up_buffer_name)i);

		if ((i % PXY_DUP_COUNT) == PXY_DUP_DPI)
		{
			for (int j = 0; j != tmpUPbufSpace >> 2; ++j)
			{
				((UINT32*)buf[i].writePoint)[j] = j;
			}

			buf[i].offset = tmpUPbufSpace;
		}
	}
}

void d912pxy_draw_up::UnInit()
{
	OnFrameEnd();

	for (int i = 0; i != PXY_DUP_COUNT * 2; ++i)
	{
		buf[i].vstream->Release();
	}

	d912pxy_noncom::UnInit();
}

void d912pxy_draw_up::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, const void * pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	LOG_DBG_DTDM2("DPUP %u %u %016llX %u", PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);

	UINT indexCount = d912pxy_s.render.iframe.GetIndexCount(PrimitiveCount, PrimitiveType);

	if (!d912pxy_s.render.iframe.CommitBatchPreCheck(PrimitiveType))
		return;

	UINT32 len = buf[PXY_DUP_DPI].vstream->GetLength();

	//megai2: make a bigger index buffer if current is to small
	if (indexCount >= (len >> 2))
	{		
		len *= 2;

		UINT32 indxCntLen = indexCount * 2 * 4;

		len = indxCntLen > len ? indxCntLen : len;

		len = len >= 0x7FFFFFF ? 0x7FFFFFF : len;

		if (buf[PXY_DUP_DPI].writePoint)
		{
			buf[PXY_DUP_DPI].vstream->UnlockRanged(0, buf[PXY_DUP_DPI].offset);
			buf[PXY_DUP_DPI].vstream->Release();
		}

		AllocateBuffer(PXY_DUP_DPI, len);
		//megai2: update for possible pooling limitations
		len = buf[PXY_DUP_DPI].vstream->GetLength();

		LockBuffer(PXY_DUP_DPI);
	
		for (int j = 0; j != len >> 2; ++j)
		{
			((UINT32*)buf[PXY_DUP_DPI].writePoint)[j] = j;
		}

		buf[PXY_DUP_DPI].offset = len;

		if (indexCount >= (len >> 2))
		{
			LOG_ERR_DTDM("DPUP Index Count = %lX is too much, skipping!", indexCount);
			return;
		}
	}

	UINT vstreamRegLen = VertexStreamZeroStride * indexCount;
	UINT vsvOffset = BufferWrite(PXY_DUP_DPV, vstreamRegLen, pVertexStreamZeroData);

	d912pxy_s.render.iframe.SetIBuf(buf[PXY_DUP_DPI].vstream);
	d912pxy_s.render.iframe.SetVBuf(buf[PXY_DUP_DPV].vstream, 0, vsvOffset, VertexStreamZeroStride);

	((IDirect3DDevice9*)&d912pxy_s.dev)->DrawIndexedPrimitive(PrimitiveType, 0, 0, 0, 0, PrimitiveCount);
	//d912pxy_s.render.iframe.CommitBatch2(PrimitiveType, 0, 0, 0, 0, PrimitiveCount);		
}

void d912pxy_draw_up::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, const void * pIndexData, D3DFORMAT IndexDataFormat, const void * pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	UINT hiInd = IndexDataFormat == D3DFMT_INDEX32;
	d912pxy_draw_up_buffer_name indBuf = hiInd ? PXY_DUP_DIPI4 : PXY_DUP_DIPI2;

	UINT indBufSz = (hiInd * 2 + 2)*d912pxy_s.render.iframe.GetIndexCount(PrimitiveCount, PrimitiveType);
	UINT vertBufSz = VertexStreamZeroStride * (MinVertexIndex + NumVertices);		

	if (!d912pxy_s.render.iframe.CommitBatchPreCheck(PrimitiveType))
		return;

	UINT vsiOffset = BufferWrite(indBuf, indBufSz, pIndexData);
	UINT vsvOffset = BufferWrite(PXY_DUP_DIPV, vertBufSz, pVertexStreamZeroData);
	
	d912pxy_s.render.iframe.SetIBuf(buf[indBuf].vstream);
	d912pxy_s.render.iframe.SetVBuf(buf[PXY_DUP_DIPV].vstream, 0, vsvOffset, VertexStreamZeroStride);	
	
	((IDirect3DDevice9*)&d912pxy_s.dev)->DrawIndexedPrimitive(PrimitiveType, 0, MinVertexIndex, NumVertices, vsiOffset >> (1 + hiInd), PrimitiveCount);
	//d912pxy_s.render.iframe.CommitBatch2(PrimitiveType, 0, MinVertexIndex, NumVertices, vsiOffset >> (1 + hiInd), PrimitiveCount);			
}

void d912pxy_draw_up::OnFrameEnd()
{
	for (int i = 0; i != PXY_DUP_COUNT; ++i)
	{
		if (buf[i].writePoint)
		{
			buf[i].vstream->UnlockRanged(0, buf[i].offset);
			buf[i].writePoint = 0;

			d912pxy_draw_up_buffer swp = buf[i+PXY_DUP_COUNT];
			buf[i + PXY_DUP_COUNT] = buf[i];
			buf[i] = swp;			
		}
	}

	
}

UINT d912pxy_draw_up::BufferWrite(d912pxy_draw_up_buffer_name bid, UINT size, const void * src)
{
	if (!buf[bid].writePoint)
		LockBuffer(bid);

	intptr_t tmpWP = buf[bid].writePoint;
	if ((tmpWP + size) >= buf[bid].endPoint)
	{
		UINT32 len = buf[bid].vstream->GetLength();
		//megai2: growing buffer size too big will end up in long buffer upload wait on frame end
		//len *= 2;
		

		len = size >= len ? size * 2 : len;

		len = len >= 0x7FFFFFF ? 0x7FFFFFF : len;

		buf[bid].vstream->UnlockRanged(0, buf[bid].offset);
		buf[bid].vstream->Release();

		AllocateBuffer(bid, len);
		LockBuffer(bid);

		tmpWP = buf[bid].writePoint;

		if ((tmpWP + size) >= buf[bid].endPoint)
		{
			LOG_ERR_DTDM("App asked %lX ibytes to draw, TOO BIG, skipping", size);
			return -1;
		}
	} 

	memcpy((void*)tmpWP, src, size);

	buf[bid].writePoint += size;

	UINT ret = buf[bid].offset;
	buf[bid].offset += size;

	return ret;
}

void d912pxy_draw_up::LockBuffer(d912pxy_draw_up_buffer_name bid)
{
	buf[bid].vstream->Lock(0, 0, (void**)&buf[bid].writePoint, 0);
	buf[bid].endPoint = buf[bid].writePoint + buf[bid].vstream->GetLength();
	buf[bid].offset = 0;
}

void d912pxy_draw_up::AllocateBuffer(d912pxy_draw_up_buffer_name bid, UINT len)
{
	UINT nFmt;
	UINT nIB;

	switch (bid % PXY_DUP_COUNT)
	{
	case PXY_DUP_DPV:
	case PXY_DUP_DIPV:
		nFmt = 0;
		nIB = 0;
		break;
	case PXY_DUP_DPI:
	case PXY_DUP_DIPI4:
		nFmt = D3DFMT_INDEX32;
		nIB = 1;
		break;
	case PXY_DUP_DIPI2:
		nFmt = D3DFMT_INDEX16;
		nIB = 1;
		break;
	default:
		LOG_ERR_THROW2(-1, "AllocateBuffer bid is wrong");
	}

	buf[bid].vstream = d912pxy_s.pool.vstream.GetVStreamObject(len, nFmt, nIB);
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
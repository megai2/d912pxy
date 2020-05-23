#pragma once
#include "stdafx.h"

typedef struct d912pxy_draw_up_buffer {
	d912pxy_vstream* vstream;
	UINT offset;
	intptr_t writePoint;
	intptr_t endPoint;
} d912pxy_draw_up_buffer;

typedef enum d912pxy_draw_up_buffer_name {
	PXY_DUP_DPI = 0,
	PXY_DUP_DPV = 1,
	PXY_DUP_DIPV,
	PXY_DUP_DIPI2,
	PXY_DUP_DIPI4,
	PXY_DUP_COUNT
} d912pxy_draw_up_buffer_name;

class d912pxy_draw_up :
	public d912pxy_noncom
{
public:
	d912pxy_draw_up();
	~d912pxy_draw_up();
	
	void Init();
	void UnInit();

	void DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	void DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);

	void OnFrameEnd();

private:
	d912pxy_draw_up_buffer buf[PXY_DUP_COUNT*2];

	UINT BufferWrite(d912pxy_draw_up_buffer_name bid, UINT size, const void* src);
	void LockBuffer(d912pxy_draw_up_buffer_name bid);

	void AllocateBuffer(d912pxy_draw_up_buffer_name bid, UINT len);
};


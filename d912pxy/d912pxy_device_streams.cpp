#include "stdafx.h"

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_DEVICE_STREAMS

//buffer binders

HRESULT WINAPI d912pxy_device::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	LOG_DBG_DTDM("bind @%u with %u : %u", StreamNumber, OffsetInBytes, Stride);

	if (StreamNumber >= PXY_INNER_MAX_VBUF_STREAMS)
		return D3DERR_INVALIDCALL;

	d912pxy_s(iframe)->SetVBuf((d912pxy_vbuf*)pStreamData, StreamNumber, OffsetInBytes, Stride);

	API_OVERHEAD_TRACK_END(0)
	
	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::SetStreamSourceFreq(UINT StreamNumber, UINT Divider)
{ 
	API_OVERHEAD_TRACK_START(0)

	LOG_DBG_DTDM("stream %u div %u", StreamNumber, Divider);

	if (StreamNumber >= PXY_INNER_MAX_VBUF_STREAMS)
		return D3DERR_INVALIDCALL;

	d912pxy_s(iframe)->SetStreamFreq(StreamNumber, Divider);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	if (pIndexData)
		d912pxy_s(iframe)->SetIBuf((d912pxy_ibuf*)pIndexData);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

//vdecl 

HRESULT WINAPI d912pxy_device::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

		if (pDecl)
		{
			d912pxy_s(psoCache)->IAFormat((d912pxy_vdecl*)pDecl);
		}

	API_OVERHEAD_TRACK_END(0)

		return D3D_OK;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
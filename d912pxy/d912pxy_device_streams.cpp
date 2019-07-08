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

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_DEVICE_STREAMS

//buffer binders

HRESULT d912pxy_device::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	LOG_DBG_DTDM("bind @%u with %u : %u", StreamNumber, OffsetInBytes, Stride);

	if (StreamNumber >= PXY_INNER_MAX_VBUF_STREAMS)
		return D3DERR_INVALIDCALL;
	
	if (pStreamData)
		d912pxy_s.render.iframe.SetVBuf(PXY_COM_LOOKUP(pStreamData, vstream), StreamNumber, OffsetInBytes, Stride);
	else
		d912pxy_s.render.iframe.SetVBuf(0, StreamNumber, OffsetInBytes, Stride);

	API_OVERHEAD_TRACK_END(0)
	
	return D3D_OK; 
}

HRESULT d912pxy_device::SetStreamSource_CAR(UINT StreamNumber, IDirect3DVertexBuffer9 * pStreamData, UINT OffsetInBytes, UINT Stride)
{
	API_OVERHEAD_TRACK_START(0)

	if (StreamNumber >= PXY_INNER_MAX_VBUF_STREAMS)
		return D3DERR_INVALIDCALL;

	if (pStreamData)
		d912pxy_s.render.iframe.SetVBufIfChanged(PXY_COM_LOOKUP(pStreamData, vstream), StreamNumber, OffsetInBytes, Stride);
	else
		d912pxy_s.render.iframe.SetVBufIfChanged(0, StreamNumber, OffsetInBytes, Stride);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT d912pxy_device::SetIndices_CAR(IDirect3DIndexBuffer9 * pIndexData)
{
	API_OVERHEAD_TRACK_START(0)

	if (pIndexData)
		d912pxy_s.render.iframe.SetIBufIfChanged(PXY_COM_LOOKUP(pIndexData, vstream));
	else
		d912pxy_s.render.iframe.SetIBufIfChanged(0);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT d912pxy_device::SetStreamSourceFreq(UINT StreamNumber, UINT Divider)
{ 
	API_OVERHEAD_TRACK_START(0)

	LOG_DBG_DTDM("stream %u div %u", StreamNumber, Divider);

	if (StreamNumber >= PXY_INNER_MAX_VBUF_STREAMS)
		return D3DERR_INVALIDCALL;

	d912pxy_s.render.iframe.SetStreamFreq(StreamNumber, Divider);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT d912pxy_device::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	if (pIndexData)
		d912pxy_s.render.iframe.SetIBuf(PXY_COM_LOOKUP(pIndexData, vstream));
	else 
		d912pxy_s.render.iframe.SetIBuf(0);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

//vdecl 

HRESULT d912pxy_device::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	if (pDecl)
	{
		d912pxy_s.render.db.pso.IAFormat(PXY_COM_LOOKUP(pDecl, vdecl));
	}

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
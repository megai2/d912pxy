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
#include "stdafx.h"

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_DEVICE_STREAMS

//buffer binders

HRESULT d912pxy_device::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{ 
	if (StreamNumber >= PXY_INNER_MAX_VBUF_STREAMS)
		return D3DERR_INVALIDCALL;

	d912pxy_s.render.iframe.SetVBuf(PXY_COM_LOOKUP(pStreamData, vstream), StreamNumber, OffsetInBytes, Stride);
	
	return D3D_OK; 
}

HRESULT d912pxy_device::SetStreamSource_CAR(UINT StreamNumber, IDirect3DVertexBuffer9 * pStreamData, UINT OffsetInBytes, UINT Stride)
{
	if (StreamNumber >= PXY_INNER_MAX_VBUF_STREAMS)
		return D3DERR_INVALIDCALL;

	d912pxy_s.render.iframe.SetVBufIfChanged(PXY_COM_LOOKUP(pStreamData, vstream), StreamNumber, OffsetInBytes, Stride);

	return D3D_OK;
}

HRESULT d912pxy_device::SetIndices_CAR(IDirect3DIndexBuffer9 * pIndexData)
{
	d912pxy_s.render.iframe.SetIBufIfChanged(PXY_COM_LOOKUP(pIndexData, vstream));
	
	return D3D_OK;
}

HRESULT d912pxy_device::SetStreamSourceFreq(UINT StreamNumber, UINT Divider)
{ 
	if (StreamNumber >= PXY_INNER_MAX_VBUF_STREAMS)
		return D3DERR_INVALIDCALL;

	d912pxy_s.render.iframe.SetStreamFreq(StreamNumber, Divider);
	
	return D3D_OK; 
}

HRESULT d912pxy_device::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{ 
	d912pxy_s.render.iframe.SetIBuf(PXY_COM_LOOKUP(pIndexData, vstream));

	return D3D_OK; 
}

//vdecl 

HRESULT d912pxy_device::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	d912pxy_s.render.state.pso.IAFormat(PXY_COM_LOOKUP(pDecl, vdecl));

	return D3D_OK;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
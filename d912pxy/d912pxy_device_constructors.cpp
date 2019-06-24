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

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_DEVICE_CONSTRUCTORS

HRESULT WINAPI d912pxy_device::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	void* extendedPlace = NULL;
	PXY_MALLOC(extendedPlace, sizeof(d912pxy_texture) + 8, void*);
	extendedPlace = (void*)((intptr_t)extendedPlace + 8);
		
	*ppTexture = new (extendedPlace) d912pxy_texture(this, Width, Height, Levels, Usage, Format);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppVolumeTexture = new d912pxy_vtexture(this, Width, Height, Depth, Levels, Usage, Format);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	void* extendedPlace = NULL;
	PXY_MALLOC(extendedPlace, sizeof(d912pxy_ctexture) + 8, void*);
	extendedPlace = (void*)((intptr_t)extendedPlace + 8);

	*ppCubeTexture = new (extendedPlace) d912pxy_ctexture(this, EdgeLength, Levels, Usage, Format);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppVertexBuffer = d912pxy_s(pool_vstream)->GetVStreamObject(Length, FVF, 0)->AsDX9VB();

	//*ppVertexBuffer = new d912pxy_vbuf(this, Length, Usage, FVF);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppIndexBuffer = d912pxy_s(pool_vstream)->GetVStreamObject(Length, Format, 1)->AsDX9IB();

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	LOG_DBG_DTDM(__FUNCTION__);

	LOG_DBG_DTDM("new RT FMT: %u", Format);

	API_OVERHEAD_TRACK_START(0)
	
	*ppSurface = d912pxy_s(pool_surface)->GetSurface(Width, Height, Format, 1, 1, D3DUSAGE_RENDERTARGET, NULL);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppSurface = d912pxy_s(pool_surface)->GetSurface(Width, Height, Format, 1, 1, D3DUSAGE_DEPTHSTENCIL, NULL);

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	d912pxy_sblock* ret = new d912pxy_sblock(this, Type);
	*ppSB = ret;

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT WINAPI d912pxy_device::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppDecl = (IDirect3DVertexDeclaration9*)(new d912pxy_vdecl(this, pVertexElements));

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppShader = (IDirect3DVertexShader9*)(new d912pxy_vshader(this, pFunction));

	API_OVERHEAD_TRACK_END(0)
	
	return D3D_OK; 
}

HRESULT WINAPI d912pxy_device::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	*ppShader = (IDirect3DPixelShader9*)(new d912pxy_pshader(this, pFunction));

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

//query!

HRESULT WINAPI d912pxy_device::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	switch (Type)
	{
		case D3DQUERYTYPE_OCCLUSION:
			*ppQuery = (IDirect3DQuery9*)new d912pxy_query_occlusion(this, Type);
			break;
		default:
			*ppQuery = (IDirect3DQuery9*)new d912pxy_query(this, Type);
	}

	API_OVERHEAD_TRACK_END(0)

	return 0; 
}

HRESULT WINAPI d912pxy_device::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	LOG_DBG_DTDM3(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

	//megai2: hacky way to fix dxgi backbuffer format change
	switch (Format)
	{
		case D3DFMT_X8R8G8B8:
		case D3DFMT_UNKNOWN:
			Format = D3DFMT_A8R8G8B8;
		break;
	}

	UINT levels = 1;
	d912pxy_surface* ret = new d912pxy_surface(this, Width, Height, Format, D3DUSAGE_D912PXY_FORCE_RT, D3DMULTISAMPLE_NONE, 0, 0, &levels, 1, NULL);

	*ppSurface = (IDirect3DSurface9*)ret;

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
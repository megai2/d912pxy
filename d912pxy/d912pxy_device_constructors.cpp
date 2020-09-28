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

HRESULT d912pxy_device::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{		
	*ppTexture = PXY_COM_CAST_(IDirect3DTexture9, d912pxy_texture::d912pxy_texture_com(Width, Height, Levels, Usage, Format));

	return D3D_OK;
}

HRESULT d912pxy_device::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
{
	*ppVolumeTexture = PXY_COM_CAST_(IDirect3DVolumeTexture9, d912pxy_vtexture::d912pxy_vtexture_com(Width, Height, Depth, Levels, Usage, Format));

	return D3D_OK;
}

HRESULT d912pxy_device::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{
	*ppCubeTexture = PXY_COM_CAST_(IDirect3DCubeTexture9, d912pxy_ctexture::d912pxy_ctexture_com(EdgeLength, Levels, Usage, Format));

	return D3D_OK;
}

HRESULT d912pxy_device::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{
	*ppVertexBuffer = PXY_COM_CAST_(IDirect3DVertexBuffer9, d912pxy_s.pool.vstream.GetVStreamObject(Length, FVF, 0));

	return D3D_OK;
}

HRESULT d912pxy_device::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{
	*ppIndexBuffer = PXY_COM_CAST_(IDirect3DIndexBuffer9, d912pxy_s.pool.vstream.GetVStreamObject(Length, Format, 1));

	return D3D_OK;
}

HRESULT d912pxy_device::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	*ppSurface = PXY_COM_CAST_(IDirect3DSurface9, d912pxy_s.pool.surface.GetSurface(Width, Height, Format, 1, 1, D3DUSAGE_RENDERTARGET, NULL));

	return D3D_OK;
}

HRESULT d912pxy_device::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	*ppSurface = PXY_COM_CAST_(IDirect3DSurface9, d912pxy_s.pool.surface.GetSurface(Width, Height, Format, 1, 1, D3DUSAGE_DEPTHSTENCIL, NULL));

	return D3D_OK;
}

HRESULT d912pxy_device::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
{
	*ppSB = PXY_COM_CAST_(IDirect3DStateBlock9, d912pxy_sblock::d912pxy_sblock_com(Type));

	return D3D_OK;
}

HRESULT d912pxy_device::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
{ 
	*ppDecl = PXY_COM_CAST_(IDirect3DVertexDeclaration9, d912pxy_vdecl::d912pxy_vdecl_com(pVertexElements));

	return D3D_OK; 
}

HRESULT d912pxy_device::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{ 
	*ppShader = PXY_COM_CAST_(IDirect3DVertexShader9, d912pxy_shader::d912pxy_shader_com(PXY_SHADER_TYPE_VS, pFunction, 0));
	
	return D3D_OK; 
}

HRESULT d912pxy_device::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
	*ppShader = PXY_COM_CAST_(IDirect3DPixelShader9, d912pxy_shader::d912pxy_shader_com(PXY_SHADER_TYPE_PS, pFunction, 0));

	return D3D_OK;
}

//query!

HRESULT d912pxy_device::CreateQuery_Optimized(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{
	if (!ppQuery)
		return D3D_OK;

	switch (Type)
	{
	case D3DQUERYTYPE_OCCLUSION:
		mFakeOccQuery->AddRef();
		*ppQuery = mFakeOccQuery;
		break;
	default:
		*ppQuery = PXY_COM_CAST_(IDirect3DQuery9, d912pxy_query::d912pxy_query_com(Type));
	}

	return 0;
}

HRESULT d912pxy_device::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{ 
	if (!ppQuery)
		return D3D_OK;

	switch (Type)
	{
		case D3DQUERYTYPE_OCCLUSION:
			*ppQuery = PXY_COM_CAST_(IDirect3DQuery9, d912pxy_query_occlusion::d912pxy_query_occlusion_com(Type));
			break;
		default:
			*ppQuery = PXY_COM_CAST_(IDirect3DQuery9, d912pxy_query::d912pxy_query_com(Type));
	}

	return 0; 
}

HRESULT d912pxy_device::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	//megai2: hacky way to fix dxgi backbuffer format change
	switch (Format)
	{
		case D3DFMT_X8R8G8B8:
		case D3DFMT_UNKNOWN:
			Format = D3DFMT_A8R8G8B8;
		break;
	}

	UINT levels = 1;
	
	*ppSurface = PXY_COM_CAST_(IDirect3DSurface9, d912pxy_surface::d912pxy_surface_com(Width, Height, Format, D3DUSAGE_D912PXY_FORCE_RT, D3DMULTISAMPLE_NONE, 0, 0, &levels, 1, NULL));

	

	return D3D_OK;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 
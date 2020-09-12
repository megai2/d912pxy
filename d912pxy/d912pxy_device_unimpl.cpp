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

//UNIMPLEMENTED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#define D912PXY_ROUTE_IMPL_STUB(ret) ImplStubCall(__FUNCTION__, __LINE__); return ret
#define D912PXY_ROUTE_IMPL_STUB_(ret) ImplStubCall(__FUNCTION__, __LINE__); 

HRESULT d912pxy_device::SetDialogBoxMode(BOOL bEnableDialogs)
{
	//ignore
	D912PXY_ROUTE_IMPL_STUB(D3D_OK);	
}

HRESULT d912pxy_device::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap)
{
	//megai2: not for full d3d9 porting here
	D912PXY_ROUTE_IMPL_STUB(D3D_OK);	
}

BOOL d912pxy_device::ShowCursor(BOOL bShow)
{
	D912PXY_ROUTE_IMPL_STUB(true);
}

//megai2: texture stage states are fixed pipeline and won't work if we use shaders, is that correct?

HRESULT d912pxy_device::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue)
{
	D912PXY_ROUTE_IMPL_STUB(D3D_OK);
}

HRESULT d912pxy_device::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	D912PXY_ROUTE_IMPL_STUB(D3D_OK);
}

HRESULT d912pxy_device::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
{
	D912PXY_ROUTE_IMPL_STUB(D3D_OK);
}

HRESULT d912pxy_device::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }

HRESULT d912pxy_device::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }

HRESULT d912pxy_device::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }

HRESULT d912pxy_device::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }

HRESULT d912pxy_device::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* OffsetInBytes, UINT* pStride) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::GetStreamSourceFreq(UINT StreamNumber, UINT* Divider) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::GetIndices(IDirect3DIndexBuffer9** ppIndexData) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }

HRESULT d912pxy_device::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }

//clipping
//^ done in shaders

HRESULT d912pxy_device::GetClipPlane(DWORD Index, float* pPlane) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::GetClipStatus(D3DCLIPSTATUS9* pClipStatus) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }

//fixed pipe states

HRESULT d912pxy_device::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }

HRESULT d912pxy_device::SetMaterial(CONST D3DMATERIAL9* pMaterial) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::GetMaterial(D3DMATERIAL9* pMaterial) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::SetLight(DWORD Index, CONST D3DLIGHT9* pLight) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::GetLight(DWORD Index, D3DLIGHT9* pLight) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::LightEnable(DWORD Index, BOOL Enable) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::GetLightEnable(DWORD Index, BOOL* pEnable) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }

//palette

HRESULT d912pxy_device::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::SetCurrentTexturePalette(UINT PaletteNumber) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::GetCurrentTexturePalette(UINT *PaletteNumber) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }

//npatch

HRESULT d912pxy_device::SetNPatchMode(float nSegments) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
float d912pxy_device::GetNPatchMode(void) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }

HRESULT d912pxy_device::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }
HRESULT d912pxy_device::DeletePatch(UINT Handle) { D912PXY_ROUTE_IMPL_STUB(D3D_OK); }

HRESULT d912pxy_device::SetFVF(DWORD FVF)
{ 
	D912PXY_ROUTE_IMPL_STUB(D3D_OK);
}

HRESULT d912pxy_device::GetFVF(DWORD* pFVF) 
{ 
	D912PXY_ROUTE_IMPL_STUB(D3D_OK);
}

HRESULT d912pxy_device::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture)
{
	D912PXY_ROUTE_IMPL_STUB(D3D_OK);
}

HRESULT d912pxy_device::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
	D912PXY_ROUTE_IMPL_STUB(D3D_OK);
}

HRESULT d912pxy_device::SetSoftwareVertexProcessing(BOOL bSoftware)
{
	D912PXY_ROUTE_IMPL_STUB(D3D_OK);
}

BOOL d912pxy_device::GetSoftwareVertexProcessing(void)
{
	D912PXY_ROUTE_IMPL_STUB(FALSE);
}

HRESULT d912pxy_device::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
{
	D912PXY_ROUTE_IMPL_STUB(D3D_OK);
}

HRESULT d912pxy_device::GetViewport(D3DVIEWPORT9* pViewport)
{
	D912PXY_ROUTE_IMPL_STUB(D3D_OK);
}

HRESULT d912pxy_device::GetScissorRect(RECT* pRect) {
	D912PXY_ROUTE_IMPL_STUB(D3D_OK);
}

HRESULT d912pxy_device::EvictManagedResources(void)
{
	//megai2: ignore this for now
	D912PXY_ROUTE_IMPL_STUB(D3D_OK);
}
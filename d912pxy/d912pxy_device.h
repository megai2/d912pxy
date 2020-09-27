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
#pragma once

#include "stdafx.h"
#include <chrono>

static const D3D12_DESCRIPTOR_HEAP_DESC d912pxy_dx12_heap_config[PXY_INNER_MAX_DSC_HEAPS] = {
	{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV,			1024,	D3D12_DESCRIPTOR_HEAP_FLAG_NONE,			0 },
	{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV,			1024,	D3D12_DESCRIPTOR_HEAP_FLAG_NONE,			0 },
	{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,	82944,	D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,	0 },
	{ D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,		64,		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,	0 }
};

static const D3D12_DESCRIPTOR_HEAP_DESC d912pxy_dx12_heap_config_compat[PXY_INNER_MAX_DSC_HEAPS] = {
	{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV,			512,	D3D12_DESCRIPTOR_HEAP_FLAG_NONE,			0 },
	{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV,			64,		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,			0 },
	{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,	82944,	D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,  0 },
	{ D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,		64,		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,  0 }
};


class d912pxy_device: public d912pxy_vtable,  public d912pxy_comhandler
{
public:
	static d912pxy_com_object* d912pxy_device_com(void* baseMem, IDirect3DDevice9* dev, void* par);

	d912pxy_device();
	~d912pxy_device(void);

	void Init(IDirect3DDevice9* dev, void* par);
	void UnInit();

//com methods
	D912PXY_METHOD_(ULONG, ReleaseDevice)(PXY_THIS);
	D912PXY_METHOD(TestCooperativeLevel)(PXY_THIS);
	D912PXY_METHOD_(UINT, GetAvailableTextureMem)(PXY_THIS);
	D912PXY_METHOD(EvictManagedResources)(PXY_THIS);
	D912PXY_METHOD(GetDirect3D)(PXY_THIS_ IDirect3D9** ppD3D9);
	D912PXY_METHOD(GetDeviceCaps)(PXY_THIS_ D3DCAPS9* pCaps);
	D912PXY_METHOD(GetDisplayMode)(PXY_THIS_ UINT iSwapChain, D3DDISPLAYMODE* pMode);
	D912PXY_METHOD(GetCreationParameters)(PXY_THIS_ D3DDEVICE_CREATION_PARAMETERS *pParameters);
	D912PXY_METHOD(SetCursorProperties)(PXY_THIS_ UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap);
	D912PXY_METHOD_(void, SetCursorPosition)(PXY_THIS_ int X, int Y, DWORD Flags);
	D912PXY_METHOD_(BOOL, ShowCursor)(PXY_THIS_ BOOL bShow);
	D912PXY_METHOD(CreateAdditionalSwapChain)(PXY_THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain);
	D912PXY_METHOD(GetSwapChain)(PXY_THIS_ UINT iSwapChain, IDirect3DSwapChain9** pSwapChain);
	D912PXY_METHOD_(UINT, GetNumberOfSwapChains)(PXY_THIS);
	D912PXY_METHOD(Reset)(PXY_THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);
	D912PXY_METHOD(Present)(PXY_THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
	D912PXY_METHOD(GetBackBuffer)(PXY_THIS_ UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer);
	D912PXY_METHOD(GetRasterStatus)(PXY_THIS_ UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus);
	D912PXY_METHOD(SetDialogBoxMode)(PXY_THIS_ BOOL bEnableDialogs);
	D912PXY_METHOD_(void, SetGammaRamp)(PXY_THIS_ UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp);
	D912PXY_METHOD_(void, GetGammaRamp)(PXY_THIS_ UINT iSwapChain, D3DGAMMARAMP* pRamp);
	D912PXY_METHOD(CreateTexture)(PXY_THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle);
	D912PXY_METHOD(CreateVolumeTexture)(PXY_THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle);
	D912PXY_METHOD(CreateCubeTexture)(PXY_THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle);
	D912PXY_METHOD(CreateVertexBuffer)(PXY_THIS_ UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle);
	D912PXY_METHOD(CreateIndexBuffer)(PXY_THIS_ UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle);
	D912PXY_METHOD(CreateRenderTarget)(PXY_THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
	D912PXY_METHOD(CreateDepthStencilSurface)(PXY_THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
	D912PXY_METHOD(UpdateSurface)(PXY_THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint);
	D912PXY_METHOD(UpdateTexture)(PXY_THIS_ IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture);
	D912PXY_METHOD(GetRenderTargetData)(PXY_THIS_ IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface);
	D912PXY_METHOD(GetFrontBufferData)(PXY_THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface);
	D912PXY_METHOD(StretchRect)(PXY_THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter);
	D912PXY_METHOD(ColorFill)(PXY_THIS_ IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color);
	D912PXY_METHOD(CreateOffscreenPlainSurface)(PXY_THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
	D912PXY_METHOD(SetRenderTarget)(PXY_THIS_ DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);
	D912PXY_METHOD(GetRenderTarget)(PXY_THIS_ DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget);
	D912PXY_METHOD(SetDepthStencilSurface)(PXY_THIS_ IDirect3DSurface9* pNewZStencil);
	D912PXY_METHOD(GetDepthStencilSurface)(PXY_THIS_ IDirect3DSurface9** ppZStencilSurface);
	D912PXY_METHOD(BeginScene)(PXY_THIS);
	D912PXY_METHOD(EndScene)(PXY_THIS);
	D912PXY_METHOD(Clear)(PXY_THIS_ DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
	D912PXY_METHOD(SetTransform)(PXY_THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix);
	D912PXY_METHOD(GetTransform)(PXY_THIS_ D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix);
	D912PXY_METHOD(MultiplyTransform)(PXY_THIS_ D3DTRANSFORMSTATETYPE, CONST D3DMATRIX*);
	D912PXY_METHOD(SetViewport)(PXY_THIS_ CONST D3DVIEWPORT9* pViewport);
	D912PXY_METHOD(GetViewport)(PXY_THIS_ D3DVIEWPORT9* pViewport);
	D912PXY_METHOD(SetMaterial)(PXY_THIS_ CONST D3DMATERIAL9* pMaterial);
	D912PXY_METHOD(GetMaterial)(PXY_THIS_ D3DMATERIAL9* pMaterial);
	D912PXY_METHOD(SetLight)(PXY_THIS_ DWORD Index, CONST D3DLIGHT9*);
	D912PXY_METHOD(GetLight)(PXY_THIS_ DWORD Index, D3DLIGHT9*);
	D912PXY_METHOD(LightEnable)(PXY_THIS_ DWORD Index, BOOL Enable);
	D912PXY_METHOD(GetLightEnable)(PXY_THIS_ DWORD Index, BOOL* pEnable);
	D912PXY_METHOD(SetClipPlane)(PXY_THIS_ DWORD Index, CONST float* pPlane);
	D912PXY_METHOD(GetClipPlane)(PXY_THIS_ DWORD Index, float* pPlane);
	D912PXY_METHOD(SetRenderState)(PXY_THIS_ D3DRENDERSTATETYPE State, DWORD Value);
	D912PXY_METHOD(GetRenderState)(PXY_THIS_ D3DRENDERSTATETYPE State, DWORD* pValue);
	D912PXY_METHOD(CreateStateBlock)(PXY_THIS_ D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB);
	D912PXY_METHOD(BeginStateBlock)(PXY_THIS);
	D912PXY_METHOD(EndStateBlock)(PXY_THIS_ IDirect3DStateBlock9** ppSB);
	D912PXY_METHOD(SetClipStatus)(PXY_THIS_ CONST D3DCLIPSTATUS9* pClipStatus);
	D912PXY_METHOD(GetClipStatus)(PXY_THIS_ D3DCLIPSTATUS9* pClipStatus);
	D912PXY_METHOD(GetTexture)(PXY_THIS_ DWORD Stage, IDirect3DBaseTexture9** ppTexture);
	D912PXY_METHOD(SetTexture)(PXY_THIS_ DWORD Stage, IDirect3DBaseTexture9* pTexture);
	D912PXY_METHOD(GetTextureStageState)(PXY_THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue);
	D912PXY_METHOD(SetTextureStageState)(PXY_THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
	D912PXY_METHOD(GetSamplerState)(PXY_THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue);
	D912PXY_METHOD(SetSamplerState)(PXY_THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
	D912PXY_METHOD(ValidateDevice)(PXY_THIS_ DWORD* pNumPasses);
	D912PXY_METHOD(SetPaletteEntries)(PXY_THIS_ UINT PaletteNumber, CONST PALETTEENTRY* pEntries);
	D912PXY_METHOD(GetPaletteEntries)(PXY_THIS_ UINT PaletteNumber, PALETTEENTRY* pEntries);
	D912PXY_METHOD(SetCurrentTexturePalette)(PXY_THIS_ UINT PaletteNumber);
	D912PXY_METHOD(GetCurrentTexturePalette)(PXY_THIS_ UINT *PaletteNumber);
	D912PXY_METHOD(SetScissorRect)(PXY_THIS_ CONST RECT* pRect);
	D912PXY_METHOD(GetScissorRect)(PXY_THIS_ RECT* pRect);
	D912PXY_METHOD(SetSoftwareVertexProcessing)(PXY_THIS_ BOOL bSoftware);
	D912PXY_METHOD_(BOOL, GetSoftwareVertexProcessing)(PXY_THIS);
	D912PXY_METHOD(SetNPatchMode)(PXY_THIS_ float nSegments);
	D912PXY_METHOD_(float, GetNPatchMode)(PXY_THIS);
	D912PXY_METHOD(DrawPrimitive)(PXY_THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
	D912PXY_METHOD(DrawIndexedPrimitive)(PXY_THIS_ D3DPRIMITIVETYPE, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
	D912PXY_METHOD(DrawPrimitiveUP)(PXY_THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	D912PXY_METHOD(DrawIndexedPrimitiveUP)(PXY_THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	D912PXY_METHOD(ProcessVertices)(PXY_THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags);
	D912PXY_METHOD(CreateVertexDeclaration)(PXY_THIS_ CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl);
	D912PXY_METHOD(SetVertexDeclaration)(PXY_THIS_ IDirect3DVertexDeclaration9* pDecl);
	D912PXY_METHOD(GetVertexDeclaration)(PXY_THIS_ IDirect3DVertexDeclaration9** ppDecl);
	D912PXY_METHOD(SetFVF)(PXY_THIS_ DWORD FVF);
	D912PXY_METHOD(GetFVF)(PXY_THIS_ DWORD* pFVF);
	D912PXY_METHOD(CreateVertexShader)(PXY_THIS_ CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader);
	D912PXY_METHOD(SetVertexShader)(PXY_THIS_ IDirect3DVertexShader9* pShader);
	D912PXY_METHOD(GetVertexShader)(PXY_THIS_ IDirect3DVertexShader9** ppShader);
	D912PXY_METHOD(SetVertexShaderConstantF)(PXY_THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount);
	D912PXY_METHOD(GetVertexShaderConstantF)(PXY_THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount);
	D912PXY_METHOD(SetVertexShaderConstantI)(PXY_THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount);
	D912PXY_METHOD(GetVertexShaderConstantI)(PXY_THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount);
	D912PXY_METHOD(SetVertexShaderConstantB)(PXY_THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount);
	D912PXY_METHOD(GetVertexShaderConstantB)(PXY_THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount);
	D912PXY_METHOD(SetStreamSource)(PXY_THIS_ UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride);
	D912PXY_METHOD(GetStreamSource)(PXY_THIS_ UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride);
	D912PXY_METHOD(SetStreamSourceFreq)(PXY_THIS_ UINT StreamNumber, UINT Setting);
	D912PXY_METHOD(GetStreamSourceFreq)(PXY_THIS_ UINT StreamNumber, UINT* pSetting);
	D912PXY_METHOD(SetIndices)(PXY_THIS_ IDirect3DIndexBuffer9* pIndexData);
	D912PXY_METHOD(GetIndices)(PXY_THIS_ IDirect3DIndexBuffer9** ppIndexData);
	D912PXY_METHOD(CreatePixelShader)(PXY_THIS_ CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader);
	D912PXY_METHOD(SetPixelShader)(PXY_THIS_ IDirect3DPixelShader9* pShader);
	D912PXY_METHOD(GetPixelShader)(PXY_THIS_ IDirect3DPixelShader9** ppShader);
	D912PXY_METHOD(SetPixelShaderConstantF)(PXY_THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount);
	D912PXY_METHOD(GetPixelShaderConstantF)(PXY_THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount);
	D912PXY_METHOD(SetPixelShaderConstantI)(PXY_THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount);
	D912PXY_METHOD(GetPixelShaderConstantI)(PXY_THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount);
	D912PXY_METHOD(SetPixelShaderConstantB)(PXY_THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount);
	D912PXY_METHOD(GetPixelShaderConstantB)(PXY_THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount);
	D912PXY_METHOD(DrawRectPatch)(PXY_THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo);
	D912PXY_METHOD(DrawTriPatch)(PXY_THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo);
	D912PXY_METHOD(DeletePatch)(PXY_THIS_ UINT Handle);
	D912PXY_METHOD(CreateQuery)(PXY_THIS_ D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery);

//NC variants 
	D912PXY_METHOD_NC_(ULONG, ReleaseDevice)(THIS);
	D912PXY_METHOD_NC(TestCooperativeLevel)(THIS);
	D912PXY_METHOD_NC_(UINT, GetAvailableTextureMem)(THIS);
	D912PXY_METHOD_NC(EvictManagedResources)(THIS);
	D912PXY_METHOD_NC(GetDirect3D)(THIS_ IDirect3D9** ppD3D9);
	D912PXY_METHOD_NC(GetDeviceCaps)(THIS_ D3DCAPS9* pCaps);
	D912PXY_METHOD_NC(GetDisplayMode)(THIS_ UINT iSwapChain, D3DDISPLAYMODE* pMode);
	D912PXY_METHOD_NC(GetCreationParameters)(THIS_ D3DDEVICE_CREATION_PARAMETERS *pParameters);
	D912PXY_METHOD_NC(SetCursorProperties)(THIS_ UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap);
	D912PXY_METHOD_NC_(void, SetCursorPosition)(THIS_ int X, int Y, DWORD Flags);
	D912PXY_METHOD_NC_(BOOL, ShowCursor)(THIS_ BOOL bShow);
	D912PXY_METHOD_NC(CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain);
	D912PXY_METHOD_NC(GetSwapChain)(THIS_ UINT iSwapChain, IDirect3DSwapChain9** pSwapChain);
	D912PXY_METHOD_NC_(UINT, GetNumberOfSwapChains)(THIS);
	D912PXY_METHOD_NC(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);
	D912PXY_METHOD_NC(Present)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
	D912PXY_METHOD_NC(GetBackBuffer)(THIS_ UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer);
	D912PXY_METHOD_NC(GetRasterStatus)(THIS_ UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus);
	D912PXY_METHOD_NC(SetDialogBoxMode)(THIS_ BOOL bEnableDialogs);
	D912PXY_METHOD_NC_(void, SetGammaRamp)(THIS_ UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp);
	D912PXY_METHOD_NC_(void, GetGammaRamp)(THIS_ UINT iSwapChain, D3DGAMMARAMP* pRamp);
	D912PXY_METHOD_NC(CreateTexture)(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle);
	D912PXY_METHOD_NC(CreateVolumeTexture)(THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle);
	D912PXY_METHOD_NC(CreateCubeTexture)(THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle);
	D912PXY_METHOD_NC(CreateVertexBuffer)(THIS_ UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle);
	D912PXY_METHOD_NC(CreateIndexBuffer)(THIS_ UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle);
	D912PXY_METHOD_NC(CreateRenderTarget)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
	D912PXY_METHOD_NC(CreateDepthStencilSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
	D912PXY_METHOD_NC(UpdateSurface)(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint);
	D912PXY_METHOD_NC(UpdateTexture)(THIS_ IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture);
	D912PXY_METHOD_NC(GetRenderTargetData)(THIS_ IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface);
	D912PXY_METHOD_NC(GetFrontBufferData)(THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface);
	D912PXY_METHOD_NC(StretchRect)(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter);
	D912PXY_METHOD_NC(ColorFill)(THIS_ IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color);
	D912PXY_METHOD_NC(CreateOffscreenPlainSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
	D912PXY_METHOD_NC(SetRenderTarget)(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);
	D912PXY_METHOD_NC(GetRenderTarget)(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget);
	D912PXY_METHOD_NC(SetDepthStencilSurface)(THIS_ IDirect3DSurface9* pNewZStencil);
	D912PXY_METHOD_NC(GetDepthStencilSurface)(THIS_ IDirect3DSurface9** ppZStencilSurface);
	D912PXY_METHOD_NC(BeginScene)(THIS);
	D912PXY_METHOD_NC(EndScene)(THIS);
	D912PXY_METHOD_NC(Clear)(THIS_ DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
	D912PXY_METHOD_NC(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix);
	D912PXY_METHOD_NC(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix);
	D912PXY_METHOD_NC(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE, CONST D3DMATRIX*);
	D912PXY_METHOD_NC(SetViewport)(THIS_ CONST D3DVIEWPORT9* pViewport);
	D912PXY_METHOD_NC(GetViewport)(THIS_ D3DVIEWPORT9* pViewport);
	D912PXY_METHOD_NC(SetMaterial)(THIS_ CONST D3DMATERIAL9* pMaterial);
	D912PXY_METHOD_NC(GetMaterial)(THIS_ D3DMATERIAL9* pMaterial);
	D912PXY_METHOD_NC(SetLight)(THIS_ DWORD Index, CONST D3DLIGHT9*);
	D912PXY_METHOD_NC(GetLight)(THIS_ DWORD Index, D3DLIGHT9*);
	D912PXY_METHOD_NC(LightEnable)(THIS_ DWORD Index, BOOL Enable);
	D912PXY_METHOD_NC(GetLightEnable)(THIS_ DWORD Index, BOOL* pEnable);
	D912PXY_METHOD_NC(SetClipPlane)(THIS_ DWORD Index, CONST float* pPlane);
	D912PXY_METHOD_NC(GetClipPlane)(THIS_ DWORD Index, float* pPlane);
	D912PXY_METHOD_NC(SetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD Value);
	D912PXY_METHOD_NC(GetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD* pValue);
	D912PXY_METHOD_NC(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB);
	D912PXY_METHOD_NC(BeginStateBlock)(THIS);
	D912PXY_METHOD_NC(EndStateBlock)(THIS_ IDirect3DStateBlock9** ppSB);
	D912PXY_METHOD_NC(SetClipStatus)(THIS_ CONST D3DCLIPSTATUS9* pClipStatus);
	D912PXY_METHOD_NC(GetClipStatus)(THIS_ D3DCLIPSTATUS9* pClipStatus);
	D912PXY_METHOD_NC(GetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture9** ppTexture);
	D912PXY_METHOD_NC(SetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture9* pTexture);
	D912PXY_METHOD_NC(GetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue);
	D912PXY_METHOD_NC(SetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
	D912PXY_METHOD_NC(GetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue);
	D912PXY_METHOD_NC(SetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
	D912PXY_METHOD_NC(ValidateDevice)(THIS_ DWORD* pNumPasses);
	D912PXY_METHOD_NC(SetPaletteEntries)(THIS_ UINT PaletteNumber, CONST PALETTEENTRY* pEntries);
	D912PXY_METHOD_NC(GetPaletteEntries)(THIS_ UINT PaletteNumber, PALETTEENTRY* pEntries);
	D912PXY_METHOD_NC(SetCurrentTexturePalette)(THIS_ UINT PaletteNumber);
	D912PXY_METHOD_NC(GetCurrentTexturePalette)(THIS_ UINT *PaletteNumber);
	D912PXY_METHOD_NC(SetScissorRect)(THIS_ CONST RECT* pRect);
	D912PXY_METHOD_NC(GetScissorRect)(THIS_ RECT* pRect);
	D912PXY_METHOD_NC(SetSoftwareVertexProcessing)(THIS_ BOOL bSoftware);
	D912PXY_METHOD_NC_(BOOL, GetSoftwareVertexProcessing)(THIS);
	D912PXY_METHOD_NC(SetNPatchMode)(THIS_ float nSegments);
	D912PXY_METHOD_NC_(float, GetNPatchMode)(THIS);
	D912PXY_METHOD_NC(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
	D912PXY_METHOD_NC(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
	D912PXY_METHOD_NC(DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	D912PXY_METHOD_NC(DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	D912PXY_METHOD_NC(ProcessVertices)(THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags);
	D912PXY_METHOD_NC(CreateVertexDeclaration)(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl);
	D912PXY_METHOD_NC(SetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9* pDecl);
	D912PXY_METHOD_NC(GetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9** ppDecl);
	D912PXY_METHOD_NC(SetFVF)(THIS_ DWORD FVF);
	D912PXY_METHOD_NC(GetFVF)(THIS_ DWORD* pFVF);
	D912PXY_METHOD_NC(CreateVertexShader)(THIS_ CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader);
	D912PXY_METHOD_NC(SetVertexShader)(THIS_ IDirect3DVertexShader9* pShader);
	D912PXY_METHOD_NC(GetVertexShader)(THIS_ IDirect3DVertexShader9** ppShader);
	D912PXY_METHOD_NC(SetVertexShaderConstantF)(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount);
	D912PXY_METHOD_NC(GetVertexShaderConstantF)(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount);
	D912PXY_METHOD_NC(SetVertexShaderConstantI)(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount);
	D912PXY_METHOD_NC(GetVertexShaderConstantI)(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount);
	D912PXY_METHOD_NC(SetVertexShaderConstantB)(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount);
	D912PXY_METHOD_NC(GetVertexShaderConstantB)(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount);
	D912PXY_METHOD_NC(SetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride);
	D912PXY_METHOD_NC(GetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride);
	D912PXY_METHOD_NC(SetStreamSourceFreq)(THIS_ UINT StreamNumber, UINT Setting);
	D912PXY_METHOD_NC(GetStreamSourceFreq)(THIS_ UINT StreamNumber, UINT* pSetting);
	D912PXY_METHOD_NC(SetIndices)(THIS_ IDirect3DIndexBuffer9* pIndexData);
	D912PXY_METHOD_NC(GetIndices)(THIS_ IDirect3DIndexBuffer9** ppIndexData);
	D912PXY_METHOD_NC(CreatePixelShader)(THIS_ CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader);
	D912PXY_METHOD_NC(SetPixelShader)(THIS_ IDirect3DPixelShader9* pShader);
	D912PXY_METHOD_NC(GetPixelShader)(THIS_ IDirect3DPixelShader9** ppShader);
	D912PXY_METHOD_NC(SetPixelShaderConstantF)(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount);
	D912PXY_METHOD_NC(GetPixelShaderConstantF)(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount);
	D912PXY_METHOD_NC(SetPixelShaderConstantI)(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount);
	D912PXY_METHOD_NC(GetPixelShaderConstantI)(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount);
	D912PXY_METHOD_NC(SetPixelShaderConstantB)(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount);
	D912PXY_METHOD_NC(GetPixelShaderConstantB)(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount);
	D912PXY_METHOD_NC(DrawRectPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo);
	D912PXY_METHOD_NC(DrawTriPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo);
	D912PXY_METHOD_NC(DeletePatch)(THIS_ UINT Handle);
	D912PXY_METHOD_NC(CreateQuery)(THIS_ D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery);

//inner methods

	ID3D12Device* GetDev12Ptr() { return m_d12evice_ptr;  }
	D3D12_HEAP_PROPERTIES GetResourceHeap(D3D12_HEAP_TYPE Type);
	d912pxy_dheap* GetDHeap(UINT slot);

	void IFrameCleanupEnqeue(d912pxy_comhandler* obj);

	UINT InterruptThreads() { return threadInterruptState; };
	void LockThread(UINT thread);
	void InitLockThread(UINT thread);

	void LockAsyncThreads();
	void UnLockAsyncThreads();

	void CopyOriginalDX9Data(IDirect3DDevice9* dev, D3DDEVICE_CREATION_PARAMETERS* origPars, D3DPRESENT_PARAMETERS* origPP);
	void InitVFS();
	
	void InitClassFields();
	void InitThreadSyncObjects();
	void InitSingletons();
	void InitComPatches();
	void InitNullSRV();
	void InitDrawUPBuffers();
	void FreeAdditionalDX9Objects();
	void InitDescriptorHeaps();
	void PrintInfoBanner();
	void InitDefaultSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters);

	void NvGPU_force_highpower();
	void NvGPU_restore();

	ComPtr<ID3D12Device> SelectSuitableGPU();
	void SetupDevice(ComPtr<ID3D12Device> device);

	ID3D12RootSignature* ConstructRootSignature(D3D12_ROOT_SIGNATURE_DESC* rootSignatureDesc);

	void AddActiveThreads(UINT cnt) { activeThreadCount += cnt; };

	char* GetCurrentGPUName();

	void ExternalFlush();
	
	d912pxy_swapchain* GetPrimarySwapChain();

	uint32_t getCPUCoreCount() { return cpuCoreCount; }

	//megai2: variants of API calls

	HRESULT DrawPrimitive_Compat(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
	HRESULT DrawIndexedPrimitive_Compat(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
	D912PXY_METHOD_NC(DrawPrimitiveUP_StateUnsafe)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	D912PXY_METHOD_NC(DrawIndexedPrimitiveUP_StateUnsafe)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);

	HRESULT Present_PG(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
	HRESULT Present_Extra(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
	HRESULT Clear_Emulated(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
		//CAR = cpu api reduction
	HRESULT SetStreamSource_CAR(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride);
	HRESULT SetIndices_CAR(IDirect3DIndexBuffer9* pIndexData);
	HRESULT SetViewport_CAR(CONST D3DVIEWPORT9* pViewport);
	HRESULT SetScissorRect_CAR(CONST RECT* pRect);
	HRESULT SetRenderTarget_Compat(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);
	D912PXY_METHOD_NC(SetRenderState_Tracked)(THIS_ D3DRENDERSTATETYPE State, DWORD Value);
	D912PXY_METHOD_NC(SetSamplerState_Tracked)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
	D912PXY_METHOD_NC(CreateQuery_Optimized)(THIS_ D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery);

	//com routes for them
	D912PXY_METHOD(DrawIndexedPrimitive_Compat)(PXY_THIS_ D3DPRIMITIVETYPE, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
	D912PXY_METHOD(DrawPrimitiveUP_StateUnsafe)(PXY_THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	D912PXY_METHOD(DrawIndexedPrimitiveUP_StateUnsafe)(PXY_THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	D912PXY_METHOD(Present_PG)(PXY_THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);	
	D912PXY_METHOD(Present_Extra)(PXY_THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
	D912PXY_METHOD(Clear_Emulated)(PXY_THIS_ DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
	D912PXY_METHOD(SetStreamSource_CAR)(PXY_THIS_ UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride);
	D912PXY_METHOD(SetIndices_CAR)(PXY_THIS_ IDirect3DIndexBuffer9* pIndexData);
	D912PXY_METHOD(SetViewport_CAR)(PXY_THIS_ CONST D3DVIEWPORT9* pViewport);
	D912PXY_METHOD(SetScissorRect_CAR)(PXY_THIS_ CONST RECT* pRect);
	D912PXY_METHOD(SetRenderTarget_Compat)(PXY_THIS_ DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);
	D912PXY_METHOD(SetRenderState_Tracked)(PXY_THIS_ D3DRENDERSTATETYPE State, DWORD Value);
	D912PXY_METHOD(SetSamplerState_Tracked)(PXY_THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
	D912PXY_METHOD(CreateQuery_Optimized)(PXY_THIS_ D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery);
	D912PXY_METHOD(DrawPrimitive_Compat)(PXY_THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
	
	HRESULT InnerPresentExecute();
	void InnerPresentFinish();

private:
	std::atomic<LONG> threadInterruptState { 0 };
	d912pxy_thread_lock threadLockdEvents[PXY_INNER_THREADID_MAX];
	d912pxy_thread_lock threadLock;
	d912pxy_thread_lock cleanupLock;	
	d912pxy_thread_lock swapOpLock;
	UINT activeThreadCount;
	d912pxy_thread_lock isRunning;

	ComPtr<ID3D12Device> m_d12evice;
	ID3D12Device* m_d12evice_ptr;
	
	
	d912pxy_surface_ops* m_emulatedSurfaceOps;

	d912pxy_dheap* m_dheaps[PXY_INNER_MAX_DSC_HEAPS];
	
	d912pxy_swapchain* swapchains[PXY_INNER_MAX_SWAP_CHAINS];
	
	d912pxy_surface* mNullTexture;
	UINT mNullTextureSRV;

	IDirect3DQuery9* mFakeOccQuery;

	//info data and dx9 catch'ups
	DWORD gpu_totalVidmemMB;
	D3DDISPLAYMODE cached_dx9displaymode;
	D3DCAPS9 cached_dx9caps;
	D3DDEVICE_CREATION_PARAMETERS creationData;
	D3DPRESENT_PARAMETERS initialPresentParameters;

	void* initPtr;

	char GPUNameA[128];
	uint32_t cpuCoreCount=1;

	//dx9 api hacks
	UINT32 gpuWriteDsc;

	d912pxy_performance_graph* perfGraph;

	//nvapi 
	nvapi_fptrs* nvapi;

	UINT nvapi_dynPstateChanged;

	UINT convertTexStage(UINT stage) { return (stage & 0xF) + 16 * ((stage >> 4) != 0); }
};


/*

Original work: https://github.com/iorlas/D3D9Proxy

*/
//FIXME: correct this to be right, as i'm not shure
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
#pragma once
#include "stdafx.h"

typedef struct Direct3DDevice9Proxy_create_params {
	UINT Adapter;
	D3DDEVTYPE DeviceType;
	HWND hFocusWindow;
	DWORD BehaviorFlags;
	D3DPRESENT_PARAMETERS* pPresentationParameters;
} Direct3DDevice9Proxy_create_params;

class IDirect3DDevice9Proxy : public IDirect3DDevice9
{
public:
	IDirect3DDevice9Proxy(IDirect3D9* pOriginal, Direct3DDevice9Proxy_create_params cp);
	virtual ~IDirect3DDevice9Proxy(void);

	//The original DX9 function definitions. Damn, it's a lot...
	HRESULT WINAPI QueryInterface (REFIID riid, void** ppvObj);
	ULONG WINAPI AddRef(void);
	ULONG WINAPI Release(void);
	HRESULT WINAPI TestCooperativeLevel(void);
	UINT WINAPI GetAvailableTextureMem(void);
	HRESULT WINAPI EvictManagedResources(void);
	HRESULT WINAPI GetDirect3D(IDirect3D9** ppD3D9);
	HRESULT WINAPI GetDeviceCaps(D3DCAPS9* pCaps);
	HRESULT WINAPI GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode);
	HRESULT WINAPI GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters);
	HRESULT WINAPI SetCursorProperties(UINT XHotSpot, UINT YHotSpot,IDirect3DSurface9* pCursorBitmap);
	void WINAPI SetCursorPosition(int X, int Y, DWORD Flags);
	BOOL WINAPI ShowCursor(BOOL bShow);
	HRESULT WINAPI CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain)  ;
	HRESULT WINAPI GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain);
	UINT WINAPI GetNumberOfSwapChains(void);
	HRESULT WINAPI Reset(D3DPRESENT_PARAMETERS* pPresentationParameters);
	HRESULT WINAPI Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
	HRESULT WINAPI GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer);
	HRESULT WINAPI GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus);
	HRESULT WINAPI SetDialogBoxMode(BOOL bEnableDialogs);
	void WINAPI SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp);
	void WINAPI GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp);
	HRESULT WINAPI CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle);
	HRESULT WINAPI CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle);
	HRESULT WINAPI CreateCubeTexture(UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle);
	HRESULT WINAPI CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle);
	HRESULT WINAPI CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle);
	HRESULT WINAPI CreateRenderTarget(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
	HRESULT WINAPI CreateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
	HRESULT WINAPI UpdateSurface(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint);
	HRESULT WINAPI UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture);
	HRESULT WINAPI GetRenderTargetData(IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface);
	HRESULT WINAPI GetFrontBufferData(UINT iSwapChain,IDirect3DSurface9* pDestSurface);
	HRESULT WINAPI StretchRect(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter);
	HRESULT WINAPI ColorFill(IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color);
	HRESULT WINAPI CreateOffscreenPlainSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
	HRESULT WINAPI SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget);
	HRESULT WINAPI GetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget);
	HRESULT WINAPI SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil);
	HRESULT WINAPI GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface);
	HRESULT WINAPI BeginScene(void);
	HRESULT WINAPI EndScene(void);
	HRESULT WINAPI Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil);
	HRESULT WINAPI SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix);
	HRESULT WINAPI GetTransform(D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix);
	HRESULT WINAPI MultiplyTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix);
	HRESULT WINAPI SetViewport(CONST D3DVIEWPORT9* pViewport);
	HRESULT WINAPI GetViewport(D3DVIEWPORT9* pViewport);
	HRESULT WINAPI SetMaterial(CONST D3DMATERIAL9* pMaterial);
	HRESULT WINAPI GetMaterial(D3DMATERIAL9* pMaterial);
	HRESULT WINAPI SetLight(DWORD Index,CONST D3DLIGHT9* pLight);
	HRESULT WINAPI GetLight(DWORD Index,D3DLIGHT9* pLight);
	HRESULT WINAPI LightEnable(DWORD Index,BOOL Enable);
	HRESULT WINAPI GetLightEnable(DWORD Index,BOOL* pEnable);
	HRESULT WINAPI SetClipPlane(DWORD Index,CONST float* pPlane);
	HRESULT WINAPI GetClipPlane(DWORD Index,float* pPlane);
	HRESULT WINAPI SetRenderState(D3DRENDERSTATETYPE State,DWORD Value);
	HRESULT WINAPI GetRenderState(D3DRENDERSTATETYPE State,DWORD* pValue);
	HRESULT WINAPI CreateStateBlock(D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB);
	HRESULT WINAPI BeginStateBlock(void);
	HRESULT WINAPI EndStateBlock(IDirect3DStateBlock9** ppSB);
	HRESULT WINAPI SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus);
	HRESULT WINAPI GetClipStatus(D3DCLIPSTATUS9* pClipStatus);
	HRESULT WINAPI GetTexture(DWORD Stage,IDirect3DBaseTexture9** ppTexture);
	HRESULT WINAPI SetTexture(DWORD Stage,IDirect3DBaseTexture9* pTexture);
	HRESULT WINAPI GetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue);
	HRESULT WINAPI SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value);
	HRESULT WINAPI GetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue);
	HRESULT WINAPI SetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value);
	HRESULT WINAPI ValidateDevice(DWORD* pNumPasses);
	HRESULT WINAPI SetPaletteEntries(UINT PaletteNumber,CONST PALETTEENTRY* pEntries);
	HRESULT WINAPI GetPaletteEntries(UINT PaletteNumber,PALETTEENTRY* pEntries);
	HRESULT WINAPI SetCurrentTexturePalette(UINT PaletteNumber);
	HRESULT WINAPI GetCurrentTexturePalette(UINT *PaletteNumber);
	HRESULT WINAPI SetScissorRect(CONST RECT* pRect);
	HRESULT WINAPI GetScissorRect(RECT* pRect);
	HRESULT WINAPI SetSoftwareVertexProcessing(BOOL bSoftware);
	BOOL WINAPI GetSoftwareVertexProcessing(void);
	HRESULT WINAPI SetNPatchMode(float nSegments);
	float WINAPI GetNPatchMode(void);
	HRESULT WINAPI DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount);
	HRESULT WINAPI DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount);
	HRESULT WINAPI DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride);
	HRESULT WINAPI DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride);
	HRESULT WINAPI ProcessVertices(UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags);
	HRESULT WINAPI CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl);
	HRESULT WINAPI SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl);
	HRESULT WINAPI GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl);
	HRESULT WINAPI SetFVF(DWORD FVF);
	HRESULT WINAPI GetFVF(DWORD* pFVF);
	HRESULT WINAPI CreateVertexShader(CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader);
	HRESULT WINAPI SetVertexShader(IDirect3DVertexShader9* pShader);
	HRESULT WINAPI GetVertexShader(IDirect3DVertexShader9** ppShader);
	HRESULT WINAPI SetVertexShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount);
	HRESULT WINAPI GetVertexShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount);
	HRESULT WINAPI SetVertexShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount);
	HRESULT WINAPI GetVertexShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount);
	HRESULT WINAPI SetVertexShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount);
	HRESULT WINAPI GetVertexShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount);
	HRESULT WINAPI SetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride);
	HRESULT WINAPI GetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* OffsetInBytes,UINT* pStride);
	HRESULT WINAPI SetStreamSourceFreq(UINT StreamNumber,UINT Divider);
	HRESULT WINAPI GetStreamSourceFreq(UINT StreamNumber,UINT* Divider);
	HRESULT WINAPI SetIndices(IDirect3DIndexBuffer9* pIndexData);
	HRESULT WINAPI GetIndices(IDirect3DIndexBuffer9** ppIndexData);
	HRESULT WINAPI CreatePixelShader(CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader);
	HRESULT WINAPI SetPixelShader(IDirect3DPixelShader9* pShader);
	HRESULT WINAPI GetPixelShader(IDirect3DPixelShader9** ppShader);
	HRESULT WINAPI SetPixelShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount);
	HRESULT WINAPI GetPixelShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount);
	HRESULT WINAPI SetPixelShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount);
	HRESULT WINAPI GetPixelShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount);
	HRESULT WINAPI SetPixelShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount);
	HRESULT WINAPI GetPixelShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount);
	HRESULT WINAPI DrawRectPatch(UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo);
	HRESULT WINAPI DrawTriPatch(UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo);
	HRESULT WINAPI DeletePatch(UINT Handle);
	HRESULT WINAPI CreateQuery(D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery);

	//megai2: use a special post init proc for proxy that not use d3d9dev for real

	virtual HRESULT PostInit(IDirect3DDevice9** realDev);

	Direct3DDevice9Proxy_create_params* GetOrigD3D9Call() { return &origD3D_create_call; };

	d912pxy_performance_graph* perfGraph;
	void InitPerfGraph();
	
private:
	IDirect3DDevice9 *origIDirect3DDevice9;
	IDirect3D9 *origID3D9;

	D3DPRESENT_PARAMETERS origPP;

	IP7_Trace* log_trace;
	IP7_Trace::hModule log_module;

	UINT batchCnt;

protected:

	Direct3DDevice9Proxy_create_params origD3D_create_call;	
};


//callback for d3d9 dev creation

typedef IDirect3DDevice9* (*d3d9ProxyCB_OnDevCreate)(IDirect3DDevice9Proxy* device, IDirect3D9* obj);
typedef void(*d3d9ProxyCB_OnDevDestroy)(IDirect3DDevice9* device);
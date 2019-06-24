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

#define PXY_INNER_MAX_VDECL_LEN 30

typedef struct d912pxy_vdecl_elesn {
	char s[255];
} d912pxy_vdecl_elesn;

class d912pxy_vdecl : public IDirect3DVertexDeclaration9, public d912pxy_comhandler
{
public:
	d912pxy_vdecl(d912pxy_device* dev, const D3DVERTEXELEMENT9* data);
	~d912pxy_vdecl();

	/*** IUnknown methods ***/
	D912PXY_METHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	D912PXY_METHOD_(ULONG, AddRef)(THIS);
	D912PXY_METHOD_(ULONG, Release)(THIS);

	/*** IDirect3DVertexDeclaration9 methods ***/
	D912PXY_METHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice);
	D912PXY_METHOD(GetDeclaration)(THIS_ D3DVERTEXELEMENT9* pElement, UINT* pNumElements);

	D3DVERTEXELEMENT9* GetDeclarationPtr(UINT* pNumElements);
	D3D12_INPUT_LAYOUT_DESC GetD12IA_InputElementFmt() { D3D12_INPUT_LAYOUT_DESC ret; ret.NumElements = declLen - 1; ret.pInputElementDescs = declData12; return ret; }

	void ModifyStreamElementType(UINT stream, D3D12_INPUT_CLASSIFICATION newMode);

	d912pxy_vdecl* GetInstancedModification();

	UINT32 GetHash();

	UINT GetUsedStreams();

private:
	
	D3DVERTEXELEMENT9 declData[PXY_INNER_MAX_VDECL_LEN];
	D3D12_INPUT_ELEMENT_DESC declData12[PXY_INNER_MAX_VDECL_LEN];
	d912pxy_vdecl_elesn semantics[PXY_INNER_MAX_VDECL_LEN];

	DWORD declLen;
	DWORD usedStreamSlots;
	UINT32 mHash;

	d912pxy_vdecl* instancedDecl;
};


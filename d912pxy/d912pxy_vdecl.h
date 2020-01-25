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

#define PXY_INNER_MAX_VDECL_LEN 20

typedef struct d912pxy_vdecl_elesn {
	char s[10];
} d912pxy_vdecl_elesn;

class d912pxy_vdecl : public d912pxy_vtable, public d912pxy_comhandler
{
public:	
	static d912pxy_vdecl* d912pxy_vdecl_com(const D3DVERTEXELEMENT9* data);

	~d912pxy_vdecl();

	D912PXY_METHOD(GetDeclaration)(PXY_THIS_ D3DVERTEXELEMENT9* pElement, UINT* pNumElements);

	D912PXY_METHOD_NC(GetDeclaration)(THIS_ D3DVERTEXELEMENT9* pElement, UINT* pNumElements);
	
	D3DVERTEXELEMENT9* GetDeclarationPtr(UINT* pNumElements);
	D3D12_INPUT_LAYOUT_DESC* GetD12IA_InputElementFmt();

	d912pxy_vdecl* GetInstancedModification(UINT stream, D3D12_INPUT_CLASSIFICATION newMode);

	UINT32 GetHash();
	UINT GetUsedStreams();
	
private:
	d912pxy_vdecl(const D3DVERTEXELEMENT9* data);
	void ModifyStreamElementType(UINT stream, D3D12_INPUT_CLASSIFICATION newMode);
	void UpdateHash();

	D3DVERTEXELEMENT9 declData[PXY_INNER_MAX_VDECL_LEN];
	D3D12_INPUT_ELEMENT_DESC declData12[PXY_INNER_MAX_VDECL_LEN];
	d912pxy_vdecl_elesn semantics[PXY_INNER_MAX_VDECL_LEN];
	D3D12_INPUT_LAYOUT_DESC inputEleFmt;

	DWORD declLen;
	DWORD usedStreamSlots;
	UINT32 mHash;
	UINT32 mInstancedModificationMask;

	d912pxy_vdecl* instancedDecl;
};

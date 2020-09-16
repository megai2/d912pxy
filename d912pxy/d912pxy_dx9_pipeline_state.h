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

#define PXY_DX9_MAX_RS_INDEX 226
#define PXY_DX9_RS_INITIAL_VALUE 7

class d912pxy_dx9_pipeline_state : public d912pxy_noncom
{
public:
	d912pxy_dx9_pipeline_state();
	~d912pxy_dx9_pipeline_state();

	void Init();
	void UnInit();

	////////setters

	//things that affect pso only
	void SetDX9RS(D3DRENDERSTATETYPE State, DWORD Value);
	void SetDX9RSTracked(D3DRENDERSTATETYPE State, DWORD Value);

	void VShader(d912pxy_shader* vs);
	void PShader(d912pxy_shader* ps);
	void IAFormat(d912pxy_vdecl* vertexDcl);
	void IAFormatInstanced(d912pxy_vdecl* vertexDcl);

	//things that affect pso but indirectly

	void RTVFormat(DXGI_FORMAT fmt, UINT idx);
	void DSVFormat(DXGI_FORMAT fmt);
	void OMReflect(UINT RTcnt, D3D12_CPU_DESCRIPTOR_HANDLE* dsv);
	void UpdateCompareSampler(UINT stage, bool use);

	///////getters

	d912pxy_vdecl* GetIAFormat();
	d912pxy_shader* GetPShader();
	d912pxy_shader* GetVShader();

	d912pxy_pso_item* GetCurrentCPSO();
	d912pxy_trimmed_pso_desc GetCurrentDesc();
	void SetCurrentDesc(d912pxy_trimmed_pso_desc& dsc);

	DWORD GetDX9RsValue(D3DRENDERSTATETYPE State);

	//////

	void Use();
	void UseCompiled(d912pxy_pso_item* it);
	void UseWithFeedbackPtr(void** feedback);
	void MarkDirty();

	//extra dx9 rener state funcs
	
	fv4Color TransformBlendFactor(DWORD val);
	DWORD TransformBlend2AlphaBlend(DWORD val);

private:
	void ProcessDX9RSChange(D3DRENDERSTATETYPE State, DWORD Value);

	d912pxy_pso_item* cCPSO;
	d912pxy_vdecl* cVDecl;
	d912pxy_trimmed_pso_desc psoDesc;
	DWORD DX9RSvalues[PXY_DX9_MAX_RS_INDEX];

	bool isDirty;
	UINT8 compareSamplerStage;
};

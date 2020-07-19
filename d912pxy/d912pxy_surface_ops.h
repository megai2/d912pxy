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

class d912pxy_surface_ops : public d912pxy_noncom
{
public:
	d912pxy_surface_ops(d912pxy_device* dev);
	~d912pxy_surface_ops();

	void Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
	void ClearIter(D3DRECT* pRect);
	void StretchRect(d912pxy_surface* pSourceSurface, d912pxy_surface* pDestSurface);

private:
	class SavedIFrameState
	{
	public:
		SavedIFrameState();
		~SavedIFrameState();

	private:
		d912pxy_iframe::StreamBindsHolder streamBinds;
		d912pxy_trimmed_pso_desc psoDsc;
		d912pxy_vdecl* vdecl;
		D3D12_VIEWPORT viewport;
		UINT restoreScissor;
	};

	enum shaderSet {
		SHSET_CLEAR,
		SHSET_STRETCH,
		SHSET_COUNT
	};

	void SetCommonState(shaderSet idx);

	d912pxy_shader* vs[SHSET_COUNT];
	d912pxy_shader* ps[SHSET_COUNT];
	d912pxy_vdecl* vdcl;

	d912pxy_vstream* vBuf;
	d912pxy_vstream* iBuf;
	d912pxy_trimmed_pso_desc localPSO;

	// bilinear, uvw wrap
	constexpr static d912pxy_trimmed_sampler_dsc bilinearSamplerDsc = { 0x92,0x49, 0, 0 };

	UINT bilinearSamplerId;
};


/*
MIT License

Copyright(c) 2019-2020 megai2

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

d912pxy_surface_clear::d912pxy_surface_clear(d912pxy_device * dev) : d912pxy_noncom( L"surface_clear")
{
	ps = d912pxy_shader::d912pxy_shader_com(0, 0, 0x5);
	vs = d912pxy_shader::d912pxy_shader_com(1, 0, 0x4);

	const D3DVERTEXELEMENT9 vDclElements[] = {
		{0,0,D3DDECLTYPE_FLOAT4,0,D3DDECLUSAGE_POSITION,0},
		{0xFF,0,D3DDECLTYPE_UNUSED,0,0,0}
	};

	vBuf = d912pxy_s.pool.vstream.GetVStreamObject(4 * 4 * 4, 0, 0);
	iBuf = d912pxy_s.pool.vstream.GetVStreamObject(2*3*2, D3DFMT_INDEX16, 1);

	float* vdat;
	WORD* idat;

	vBuf->Lock(0, 0, (void**)&vdat, 0);
	iBuf->Lock(0, 0, (void**)&idat, 0);

	ZeroMemory(vdat, 4 * 4 * 4);

	vdat[4 * 1 + 0] = 1;
	vdat[4 * 2 + 0] = 1;
	vdat[4 * 2 + 1] = 1;	
	vdat[4 * 3 + 1] = 1;

	idat[0] = 0;
	idat[1] = 3;
	idat[2] = 1;
	idat[3] = 1;
	idat[4] = 3;
	idat[5] = 2;

	vBuf->Unlock();
	iBuf->Unlock();

	
	vdcl = d912pxy_vdecl::d912pxy_vdecl_com(vDclElements);
}

d912pxy_surface_clear::~d912pxy_surface_clear()
{
	vBuf->Release();
	iBuf->Release();

	if (ps)
		ps->Release();

	if (vs)
		vs->Release();

	vdcl->Release();
}

void d912pxy_surface_clear::Clear(DWORD Count, const D3DRECT * pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	//TODO obsolete, remove

	//megai2: save current pso & related
/*	d912pxy_trimmed_pso_desc psoDsc = d912pxy_s.render.state.pso.GetCurrentDsc();
	d912pxy_trimmed_pso_desc oldDsc = psoDsc;

	d912pxy_vstream* oi = d912pxy_s.render.iframe.GetIBuf();
	d912pxy_device_streamsrc oss = d912pxy_s.render.iframe.GetStreamSource(0);
	d912pxy_device_streamsrc ossi = d912pxy_s.render.iframe.GetStreamSource(1);
	d912pxy_vdecl* oldVDecl = d912pxy_s.render.state.pso.GetIAFormat();

	UINT oldStencilRef;
	if (Flags & D3DCLEAR_STENCIL)	
		oldStencilRef = d912pxy_s.render.state.pso.GetDX9RsValue(D3DRS_STENCILREF);

	UINT restoreScissor = d912pxy_s.render.state.pso.GetDX9RsValue(D3DRS_SCISSORTESTENABLE);

	//set needed things

	d912pxy_s.render.iframe.SetStreamFreq(0, 1);
	d912pxy_s.render.iframe.SetStreamFreq(1, 0);

	d912pxy_s.render.iframe.SetIBuf(iBuf);
	d912pxy_s.render.iframe.SetVBuf(vBuf, 0, 0, 4 * 4);

	d912pxy_s.render.state.pso.IAFormat(vdcl);
	psoDsc->ref.PS = ps;
	psoDsc->ref.VS = vs;

	d912pxy_s.dev.SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	d912pxy_s.dev.SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, false);
	d912pxy_s.dev.SetRenderState(D3DRS_SCISSORTESTENABLE, 0);

	psoDsc->val.rast.cullMode = D3D12_CULL_MODE_NONE;

	if (Flags & D3DCLEAR_TARGET)
		psoDsc->val.rt0.blend.writeMask = 0xF;
	else
		psoDsc->val.rt0.blend.writeMask = 0;

	if (Flags & D3DCLEAR_ZBUFFER)
	{
		d912pxy_s.dev.SetRenderState(D3DRS_ZENABLE, 1);
		d912pxy_s.dev.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
		d912pxy_s.dev.SetRenderState(D3DRS_ZWRITEENABLE, 1);
	}
	else {
		d912pxy_s.dev.SetRenderState(D3DRS_ZENABLE, 0);
	}

	if (Flags & D3DCLEAR_STENCIL)
	{
		d912pxy_s.dev.SetRenderState(D3DRS_STENCILREF, Stencil);
		d912pxy_s.dev.SetRenderState(D3DRS_STENCILENABLE, 1);
		d912pxy_s.dev.SetRenderState(D3DRS_STENCILWRITEMASK, 0xFF);
		d912pxy_s.dev.SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
		d912pxy_s.dev.SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
	}
	else {
		d912pxy_s.dev.SetRenderState(D3DRS_STENCILENABLE, 0);
	}

	//set shader constants

	if (Flags & D3DCLEAR_TARGET)
	{
		float fvColor[4] =
		{
			((Color >> 16) & 0xFF) / 255.0f,			
			((Color >> 0) & 0xFF) / 255.0f,
			((Color >> 8) & 0xFF) / 255.0f,	
			((Color >> 24) & 0xFF) / 255.0f
		};

		d912pxy_s.render.batch.SetShaderConstF(1, PXY_INNER_EXTRA_SHADER_CONST_CLEAR_COLOR, 1, fvColor);
	}

	float fvZVal[4] = {
		Z,
		1,
		1,
		0
	};

	if (Count)
	{
		D3D12_VIEWPORT* cuVWP = d912pxy_s.render.iframe.GetViewport();

		fvZVal[1] = cuVWP->Width;
		fvZVal[2] = cuVWP->Height;
	}

	d912pxy_s.render.batch.SetShaderConstF(1, PXY_INNER_EXTRA_SHADER_CONST_CLEAR_ZWH, 1, fvZVal);

	//do iterated clear for each rect specified or use viewport rect

	if (Count)
	{
		for (int i = 0; i != Count; ++i)
		{
			ClearIter((D3DRECT*)&pRects[i]);
		}
	}
	else {
		static D3DRECT wpRect = { 0,0,1,1 };
	
		ClearIter(&wpRect);
	}

	//restore PSO and other things

	d912pxy_s.render.iframe.SetIBuf(oi);
	d912pxy_s.render.iframe.SetVBuf(oss.buffer, 0, oss.offset, oss.stride);
	d912pxy_s.render.iframe.SetStreamFreq(0, oss.divider);
	d912pxy_s.render.iframe.SetStreamFreq(1, ossi.divider);

	*psoDsc = oldDsc;

	d912pxy_s.render.state.pso.IAFormat(oldVDecl);

	d912pxy_s.render.iframe.ProcessSurfaceBinds(1);

	if (Flags & D3DCLEAR_STENCIL)
		d912pxy_s.dev.SetRenderState(D3DRS_STENCILREF, oldStencilRef);	

	if (restoreScissor)
		d912pxy_s.render.state.pso.SetDX9RS(D3DRS_SCISSORTESTENABLE, 1);

	d912pxy_s.render.state.pso.MarkDirty();*/

}

void d912pxy_surface_clear::ClearIter(D3DRECT * pRect)
{
	float fvRect[4] = {
		(float)pRect->x1,
		(float)pRect->y1,
		(float)pRect->x2,
		(float)pRect->y2
	};

	d912pxy_s.render.batch.SetShaderConstF(1, PXY_INNER_EXTRA_SHADER_CONST_CLEAR_RECT, 1, fvRect);

	PXY_COM_CAST_(IDirect3DDevice9, &d912pxy_s.dev)->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
}

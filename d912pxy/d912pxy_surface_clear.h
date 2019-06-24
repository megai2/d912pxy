#pragma once
#include "stdafx.h"

class d912pxy_surface_clear : public d912pxy_noncom
{
public:
	d912pxy_surface_clear(d912pxy_device* dev);
	~d912pxy_surface_clear();

	void Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);

	void ClearIter(D3DRECT* pRect);

private:
	d912pxy_vshader* vs;
	d912pxy_pshader* ps;
	d912pxy_vdecl* vdcl;

	d912pxy_vstream* vBuf;
	d912pxy_vstream* iBuf;
};


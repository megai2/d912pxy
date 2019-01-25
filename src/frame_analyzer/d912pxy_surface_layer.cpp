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

d912pxy_surface_layer::d912pxy_surface_layer(d912pxy_surface * iBase, UINT32 iSubres, UINT32 iBSize, UINT32 iWPitch, UINT32 iWidth, UINT32 imemPerPix)
{
	base = iBase;
	subres = iSubres;
	wPitch = iWPitch;
	width = iWidth;
	memPerPix = imemPerPix;	

	surfMem = malloc(iBSize);	
	intRefc = 0;
}

d912pxy_surface_layer::~d912pxy_surface_layer()
{
	free(surfMem);
}

#define D912PXY_METHOD_IMPL_CN d912pxy_surface_layer

D912PXY_METHOD_IMPL(QueryInterface)(THIS_ REFIID riid, void** ppvObj)
{
	return base->QueryInterface(riid, ppvObj);
}

D912PXY_METHOD_IMPL_(ULONG, AddRef)(THIS)
{
	++intRefc;
	base->AddRef();
	return intRefc;
}

D912PXY_METHOD_IMPL_(ULONG, Release)(THIS)
{
	--intRefc;
	base->Release();
	return intRefc;
}

/*** IDirect3DResource9 methods ***/
D912PXY_METHOD_IMPL(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) { return base->GetDevice(ppDevice); }
D912PXY_METHOD_IMPL(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags) { return base->SetPrivateData(refguid, pData, SizeOfData, Flags); }
D912PXY_METHOD_IMPL(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData) { return base->GetPrivateData(refguid, pData, pSizeOfData); }
D912PXY_METHOD_IMPL(FreePrivateData)(THIS_ REFGUID refguid) { return base->FreePrivateData(refguid); }
D912PXY_METHOD_IMPL_(DWORD, SetPriority)(THIS_ DWORD PriorityNew) { return base->SetPriority(PriorityNew); }
D912PXY_METHOD_IMPL_(DWORD, GetPriority)(THIS) { return base->GetPriority(); }
D912PXY_METHOD_IMPL_(void, PreLoad)(THIS) { base->PreLoad(); }
D912PXY_METHOD_IMPL_(D3DRESOURCETYPE, GetType)(THIS) { return base->GetType(); }

//surface methods
D912PXY_METHOD_IMPL(GetContainer)(THIS_ REFIID riid, void** ppContainer)
{
	return D3DERR_INVALIDCALL;
}

D912PXY_METHOD_IMPL(GetDesc)(THIS_ D3DSURFACE_DESC *pDesc)
{
	return base->GetDesc(pDesc);	
}

D912PXY_METHOD_IMPL(LockRect)(THIS_ D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
	void* surfMemRef = surfMem;

	if (pRect)
	{
		pLockedRect->pBits = (void*)((intptr_t)surfMemRef + (intptr_t)((pRect->left + pRect->top*width)*memPerPix));
	}
	else {
		pLockedRect->pBits = (void*)((intptr_t)surfMemRef);
	}

	pLockedRect->Pitch = wPitch;

	if ((Flags & D3DLOCK_READONLY) == 0)
		++lockDepth;

	return D3D_OK;
}

D912PXY_METHOD_IMPL(UnlockRect)(THIS)
{
	if (lockDepth)
		--lockDepth;

	if (!lockDepth)
	{
		d912pxy_s(texloadThread)->IssueUpload(base, surfMem, subres);
	}

	return D3D_OK;
}

D912PXY_METHOD_IMPL(GetDC)(THIS_ HDC *phdc)
{
	return D3DERR_INVALIDCALL;
}

D912PXY_METHOD_IMPL(ReleaseDC)(THIS_ HDC hdc)
{
	return D3D_OK;
}

#undef D912PXY_METHOD_IMPL_CN

void d912pxy_surface_layer::SetDirtyRect(UINT32 left, UINT32 right, UINT32 top, UINT32 bottom)
{
	++isDrect;

	drect.x1 = left;
	drect.x2 = right;
	drect.y1 = top;
	drect.y2 = bottom;
}

BOOL FileExists(LPCSTR szPath)
{
	DWORD dwAttrib = GetFileAttributesA(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

HRESULT d912pxy_surface_layer::UnlockRectEx(UINT32 transform)
{
	return UnlockRect();
}

void * d912pxy_surface_layer::SurfacePixel(UINT32 x, UINT32 y)
{
	void* surfMemRef = surfMem;

	return (void*)((intptr_t)surfMemRef + (intptr_t)((x + y*width)*memPerPix));
}

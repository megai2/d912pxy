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

d912pxy_surface_layer::d912pxy_surface_layer(d912pxy_com_object * iBase, UINT32 iSubres, UINT32 iBSize, UINT32 iWPitch, UINT32 iWidth, UINT32 imemPerPix)
	: base(iBase)
	, subres(iSubres)
	, wPitch(iWPitch)
	, width(iWidth)
	, memPerPix(imemPerPix)
	, lockDepth(0)
	, lockWrite(0)
	, isDrect(0)
	, drect{ 0 }
{
	PXY_MALLOC_GPU_HOST_COPY(surfMem, iBSize, void*);
}

d912pxy_surface_layer::~d912pxy_surface_layer()
{
	PXY_FREE_GPU_HOST_COPY(surfMem);	 
}

#define D912PXY_METHOD_IMPL_CN d912pxy_surface_layer

D912PXY_METHOD_IMPL_NC(QueryInterface)(THIS_ REFIID riid, void** ppvObj)
{
	return base->com.QueryInterface(riid, ppvObj);
}

D912PXY_METHOD_IMPL_NC_(ULONG, AddRef)(THIS)
{
	return base->com.AddRef();
}

D912PXY_METHOD_IMPL_NC_(ULONG, Release)(THIS)
{
	return base->com.Release();
}

D912PXY_METHOD_IMPL_NC(GetDesc)(THIS_ D3DSURFACE_DESC *pDesc)
{
	*pDesc = base->surface.GetDX9DescAtLevel(subres);
	return D3D_OK;
}

D912PXY_METHOD_IMPL_NC(LockRect)(THIS_ D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
	void* surfMemRef = surfMem;

	if (pRect)
	{
		pLockedRect->pBits = (void*)((intptr_t)surfMemRef + ((intptr_t)pRect->left*memPerPix + (intptr_t)pRect->top*wPitch));
	}
	else {
		pLockedRect->pBits = (void*)((intptr_t)surfMemRef);
	}

	pLockedRect->Pitch = wPitch;

	if (!(Flags & D3DLOCK_READONLY))
	{
		lockWrite |= 1;
	}

	++lockDepth;

	return D3D_OK;
}

D912PXY_METHOD_IMPL_NC(UnlockRect)(THIS)
{
	--lockDepth;

	if (!lockDepth && lockWrite)
	{
		lockWrite = 0;
		d912pxy_s.thread.texld.IssueUpload(&base->surface, surfMem, subres);
	}

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

void * d912pxy_surface_layer::SurfacePixel(UINT32 x, UINT32 y)
{
	void* surfMemRef = surfMem;

	return (void*)((intptr_t)surfMemRef + (x + (intptr_t)y*width)*memPerPix);
}

d912pxy_surface_layer * d912pxy_surface_layer::d912pxy_surface_layer_com(d912pxy_com_object * iBase, UINT32 iSubres, UINT32 iBSize, UINT32 iWPitch, UINT32 iWidth, UINT32 imemPerPix)
{
	d912pxy_com_object* ret = d912pxy_s.com.AllocateComObj(PXY_COM_OBJ_SURFACE_LAYER);
	
	new (&ret->layer)d912pxy_surface_layer(iBase, iSubres, iBSize, iWPitch, iWidth, imemPerPix);
	ret->vtable = d912pxy_com_route_get_vtable(PXY_COM_ROUTE_SURFACE_LAYER);

	return &ret->layer;
}

void d912pxy_surface_layer::DeAllocate(d912pxy_surface_layer * obj)
{
	obj->~d912pxy_surface_layer();
	d912pxy_s.com.DeAllocateComObj(PXY_COM_CAST_(d912pxy_com_object, obj));
}

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

d912pxy_comhandler::d912pxy_comhandler(d912pxy_com_obj_typeid tid, const wchar_t* moduleText) : d912pxy_noncom(moduleText)
{
	objType = tid;
	refc = 1;
	thrdRefc = 0;
	beingWatched = 0;
	poolSync.LockedSet(1);
	persistentlyPooled = 0;
	comBase = (d912pxy_com_object*)((intptr_t)this - 8);
}

d912pxy_comhandler::d912pxy_comhandler()
{
}

d912pxy_comhandler::~d912pxy_comhandler()
{	

}

void d912pxy_comhandler::Init(d912pxy_com_obj_typeid tid, const wchar_t * moduleText)
{
	persistentlyPooled = 0;
	objType = tid;
	refc = 1;
	thrdRefc = 0;
	beingWatched = 0;
	poolSync.Init();
	poolSync.LockedSet(1);
	NonCom_Init(moduleText);
	comBase = (d912pxy_com_object*)((intptr_t)this - 8);
}

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_COM

HRESULT d912pxy_comhandler::QueryInterface(REFIID riid, void ** ppvObj)
{
	LOG_DBG_DTDM("::CQI");
	*ppvObj = this;
	return NOERROR;
}

ULONG d912pxy_comhandler::AddRef()
{
	LOG_DBG_DTDM("::CAR");

	return InterlockedAdd(&refc, 1);
}

ULONG d912pxy_comhandler::Release()
{
	LONG decR = InterlockedAdd(&refc, -1);

	if (decR == 0)
	{
		LOG_DBG_DTDM("::CRE 0 %016llX", this);

		if (d912pxy_s.dev_vtable)
			d912pxy_s.dev.IFrameCleanupEnqeue(this);
		else {

			if (FinalReleaseCB())
				DeAllocateBase();
			
			return 0;
		}
	}

	

	return decR;
}

UINT d912pxy_comhandler::FinalReleaseTest()
{
	if (InterlockedAdd(&thrdRefc, 0))
	{
		d912pxy_s.dev.IFrameCleanupEnqeue(this);
		return 2;
	}
	else {
		if (FinalReleaseCB())
			return 3;
		return 1;
	}
}

UINT d912pxy_comhandler::FinalRelease()
{
	if (InterlockedAdd(&thrdRefc, 0))
	{
		d912pxy_s.dev.IFrameCleanupEnqeue(this);
		return 2;
	}
	else {		
		if (FinalReleaseCB())
		{
			DeAllocateBase();
		}
		return 1;
	}
}

UINT d912pxy_comhandler::FinalReleaseCB()
{
	return 1;
}

void d912pxy_comhandler::ThreadRef(INT ic)
{
	if (ic > 0)
		InterlockedAdd(&thrdRefc, 1);
	else
		InterlockedAdd(&thrdRefc, -1);
}

void d912pxy_comhandler::NoteDeletion(UINT32 time)
{
	poolSync.Hold();
	timestamp = time;
	poolSync.Release();
}

UINT d912pxy_comhandler::CheckExpired(UINT32 nt, UINT32 lifetime)
{
	return ((timestamp + lifetime) <= nt);
}

UINT32 d912pxy_comhandler::PooledAction(UINT32 use)
{
	UINT32 ret = 0;

	poolSync.Hold();

	LONG state = poolSync.GetValue();

	ret = state ^ use;

	if (ret)
	{
		if (!use)
		{
			if (timestamp > 0)
			{
				timestamp = 0;
				poolSync.SetValueAsync(0);
			}
			else
				ret = 0;
		} else 
			poolSync.SetValueAsync(1);
	}
	else if (use)
		timestamp = 0;

	if (!ret)
		poolSync.Release();

	return ret;
}

void d912pxy_comhandler::PooledActionExit()
{
	poolSync.Release();
}

int d912pxy_comhandler::Watching(LONG v)
{
	return InterlockedAdd(&beingWatched, v);
}

void d912pxy_comhandler::DeAllocateBase()
{
#define dtor_call(a) ((a*)(this))->~a()

	switch (objType)
	{
	case PXY_COM_OBJ_VSTREAM:
		dtor_call(d912pxy_vstream);
		d912pxy_s.com.DeAllocateComObj(comBase);
		break;
	case PXY_COM_OBJ_SURFACE:
		dtor_call(d912pxy_surface);
		d912pxy_s.com.DeAllocateComObj(comBase);
		break;
	case PXY_COM_OBJ_QUERY:
		dtor_call(d912pxy_query_non_derived);
		d912pxy_s.com.DeAllocateComObj(comBase);
		break;
	case PXY_COM_OBJ_QUERY_OCC:
		dtor_call(d912pxy_query_occlusion);
		d912pxy_s.com.DeAllocateComObj(comBase);
		break;
	case PXY_COM_OBJ_TEXTURE:
		(&comBase->basetex)->~d912pxy_basetexture();
		d912pxy_s.com.DeAllocateComObj(comBase);
		break;
	case PXY_COM_OBJ_TEXTURE_RTDS:
		(&comBase->basetex)->~d912pxy_basetexture();
		d912pxy_s.com.DeAllocateComObj(comBase);
		break;
	case PXY_COM_OBJ_VDECL:
		dtor_call(d912pxy_vdecl);
		d912pxy_s.com.DeAllocateComObj(comBase);
		break;
	case PXY_COM_OBJ_SHADER:
		dtor_call(d912pxy_shader);
		d912pxy_s.com.DeAllocateComObj(comBase);
		break;
	case PXY_COM_OBJ_SWAPCHAIN:
		dtor_call(d912pxy_swapchain);
		d912pxy_s.com.DeAllocateComObj(comBase);
		break;
	case PXY_COM_OBJ_SURFACE_LAYER:
		LOG_ERR_THROW2(-1, "surface_layer comhandler dtor");
		break;
	case PXY_COM_OBJ_SBLOCK:
		dtor_call(d912pxy_sblock);
		d912pxy_s.com.DeAllocateComObj(comBase);
		break;
	case PXY_COM_OBJ_PSO_ITEM:
		dtor_call(d912pxy_pso_cache_item);
		d912pxy_s.com.DeAllocateComObj(comBase);
		break;
	case PXY_COM_OBJ_NOVTABLE:
		delete this;
		break;
	case PXY_COM_OBJ_UNMANAGED:		
		PXY_FREE(comBase);
		break;
	case PXY_COM_OBJ_STATIC:
		;
		break;
	case PXY_COM_OBJ_RESOURCE:
		dtor_call(d912pxy_resource);		
		break;
	default:
		LOG_ERR_THROW2(-1, "wrong com object typeid");
		break;
	}
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE
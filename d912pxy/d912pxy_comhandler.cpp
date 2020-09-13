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
#include "stdafx.h"

d912pxy_comhandler::d912pxy_comhandler(d912pxy_com_obj_typeid tid, const wchar_t* moduleText)
	: d912pxy_noncom(moduleText)
	, refc(1)
	, timestamp(0)
	, thrdRefc(0)
	, thrdRefcFlag(0)
	, beingWatched(0)
	, persistentlyPooled(0)
	, objType(tid)
{
	poolSync.LockedSet(1);
	comBase = (d912pxy_com_object*)((intptr_t)this - sizeof(void*));
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
	comBase = (d912pxy_com_object*)((intptr_t)this - sizeof(void*));
}

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_COM

HRESULT d912pxy_comhandler::QueryInterface(REFIID riid, void ** ppvObj)
{
	LOG_DBG_DTDM("::CQI");
	*ppvObj = comBase;
	AddRef();
	return NOERROR;
}

ULONG d912pxy_comhandler::AddRef()
{
	LOG_DBG_DTDM("::CAR");

	return ++refc;
}

ULONG d912pxy_comhandler::Release()
{
	ULONG decR = --refc;

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
	if (thrdRefc)
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
	if (thrdRefc)
	{
		d912pxy_s.dev.IFrameCleanupEnqeue(this);
		return 3;
	}
	else {		

		if (thrdRefcFlag)
		{
			--thrdRefcFlag;
			d912pxy_s.dev.IFrameCleanupEnqeue(this);
			return 2;
		}

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
	//megai2: we must be sure that object referenced by some internal threads/logic are not deleted before gpu processed current submitted command list
	//example: interframe flush, currently used vs objects will be holded on, but can be deleted after flushed cl are executed before next cl that have reference to that objects are executed
	//			making GPU crash possible	
	if (!refc && !thrdRefc)
		//this will make object persist one cleanup cycle
		++thrdRefcFlag;

	if (ic > 0)
		++thrdRefc;
	else
		--thrdRefc;
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
	beingWatched += v;
	return beingWatched;
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
		dtor_call(d912pxy_pso_item);
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
		delete ((d912pxy_resource*)this);
		break;
	default:
		LOG_ERR_THROW2(-1, "wrong com object typeid");
		break;
	}
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE
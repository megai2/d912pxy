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

#define D912PXY_METHOD_IMPL_CN d912pxy_basetexture

d912pxy_basetexture::d912pxy_basetexture() : d912pxy_resource(RTID_TEX, PXY_COM_OBJ_TEXTURE, L"texture")
{
	ThreadRef(1);
}

d912pxy_basetexture::~d912pxy_basetexture()
{	
}

D912PXY_METHOD_IMPL_NC_(DWORD, GetPriority_SRVhack)(THIS)
{
	return baseSurface->GetSRVHeapId();
}

#undef D912PXY_METHOD_IMPL_CN

UINT d912pxy_basetexture::GetSRVHeapId()
{
	return GetSRVHeapId(attachedCache.shouldBarrier);
}

UINT d912pxy_basetexture::GetSRVHeapId(UINT genBarrier)
{
	return genBarrier ? baseSurface->GetSRVHeapIdRTDS() : attachedCache.srvId;
}

UINT d912pxy_basetexture::FinalRelease()
{
	UINT ret = d912pxy_comhandler::FinalRelease();
	if (ret == 3)
	{
		//megai2: keep threadRef if surface is still in use, otherwise free surface and remove threadRef
		if (baseSurface->GetCOMRefCount() == 1)
		{
			baseSurface->Release();
			ThreadRef(-1);
		}
	}

	return ret;
}

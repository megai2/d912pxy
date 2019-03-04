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

d912pxy_comhandler::d912pxy_comhandler(const wchar_t* moduleText) : d912pxy_noncom(NULL, moduleText)
{
	refc = 1;
	thrdRefc = 0;
	beingWatched = 0;
}

d912pxy_comhandler::d912pxy_comhandler(d912pxy_device * dev, const wchar_t * moduleText) : d912pxy_noncom(dev, moduleText)
{
	refc = 1;
	thrdRefc = 0;
}

d912pxy_comhandler::~d912pxy_comhandler()
{

}

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

		if (m_dev)
			m_dev->IFrameCleanupEnqeue(this);
		else
			delete this;
		return 0;
	}

	return decR;
}

UINT d912pxy_comhandler::FinalReleaseTest()
{
	if (thrdRefc)
	{
		m_dev->IFrameCleanupEnqeue(this);
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
		m_dev->IFrameCleanupEnqeue(this);
		return 2;
	}
	else {		
		if (FinalReleaseCB())
			delete this;
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
	timestamp = time;
}

UINT d912pxy_comhandler::CheckExpired(UINT32 nt)
{
	return ((timestamp + PXY_INNER_GC_LIFETIME) <= nt);	
}

UINT32 d912pxy_comhandler::PooledAction(UINT32 use)
{
	if (use)
	{
		if (timestamp == 1)
		{
			timestamp = 0;
			return 1;
		}
		else {
			timestamp = 0;
			return 0;
		}
	}
	else {
		if (timestamp > 1)
		{
			timestamp = 1;
			return 1;
		}
		else
			return 0;
	}
}

int d912pxy_comhandler::Watching(LONG v)
{
	return InterlockedAdd(&beingWatched, v);
}

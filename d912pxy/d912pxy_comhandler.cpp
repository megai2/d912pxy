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
	poolSync.LockedSet(1);
}

d912pxy_comhandler::d912pxy_comhandler(d912pxy_device * dev, const wchar_t * moduleText) : d912pxy_noncom(dev, moduleText)
{
	refc = 1;
	thrdRefc = 0;
	beingWatched = 0;
	poolSync.LockedSet(1);
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

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_COM

ULONG d912pxy_comhandler::Release()
{
	API_OVERHEAD_TRACK_START(1)

	LONG decR = InterlockedAdd(&refc, -1);

	if (decR == 0)
	{
		LOG_DBG_DTDM("::CRE 0 %016llX", this);

		if (m_dev)
			m_dev->IFrameCleanupEnqeue(this);
		else {

			API_OVERHEAD_TRACK_END(1)

			delete this;
			return 0;
		}
	}

	API_OVERHEAD_TRACK_END(1)

	return decR;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE

UINT d912pxy_comhandler::FinalReleaseTest()
{
	if (InterlockedAdd(&thrdRefc, 0))
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
	if (InterlockedAdd(&thrdRefc, 0))
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

	poolSync.Release();

	return ret;
}

int d912pxy_comhandler::Watching(LONG v)
{
	return InterlockedAdd(&beingWatched, v);
}

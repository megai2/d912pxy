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

d912pxy_replay_thread::d912pxy_replay_thread(d912pxy_device * dev, d912pxy_gpu_cmd_list_group iListGrp, char* threadName) : d912pxy_noncom(dev, L"replay thread"), d912pxy_thread(threadName)
{
	exchRI = new d912pxy_ringbuffer<UINT32>(4, 0);
	listGrp = iListGrp;
}

d912pxy_replay_thread::~d912pxy_replay_thread()
{
	delete exchRI;
}

void d912pxy_replay_thread::ThreadJob()
{
	d912pxy_replay* rpl = d912pxy_s(CMDReplay);

	d912pxy_s(iframe)->SetRSigOnList(listGrp);

	ID3D12GraphicsCommandList* cl = d912pxy_s(GPUcl)->GID(listGrp);

	if (drawStateSaved)
	{
		d912pxy_replay_item* rit = rpl->BacktraceItemType(DRPL_OMSR, 0);

		if (rit)
			rpl->PlayId(rit, cl);

		rit = rpl->BacktraceItemType(DRPL_OMBF, 0);

		if (rit)
			rpl->PlayId(rit, cl);

		d912pxy_s(iframe)->SetDrawStateOnCL(cl, drawStateSaved);
		drawStateSaved = NULL;
	}

	if (exchRI->HaveElements())
		rpl->Replay(exchRI->PopElement(), exchRI->PopElement(), cl, this);

	m_dev->LockThread(PXY_INNER_THREADID_RPL_THRD0 + (UINT)listGrp - CLG_RP1);

	IgnoreJob();
}

void d912pxy_replay_thread::ExecRange(UINT start, UINT end)
{
	exchRI->WriteElement(end);
	exchRI->WriteElement(start);	
}

void d912pxy_replay_thread::Finish()
{
	SignalWork();	
}

void d912pxy_replay_thread::ThreadInitProc()
{
	d912pxy_s(dev)->InitLockThread(PXY_INNER_THREADID_RPL_THRD0 + (UINT)listGrp - CLG_RP1);
}

void d912pxy_replay_thread::RecordIFrameDrawState()
{
	d912pxy_s(iframe)->GetCurrentDrawState(&drawState);
	drawStateSaved = &drawState;	
}

void d912pxy_replay_thread::DoAdditionalJob(UINT end)
{
	addRI = end;
}

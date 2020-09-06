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

d912pxy_buffer_loader::d912pxy_buffer_loader()
{
}

d912pxy_buffer_loader::~d912pxy_buffer_loader()
{	
	
}

void d912pxy_buffer_loader::Init()
{	
	d912pxy_async_upload_thread::Init(PXY_INNER_MAX_ASYNC_BUFFERLOADS, PXY_INNER_THREADID_BUF_LOADER, PXY_WAKE_FACTOR_BUFFER, L"buffer upload thread", "d912pxy bufld");

	d912pxy_s.dx12.que.EnableGID(CLG_BUF, PXY_INNER_CLG_PRIO_ASYNC_LOAD);
	Resume();
}

void d912pxy_buffer_loader::UploadItem(d912pxy_vstream_lock_data* it)
{
	it->dst->ProcessUpload(it, cl);
	it->dst->ThreadRef(-1);
}

void d912pxy_buffer_loader::ThreadWake()
{
	cl = d912pxy_s.dx12.cl->GID(CLG_BUF);
	PIXBeginEvent(cl, 0xAA, "vstream upload");
}

void d912pxy_buffer_loader::OnThreadInterrupt()
{
	while (finishList->HaveElements())
	{
		((d912pxy_vstream*)finishList->PopElement())->FinishUpload(cl);
	}

	PIXEndEvent(cl);
}

void d912pxy_buffer_loader::IssueUpload(d912pxy_vstream_lock_data lck)
{
	lck.dst->ThreadRef(1);	
	QueueItem(lck);
}
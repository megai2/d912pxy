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

d912pxy_buffer_loader::d912pxy_buffer_loader(d912pxy_device * dev) : d912pxy_async_upload_thread(dev, PXY_INNER_MAX_ASYNC_BUFFERLOADS, PXY_INNER_THREADID_BUF_LOADER, 10, L"buffer upload thread", "d912pxy bufld")
{
	d912pxy_s(bufloadThread) = this;
	d912pxy_s(GPUque)->EnableGID(CLG_BUF, PXY_INNER_CLG_PRIO_ASYNC_LOAD);

	finishList = new d912pxy_ringbuffer<d912pxy_vstream*>(64, 2);
}

d912pxy_buffer_loader::~d912pxy_buffer_loader()
{
	delete finishList;
}

void d912pxy_buffer_loader::UploadItem(d912pxy_vstream_lock_data* it)
{
	it->dst->ProcessUpload(it, cl);
	it->dst->ThreadRef(-1);
}

void d912pxy_buffer_loader::ThreadWake()
{
	cl = d912pxy_s(GPUcl)->GID(CLG_BUF);
}

void d912pxy_buffer_loader::AddToFinishList(d912pxy_vstream * it)
{
	finishList->WriteElement(it);
}

void d912pxy_buffer_loader::OnThreadInterrupt()
{
	while (finishList->HaveElements())
	{
		finishList->PopElement()->FinishUpload(cl);
	}
}

void d912pxy_buffer_loader::IssueUpload(d912pxy_vstream_lock_data lck)
{
	lck.dst->ThreadRef(1);	
	QueueItem(lck);
}
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

d912pxy_buffer_loader::d912pxy_buffer_loader(d912pxy_device * dev) : d912pxy_noncom(dev, L"buffer loader"), d912pxy_thread("d912pxy bufld")
{
	poolPtr = 0;
	needSignal = 0;

	buffer = new d912pxy_ringbuffer<d912pxy_vstream*>(PXY_INNER_MAX_ASYNC_BUFFERLOADS, 0);	

	d912pxy_s(bufloadThread) = this;
}

d912pxy_buffer_loader::~d912pxy_buffer_loader()
{
	Stop();

	delete buffer;
}

void d912pxy_buffer_loader::IssueUpload(d912pxy_vstream * dst)
{
	dst->ThreadRef(1);
	
	writeLock.Hold();
	buffer->WriteElement(dst);
	writeLock.Release();
	
	++needSignal;

	if ((needSignal % 10) == 0)
		SignalWork();
}

void d912pxy_buffer_loader::ThreadJob()
{
	ID3D12GraphicsCommandList* cl = d912pxy_s(GPUcl)->GID(CLG_BUF);	

	while (buffer->HaveElements())
	{
		d912pxy_vstream* it = buffer->GetElement();

		it->ProcessUpload(cl);	
		it->ThreadRef(-1);

		buffer->Next();
	}

	if (m_dev->InterruptThreads())
	{
		m_dev->LockThread(PXY_INNER_THREADID_BUF_LOADER);
	}
}

void d912pxy_buffer_loader::ThreadInitProc()
{
	d912pxy_s(dev)->InitLockThread(PXY_INNER_THREADID_BUF_LOADER);
}

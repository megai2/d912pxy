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

d912pxy_buffer_loader::d912pxy_buffer_loader(d912pxy_device * dev) : d912pxy_noncom(dev, L"buffer loader"), d912pxy_thread()
{
	poolPtr = 0;
	needSignal = 0;

	buffer = new d912pxy_ringbuffer<d912pxy_buffer_load_item*>(PXY_INNER_MAX_ASYNC_BUFFERLOADS, 0);
	swapBuffer = new d912pxy_ringbuffer<d912pxy_vstream*>(PXY_INNER_MAX_ASYNC_BUFFERLOADS, 0);

	dev->InitLockThread(PXY_INNER_THREADID_BUF_LOADER);
}

d912pxy_buffer_loader::~d912pxy_buffer_loader()
{
	Stop();

	delete swapBuffer;
	delete buffer;
}

void d912pxy_buffer_loader::IssueUpload(d912pxy_vstream * dst, d912pxy_upload_item * ul, UINT offset, UINT size)
{
	d912pxy_buffer_load_item* it = &pool[poolPtr];

	it->dst = dst;
	it->ul = ul;
	it->offset = offset;
	it->size = size;
	dst->ThreadRef(1);

	++poolPtr;
	if (poolPtr >= PXY_INNER_MAX_ASYNC_BUFFERLOADS)
		poolPtr = 0;

	//megai2: as ringbufer is not growing, we hit overrun and crash if we overwriting pool data
	buffer->WriteElement(it);

	++needSignal;

	if ((needSignal % 10) == 0)
		SignalWork();
}

void d912pxy_buffer_loader::ThreadJob()
{
	ID3D12GraphicsCommandList* cq = d912pxy_s(GPUcl)->GID(CLG_BUF).Get();	

	while (buffer->HaveElements())
	{
		d912pxy_buffer_load_item* it = buffer->GetElement();

		it->dst->AsyncUploadDataCopy(it->offset, it->size,cq);		
		//it->dst->AsyncBufferCopy(it->ul, it->offset, it->size, cq);
		
		
		if (it->dst->IsFirstFrameUploadRef() == 1)
		{
			swapBuffer->WriteElement(it->dst);			
		}

		buffer->Next();
	}

	if (m_dev->InterruptThreads())
	{
		while (swapBuffer->HaveElements())
		{
			swapBuffer->GetElement()->IFrameEndRefSwap();
			swapBuffer->Next();
		}

		m_dev->LockThread(PXY_INNER_THREADID_BUF_LOADER);
	}
}

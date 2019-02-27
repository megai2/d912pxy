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
#pragma once
#include "stdafx.h"

typedef struct d912pxy_buffer_load_item {
	d912pxy_vstream* dst;
	d912pxy_upload_item* ul;
	UINT offset;
	UINT size;
} d912pxy_buffer_load_item;


class d912pxy_buffer_loader : public d912pxy_noncom, public d912pxy_thread
{
public:
	d912pxy_buffer_loader(d912pxy_device* dev);
	~d912pxy_buffer_loader();

	void IssueUpload(d912pxy_vstream* dst, d912pxy_upload_item* ul, UINT offset, UINT size);

	void ThreadJob();
	void ThreadInitProc();

private:
	UINT isRunning;

	d912pxy_buffer_load_item pool[PXY_INNER_MAX_ASYNC_BUFFERLOADS];
	UINT poolPtr;
	UINT needSignal;

	d912pxy_ringbuffer<d912pxy_buffer_load_item*>* buffer;
	d912pxy_ringbuffer<d912pxy_vstream*>* swapBuffer;

	CRITICAL_SECTION writeLock;
};


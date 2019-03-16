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

d912pxy_texture_loader::d912pxy_texture_loader(d912pxy_device* dev) : d912pxy_async_upload_thread(dev, PXY_INNER_MAX_ASYNC_TEXLOADS, PXY_INNER_THREADID_TEX_LOADER, 3, L"texture upload thread", "d912pxy texld")
{
	d912pxy_s(texloadThread) = this;
	d912pxy_s(GPUque)->EnableGID(CLG_TEX, PXY_INNER_CLG_PRIO_ASYNC_LOAD);
}

d912pxy_texture_loader::~d912pxy_texture_loader()
{
}

void d912pxy_texture_loader::IssueUpload(d912pxy_surface * surf, void* mem, UINT subRes)
{
	d912pxy_texture_load_item it = { surf, mem, subRes };	
	surf->ThreadRef(1);
	
	QueueItem(it);
}

void d912pxy_texture_loader::ThreadWake()
{
}

void d912pxy_texture_loader::UploadItem(d912pxy_texture_load_item* it)
{
	it->surf->DelayedLoad(it->ul, it->subRes);
}
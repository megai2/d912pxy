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

d912pxy_texture_loader::d912pxy_texture_loader() : d912pxy_async_upload_thread()
{

}

d912pxy_texture_loader::~d912pxy_texture_loader()
{
	if (asyncLoadPendingItems)
	{
		if (asyncLoadPendingItems->HaveElements())
		{
			while (asyncLoadPendingItems->HaveElements())
			{
				asyncLoadPendingItems->PopElement()->ThreadRef(-1);
			}
			LOG_INFO_DTDM("pending items dereferenced");
		}

		delete asyncLoadPendingItems;
	}
}

void d912pxy_texture_loader::Init()
{
	d912pxy_async_upload_thread::Init(PXY_INNER_MAX_ASYNC_TEXLOADS, PXY_INNER_THREADID_TEX_LOADER, PXY_WAKE_FACTOR_TEXTURE, L"texture upload thread", "d912pxy texld");

	d912pxy_s.dx12.que.EnableGID(CLG_TEX, PXY_INNER_CLG_PRIO_ASYNC_LOAD);

	allowAsyncLoad = d912pxy_s.config.GetValueUI32(PXY_CFG_UPLOAD_TEX_ASYNC);

	if (allowAsyncLoad)
		asyncLoadPendingItems = new d912pxy_ringbuffer<d912pxy_surface*>(64, 2);
	else
		asyncLoadPendingItems = NULL;

	Resume();
}

void d912pxy_texture_loader::IssueUpload(d912pxy_surface * surf, void* mem, UINT subRes)
{
	d912pxy_texture_load_item it = { surf, mem, subRes };	
	surf->ThreadRef(1);
	
	QueueItem(it);
}

void d912pxy_texture_loader::ThreadWake()
{
	PIXBeginEvent(d912pxy_s.dx12.cl->GID(CLG_TEX), 0x0000AA, "surface upload");
}

void d912pxy_texture_loader::OnThreadInterrupt()
{
	if (allowAsyncLoad)
	{
		//megai2: delay finish pass 1 frame due to possible dereferencing while in GPU queue
		//then safely remove reference
		while (asyncLoadPendingItems->HaveElements())
		{
			asyncLoadPendingItems->PopElement()->ThreadRef(-1);
		}
		
		while (finishList->HaveElements())
		{
			d912pxy_surface* fItem = ((d912pxy_surface*)finishList->PopElement());

			fItem->ThreadRef(1);
			fItem->FinishUpload();

			asyncLoadPendingItems->WriteElement(fItem);
		}
	}
	else {
		while (finishList->HaveElements())
		{
			((d912pxy_surface*)finishList->PopElement())->FinishUpload();
		}
	}

	PIXEndEvent(d912pxy_s.dx12.cl->GID(CLG_TEX));
}

void d912pxy_texture_loader::UploadItem(d912pxy_texture_load_item* it)
{	
	it->surf->DelayedLoad(it->ul, it->subRes);

	if (allowAsyncLoad)
	{
		//megai2: allow only N items to be left on queue when we encounter thread interrupt
		//otherwise if machine can't handle per frame upload requests in reasonable time it will thrash memory
		if (ItemsOnQueue() < allowAsyncLoad)
			CheckInterrupt();
	}
}
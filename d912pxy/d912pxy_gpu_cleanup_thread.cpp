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
#include "d912pxy_gpu_cleanup_thread.h"

d912pxy_gpu_cleanup_thread::d912pxy_gpu_cleanup_thread() : d912pxy_thread()
{
	InitThread("d912pxy gpu cleanup", 0);
	finishedEvent = CreateEvent(0, 0, 1, 0);
	items = 0;
	cl = 0;
}

d912pxy_gpu_cleanup_thread::~d912pxy_gpu_cleanup_thread()
{
	items = 0;
	Stop();
	CloseHandle(finishedEvent);	
}

void d912pxy_gpu_cleanup_thread::ThreadJob()
{
	if (items)
		cl->CleanupReferenced(items);
	
//	d912pxy_s.dx12.que.ExecuteCommandsThreaded(1);

	SetEvent(finishedEvent);
}

void d912pxy_gpu_cleanup_thread::IssueItems(d912pxy_gpu_cmd_list * iCL, UINT cnt)
{	
	Finish();
	cl = iCL;
	items = cnt;	
}

void d912pxy_gpu_cleanup_thread::Finish()
{
	WaitForSingleObject(finishedEvent, INFINITE);	
}

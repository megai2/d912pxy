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

//start with 2^16 end with 2^28
//this will allow creating buffers with size up to 256MB
//megai2: 16 is for future placed resource heap aligment, if it work that whay
#define PXY_INNDER_VSTREAM_POOL_BITIGNORE 16 
#define PXY_INNDER_VSTREAM_POOL_BITLIMIT 28
#define PXY_INNDER_VSTREAM_POOL_BITCNT (PXY_INNDER_VSTREAM_POOL_BITLIMIT - PXY_INNDER_VSTREAM_POOL_BITIGNORE)

class d912pxy_vstream_pool : public d912pxy_pool_memcat<d912pxy_vstream*, d912pxy_vstream_pool*>
{
public:
	d912pxy_vstream_pool();
	~d912pxy_vstream_pool();

	void Init();
	void UnInit();

	d912pxy_vstream* GetVStreamObject(UINT size, UINT fmt, UINT isIB);

	d912pxy_vstream* AllocProc(UINT32 cat);

	ID3D12Resource* GetPlacedVStream(UINT32 size);

	void EarlyInitProc();
};


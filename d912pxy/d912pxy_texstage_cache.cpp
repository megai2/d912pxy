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

UINT16 texstage_memjmp_map[] = {
	0,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,10,3,0,2,0,2,0,2,0,2,3,3,3,3,3,3,3,3,7,3,3,3,3,3,4
};

d912pxy_texstage_cache::d912pxy_texstage_cache(d912pxy_device * dev) : d912pxy_noncom(dev, L"texture stage cache")
{
	d912pxy_s(textureState) = this;

	current = &currentBase;
	current->dirty = 0xFFFF;
}

d912pxy_texstage_cache::~d912pxy_texstage_cache()
{
}

void d912pxy_texstage_cache::SetTexStage(UINT stage, UINT srv)
{
	current->dirty |= 1 << (stage >> 2);
	current->texHeapID[stage] = srv;	
}

void d912pxy_texstage_cache::SetTexStageBit(UINT stage, UINT bit, UINT set)
{	
	UINT ov = current->texHeapID[stage];
	UINT val;

	if (set)
		val = (1 << bit) | ov;
	else
		val = (~(1 << bit)) & ov;

	current->dirty |= 1 << (stage >> 2);

	current->texHeapID[stage] = val;	
}

UINT d912pxy_texstage_cache::Use()
{	
	UINT32 dbits = d912pxy_s(samplerState)->IsDirty();
	UINT64 gbits = current->dirty + 0ULL;
	
	if (dbits)
	{	
		int i = 0;

		while (dbits)
		{
			if (dbits & 1)
			{				
				current->splHeapID[i] = d912pxy_s(samplerState)->GetDirtyDHeapId(i);
				gbits |= ((1 << (i >> 2)) << 8);
			}
			++i;
			dbits = dbits >> 1;			
		}		
	}
	
	int i = 0;

	while (gbits)
	{
		if (gbits & 1)
			d912pxy_s(batch)->GPUWrite((void*)((intptr_t)current + i*16 + 4), 1, i);
		++i;
		gbits = gbits >> 1;
	}

	current->dirty = 0;

	return 0;
}

void d912pxy_texstage_cache::Cleanup()
{

}

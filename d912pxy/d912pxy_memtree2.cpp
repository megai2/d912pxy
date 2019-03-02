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

d912pxy_memtree2::d912pxy_memtree2(UINT nMemSz, UINT iMaxNodes, UINT iGrow) : d912pxy_noncom(NULL, L"memtree2")
{
	maxNodes = iMaxNodes;
	grow = iGrow;
	selectedNode = 0;
	dataMemSz = nMemSz;
	nodePoolIdx = 1;

	UINT32 memSz = maxNodes * sizeof(d912pxy_memtree2_node);
	nodePool = (d912pxy_memtree2_node*)malloc(memSz);
	ZeroMemory(nodePool, memSz);
	ZeroMemory(&base, sizeof(d912pxy_memtree2_node));
}


d912pxy_memtree2::~d912pxy_memtree2()
{
#ifdef CAPTURE_JMP_MAP
	if (!jmpMap)
	{
		LOG_DBG_DTDM("memtree jmp map");
		wchar_t buf[4096];

		int jmpA = 0;

		buf[0] = 0;

		for (int i = 0; i != dataMemSz; ++i)
		{
			if (memDiffCnt[i] > 1)
			{
				wsprintf(buf, L"%s,%u", buf, jmpA);
				jmpA = 0;
			}
			else
				++jmpA;

			//LOG_DBG_DTDM("dt[%u] = %u", i, memDiffCnt[i]);
		}

		LOG_DBG_DTDM("jmp map = %s, + %u", buf, jmpA);
	}
#endif

	free(nodePool);
}

UINT64 d912pxy_memtree2::PointAt32(void * mem)
{
	UINT32 hv = memHash32(mem);

	return PointAtMem(&hv, 4);
}

UINT64 d912pxy_memtree2::PointAt64(void * mem)
{
	UINT64 hv = memHash64(mem);

	return PointAtMem(&hv, 8);
}

UINT32 d912pxy_memtree2::memHash32s(void * mem, UINT msz)
{
	UINT32 hash = 0x811c9dc5;
	UINT ctr = 0;

	while (ctr < msz)
	{
		UINT8 dataByte = ((UINT8*)mem)[ctr];

		hash = hash ^ dataByte;
		hash = hash * 16777619;
		++ctr;
	}
	return hash;
}

UINT64 d912pxy_memtree2::memHash64s(void * mem, UINT msz)
{
	UINT64 hash = 0xcbf29ce484222325;
	UINT ctr = 0;

	while (ctr != msz)
	{
		UINT8 dataByte = ((UINT8*)mem)[ctr];

		hash = hash ^ dataByte;
		hash = hash * 1099511628211;
		++ctr;
	}
	return hash;
}

UINT64 d912pxy_memtree2::PointAtMem(void * mem, UINT32 dataMemSz2)
{
	UINT32 depth = 0;
	d912pxy_memtree2_node* root = &base;

	UINT8* byteAc = (UINT8*)mem;

	while (depth < dataMemSz2)
	{

		UINT8 ci = byteAc[depth];
		
		UINT32 npi = root->childs[ci];
		if (npi == 0)
		{
			UINT32 newNode = InterlockedAdd((LONG*)&nodePoolIdx, 1)-1;
			root->childs[ci] = newNode;
			root = &nodePool[newNode];			
			++newNode;

			if (newNode >= maxNodes)
			{
				LOG_DBG_DTDM3("growing memtree %u => %u", maxNodes, maxNodes * grow);
				if (grow > 1)
				{
					maxNodes *= grow;
					UINT32 memSz = maxNodes * sizeof(d912pxy_memtree2_node);
					nodePool = (d912pxy_memtree2_node*)realloc(nodePool, memSz);	
					ZeroMemory(&nodePool[nodePoolIdx], memSz / grow);
				}
				else {
					LOG_ERR_THROW2(-1,"memtree full!");
					return -1;
				}

				root = &nodePool[nodePoolIdx-1];
			}
		}
		else {
			root = &nodePool[npi];
		}
		++depth;
	}
	selectedNode = root;
	return selectedNode->contentId;
}

void d912pxy_memtree2::SetValue(UINT64 val)
{
	selectedNode->contentId = val;
}

UINT64 d912pxy_memtree2::CurrentCID()
{
	return selectedNode->contentId;
}

void d912pxy_memtree2::Clear()
{
	UINT32 memSz = maxNodes * sizeof(d912pxy_memtree2_node);	
	ZeroMemory(nodePool, memSz);
	ZeroMemory(&base, sizeof(d912pxy_memtree2_node));
	nodePoolIdx = 1;
}

void d912pxy_memtree2::Begin()
{
	itrNPI = 1;
	selectedNode = &nodePool[itrNPI];
}

void d912pxy_memtree2::Next()
{
	++itrNPI;
	selectedNode = &nodePool[itrNPI];
}

UINT d912pxy_memtree2::IterEnd()
{
	return (itrNPI == nodePoolIdx);
}

UINT64 d912pxy_memtree2::memHash64(void* mem)
{
	UINT64 hash = 0xcbf29ce484222325;
	UINT ctr = 0;

	while (ctr!=dataMemSz)
	{
		UINT8 dataByte = ((UINT8*)mem)[ctr];

		hash = hash ^ dataByte;
		hash = hash * 1099511628211;		
		++ctr;
	}
	return hash;
}

UINT32 d912pxy_memtree2::memHash32(void* mem)
{
	UINT32 hash = 0x811c9dc5;
	UINT ctr = 0;
	UINT jmpMapId = 0;

	while (ctr < dataMemSz)
	{
		UINT8 dataByte = ((UINT8*)mem)[ctr];

		hash = hash ^ dataByte;
		hash = hash * 16777619;
		++ctr;
	}
	return hash;
}

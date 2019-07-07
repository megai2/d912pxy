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

d912pxy_memtree::d912pxy_memtree(UINT nMemSz, UINT iMaxNodes, UINT iGrow, const UINT16* iJmpMap) : d912pxy_noncom( L"memtree")
{
	dataMemSz = nMemSz;

	mtData.clear();
	dataMemSz = nMemSz;
	jmpMap = (UINT16*)iJmpMap;

#ifdef CAPTURE_JMP_MAP
	if (!iJmpMap)
	{
		memDiffData = (UINT8*)malloc(nMemSz);  // Alrai: Going to leave these mallocs for now.
		ZeroMemory(memDiffData, nMemSz);
		memDiffCnt = (UINT64*)malloc(nMemSz * 8);
		ZeroMemory(memDiffCnt, nMemSz * 8);
		jmpMap = 0;
	}
#endif
}


d912pxy_memtree::~d912pxy_memtree()
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
}

UINT64 d912pxy_memtree::PointAt(void * mem)
{
	pointKey = memHash32(mem);

	mtDataIter = mtData.find(pointKey);

	if (mtDataIter == mtData.end())
	{
		return 0;
	}
	else
		return mtDataIter->second;
}

UINT64 d912pxy_memtree::PointAtNH(UINT32 ha)
{
	pointKey = ha;

	mtDataIter = mtData.find(pointKey);

	if (mtDataIter == mtData.end())
	{
		return 0;
	}
	else
		return mtDataIter->second;
}

UINT64 d912pxy_memtree::PointAt2(void * mem)
{
	return 0;
}

void d912pxy_memtree::SetValue(UINT64 val)
{
	mtData[pointKey] = val;
}

UINT64 d912pxy_memtree::CurrentCID()
{
	if (mtDataIter != mtData.end())
		return mtDataIter->second;
	else
		return 0;
}

void d912pxy_memtree::Clear()
{
	mtData.clear();
}

void d912pxy_memtree::Begin()
{
	mtDataIter = mtData.begin();
	if (mtDataIter != mtData.end())
		pointKey = mtDataIter->first;
}

void d912pxy_memtree::Next()
{
	++mtDataIter;
	if (mtDataIter != mtData.end())
		pointKey = mtDataIter->first;
}

UINT d912pxy_memtree::IterEnd()
{
	return (mtDataIter == mtData.end());
}

UINT64 d912pxy_memtree::memHash(void* mem)
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

UINT32 d912pxy_memtree::memHash32(void* mem)
{
	UINT32 hash = 0x811c9dc5;
	UINT ctr = 0;
	UINT jmpMapId = 0;

	while (ctr < dataMemSz)
	{
		if (jmpMap)
		{
			ctr += jmpMap[jmpMapId];
			if (ctr > dataMemSz)
				break;
			++jmpMapId;
		}

		UINT8 dataByte = ((UINT8*)mem)[ctr];

		hash = hash ^ dataByte;
		hash = hash * 16777619;
		++ctr;
	}
	return hash;
}

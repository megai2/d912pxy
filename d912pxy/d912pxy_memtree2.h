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

//#define CAPTURE_JMP_MAP 1

typedef struct d912pxy_memtree2_node {
	UINT32 childs[256];
	UINT64 contentId;
} d912pxy_memtree2_node;

class d912pxy_memtree2 : public d912pxy_noncom
{
public:
	d912pxy_memtree2(UINT nMemSz, UINT iMaxNodes, UINT iGrow, const UINT16* iJmpMap);
	~d912pxy_memtree2();

	UINT64 PointAt2(void* mem);
	UINT64 PointAt(void* mem);
	UINT64 PointAtNH(void* mem);

	static UINT32 memHash32s(void* mem, UINT msz);
	UINT32 memHash32(void* mem);

	void SetValue(UINT64 val);
	
	UINT64 CurrentCID();
	
	void Clear();

	void Begin();
	void Next();
	UINT IterEnd();

	d912pxy_memtree2_node* AsyncIterBase(UINT32* mi) { *mi = nodePoolIdx; return nodePool; };

private:
	UINT64 memHash(void* mem);
	

	d912pxy_memtree2_node base;
	d912pxy_memtree2_node* selectedNode;	

	d912pxy_memtree2_node* nodePool;
	ULONG nodePoolIdx;
	UINT32 maxNodes;
	UINT32 itrNPI;

	UINT32 dataMemSz;

	UINT32 grow;

	UINT16* jmpMap;

	HANDLE sync;

#ifdef CAPTURE_JMP_MAP
	UINT8* memDiffData;
	UINT64* memDiffCnt;
#endif
};


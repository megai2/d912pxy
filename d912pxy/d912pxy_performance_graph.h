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

#define PXY_PERFGRPH_FRAMETIME_PTS 1024
#define PXY_PERFGRPH_FRAMETIME_MAX 112400
#define PXY_PERFGRPH_FRAMETIME_MIN  10000
#define PXY_PERFGRPH_BATCH_PTS 1024
#define PXY_PERFGRPH_BATCH_DIV 8

class d912pxy_performance_graph
{
public:
	d912pxy_performance_graph(UINT isDX9);
	~d912pxy_performance_graph();
	void RecordPresent(int batchCount);
private:
	Stopwatch frameTime;

	UINT dx9;

	UINT32 dataAcm[PXY_PERFGRPH_BATCH_PTS* PXY_PERFGRPH_FRAMETIME_PTS];
};


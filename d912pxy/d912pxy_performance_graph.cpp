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

d912pxy_performance_graph::d912pxy_performance_graph(UINT isDX9)
{
	dx9 = isDX9;
	frameTime.Reset();
	ZeroMemory(dataAcm, PXY_PERFGRPH_BATCH_PTS * PXY_PERFGRPH_FRAMETIME_PTS * 4);	
}


d912pxy_performance_graph::~d912pxy_performance_graph()
{
	UINT8* imgData = (UINT8*)malloc(PXY_PERFGRPH_BATCH_PTS * PXY_PERFGRPH_FRAMETIME_PTS * 3);

	for (int j = 0; j != PXY_PERFGRPH_BATCH_PTS; ++j)
	{	
		UINT32 maxValue2 = 1;		
		for (int k = 0; k != PXY_PERFGRPH_FRAMETIME_PTS; ++k)
		{
			UINT32 tmp = dataAcm[k*PXY_PERFGRPH_BATCH_PTS + j];
			if (tmp > maxValue2)
				maxValue2 = tmp;
		}

		for (int k = 0; k != PXY_PERFGRPH_FRAMETIME_PTS; ++k)
		{
			int i = j + k * PXY_PERFGRPH_BATCH_PTS;

			double pv = (double)dataAcm[i] / ((double)maxValue2);
			pv *= 256;

			UINT32 pvi = (UINT32)pv;

			if (pvi > 1)
				pvi -= 1;

			if (pvi > 256 * 0)
			{
				imgData[i * 3 + 0] = pvi;
				imgData[i * 3 + 1] = pvi;
				imgData[i * 3 + 2] = pvi;
			} else {
				imgData[i * 3 + 0] = 0;
				imgData[i * 3 + 1] = 0;
				imgData[i * 3 + 2] = 0;
			}

		}
	}

	if (dx9)
		stbi_write_png("performance_graph_dx9.png", PXY_PERFGRPH_BATCH_PTS, PXY_PERFGRPH_FRAMETIME_PTS, 3, imgData, 0);
	else
		stbi_write_png("performance_graph.png", PXY_PERFGRPH_BATCH_PTS, PXY_PERFGRPH_FRAMETIME_PTS, 3, imgData, 0);
}


void d912pxy_performance_graph::RecordPresent(int batchCount)
{
	UINT64 r = frameTime.Elapsed();

	if (r > 0)
	{
		if (r < PXY_PERFGRPH_FRAMETIME_MIN)
			r = 0;
		else
			r = r - PXY_PERFGRPH_FRAMETIME_MIN;

		r = r / ((PXY_PERFGRPH_FRAMETIME_MAX - PXY_PERFGRPH_FRAMETIME_MIN) / PXY_PERFGRPH_FRAMETIME_PTS);

		if (r >= PXY_PERFGRPH_FRAMETIME_PTS)
			r = PXY_PERFGRPH_FRAMETIME_PTS - 1;
	}

	batchCount = batchCount / PXY_PERFGRPH_BATCH_DIV;

	if (batchCount >= PXY_PERFGRPH_BATCH_PTS)
		batchCount = PXY_PERFGRPH_BATCH_PTS - 1;

	++dataAcm[r * PXY_PERFGRPH_BATCH_PTS + batchCount];

	frameTime.Reset();
}

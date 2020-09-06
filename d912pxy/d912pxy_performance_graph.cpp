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

d912pxy_performance_graph::d912pxy_performance_graph(UINT isDX9) : dx9(isDX9)
{
	frameTime.Reset();
	ZeroMemory(dataAcm, PXY_PERFGRPH_BATCH_PTS * PXY_PERFGRPH_FRAMETIME_PTS * 4);	
}


d912pxy_performance_graph::~d912pxy_performance_graph()
{
	DumpData();
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

void d912pxy_performance_graph::DumpData()
{
	FILE* of = NULL;

	if (dx9)
	{		
		of = fopen(d912pxy_helper::GetFilePath(FP_PERF_GRAPH_DX9_OUTFILE)->s, "wb");
	}
	else
		of = fopen(d912pxy_helper::GetFilePath(FP_PERF_GRAPH_OUTFILE)->s, "wb");

	UINT8* imgData = NULL;
	PXY_MALLOC(imgData, PXY_PERFGRPH_BATCH_PTS * PXY_PERFGRPH_FRAMETIME_PTS * 4, UINT8*);

	{
		static const char* header[] = {
			"<html>\n",
			"	<head><title>d912pxy performance graph</title>\n",
			"	</head>\n",
			"   <body>\n<div style=\"width:100 % ; font - size: 20px; padding : 0; margin : 0; \">\n"
		};

		for (int i = 0; i != 4; ++i)
		{
			fwrite(header[i], 1, strlen(header[i]), of);
		}
	}

	for (UINT32 j = 0; j != PXY_PERFGRPH_BATCH_PTS; ++j)
	{
		UINT32 maxValue2 = 1;
		for (int k = 0; k != PXY_PERFGRPH_FRAMETIME_PTS; ++k)
		{
			UINT32 tmp = dataAcm[k*PXY_PERFGRPH_BATCH_PTS + j];
			if (tmp > maxValue2)
				maxValue2 = tmp;
		}

		for (UINT32 k = 0; k != PXY_PERFGRPH_FRAMETIME_PTS; ++k)
		{
			int i = j + k * PXY_PERFGRPH_BATCH_PTS;

			double pv = (double)dataAcm[i] / ((double)maxValue2);
			pv *= 255;

			UINT32 pvi = (UINT32)pv;

			if (pvi > 1)
				pvi -= 1;

			if (pvi > 256 * 0)
			{
				imgData[i * 4 + 0] = pvi *  dx9;
				imgData[i * 4 + 1] = pvi * !dx9;
				imgData[i * 4 + 2] = 0;
				imgData[i * 4 + 3] = 128;
			}
			else {
				imgData[i * 4 + 0] = 0;
				imgData[i * 4 + 1] = 0;
				imgData[i * 4 + 2] = 0;
				imgData[i * 4 + 3] = 0;
			}
		}
	}

	{
		static const char* footer[] = {
			"     </div>\n",
			"	  <div style=\"width:100 % ; padding : 0; margin : 0; position:relative\">\n",
			"		<img src=\"dx12_perf_graph.png\" style=\"position: absolute; top: 0; left: 0; z-index: 1; border: 1px solid\" / >\n",
			"		<img src=\"dx9_perf_graph.png\" style=\"position: absolute; top: 0; left: 0; z-index: 1; border: 1px solid\" / >\n",
			"     </div>\n",
			"	</body>\n",
			"</html>\n"
		};



		char* cpub = d912pxy_helper::GetCPUBrandString();
		const char* gpun = "DX9";
		const char* br = "<br>\n";

		if (!dx9)
			gpun = d912pxy_s.dev.GetCurrentGPUName();

		fwrite(cpub, 1, strlen(cpub), of);		
		fwrite(br, 1, strlen(br), of);
		fwrite(gpun, 1, strlen(gpun), of);		
		fwrite(br, 1, strlen(br), of);
		fwrite(BUILD_VERSION_NAME, 1, strlen(BUILD_VERSION_NAME), of);			
		
		for (int i = 0; i != 7; ++i)
		{
			fwrite(footer[i], 1, strlen(footer[i]), of);
		}
	}

	fflush(of);
	fclose(of);

	if (dx9)
		stbi_write_png(d912pxy_helper::GetFilePath(FP_PERF_GRAPH_DX9_OUTFILE_PNG)->s, PXY_PERFGRPH_BATCH_PTS, PXY_PERFGRPH_FRAMETIME_PTS, 4, imgData, PXY_PERFGRPH_BATCH_PTS * 4);
	else
		stbi_write_png(d912pxy_helper::GetFilePath(FP_PERF_GRAPH_OUTFILE_PNG)->s, PXY_PERFGRPH_BATCH_PTS, PXY_PERFGRPH_FRAMETIME_PTS, 4, imgData, PXY_PERFGRPH_BATCH_PTS * 4);

	PXY_FREE(imgData);
}

void d912pxy_performance_graph::ResetTime()
{
	frameTime.Reset();
}

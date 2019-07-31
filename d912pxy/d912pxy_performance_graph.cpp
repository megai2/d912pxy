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

#include "../thirdparty/font.c"

d912pxy_performance_graph::d912pxy_performance_graph(UINT isDX9)
{
	dx9 = isDX9;
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

	char* stringAcm[4] = { 0 };
	size_t acmEndpoint[4];

	if (dx9)
	{
		of = fopen(d912pxy_perf_graph_dx9_outfile, "wb");
	}
	else
		of = fopen(d912pxy_perf_graph_outfile, "wb");

	{
		static const char* header[] = {
			"<html>\n",
			"	<head>\n",
			"		<!--Plotly.js-->\n",
			"		<script src = \"https://cdn.plot.ly/plotly-latest.min.js\"></script>\n",
			"	</head>\n",
			"   <body>\n",
			"		<div id = \"myDiv\" style=\"width: 100%; height: 100% \"><!--Plotly chart will be drawn inside this DIV--></div>\n",
			"		<script>\n"
		};

		for (int i = 0; i != 8; ++i)
		{
			fwrite(header[i], 1, strlen(header[i]), of);
		}

		for (int i = 0; i != 4; ++i)
		{
			PXY_MALLOC(stringAcm[i], 1024 * 1024 * 20, char*);
			stringAcm[i][0] = 0;
			acmEndpoint[i] = 0;
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

			{
				if (pv <= 0)
					continue;

				static char ysBuf[255];
				static char xsBuf[255];
				static char pvBuf[255];

				UINT64 ftime = (PXY_PERFGRPH_FRAMETIME_MIN + k * ((PXY_PERFGRPH_FRAMETIME_MAX - PXY_PERFGRPH_FRAMETIME_MIN) / PXY_PERFGRPH_FRAMETIME_PTS));

				sprintf(ysBuf, ", %f", 1000000.0f / ftime);
				sprintf(xsBuf, ", %u", j * PXY_PERFGRPH_BATCH_DIV);
				sprintf(pvBuf, ", %f", pv * 255);

				strcat(stringAcm[0], ysBuf);
				strcat(stringAcm[1], xsBuf);
				strcat(stringAcm[2], pvBuf);

				acmEndpoint[0] += strlen(ysBuf);
				acmEndpoint[1] += strlen(xsBuf);
				acmEndpoint[2] += strlen(pvBuf);
			}
		}
	}

	{
		static const char* emts[] = {
			"			var trace1 = {\n",
			"				x: [0",
			"				y: [0",
			"				mode: 'markers',\n",
			"				marker: {\n",
			"					size: 5,\n",
			"					color: [0",
			"				}\n",
			"			};\n\n",
			"			var data = [trace1];\n\n",
			"			var layout = {\n",
			"				title: 'Performance statistics<br><b>CPU</b>: ",
			"; <b>GPU</b>: ",
			"; ",
			"',\n"
		};

		static const char* footer[] = {
			"				xaxis: {\n",
			"					title: {\n",
			"						text: \"Draw calls per frame\"\n",
			"					}\n",
			"				},\n",
			"				yaxis : {\n",
			"					title: {\n",
			"						text: \"FPS\"\n",
			"					}\n",
			"				}\n",
			"			};\n\n",
			"			Plotly.newPlot('myDiv', data, layout);\n",
			"		</script>\n",
			"	</body>\n",
			"</html>\n",
		};

		strcat(stringAcm[0], "],\n");
		strcat(stringAcm[1], "],\n");
		strcat(stringAcm[2], "]\n");

		fwrite(emts[0], 1, strlen(emts[0]), of);
		fwrite(emts[1], 1, strlen(emts[1]), of);
		fwrite(stringAcm[1], 1, acmEndpoint[1] + 3, of);
		fwrite(emts[2], 1, strlen(emts[2]), of);
		fwrite(stringAcm[0], 1, acmEndpoint[0] + 3, of);
		fwrite(emts[3], 1, strlen(emts[3]), of);
		fwrite(emts[4], 1, strlen(emts[4]), of);
		fwrite(emts[5], 1, strlen(emts[5]), of);
		fwrite(emts[6], 1, strlen(emts[6]), of);
		fwrite(stringAcm[2], 1, acmEndpoint[2] + 2, of);
		fwrite(emts[7], 1, strlen(emts[7]), of);
		fwrite(emts[8], 1, strlen(emts[8]), of);
		fwrite(emts[9], 1, strlen(emts[9]), of);
		fwrite(emts[10], 1, strlen(emts[10]), of);
		fwrite(emts[11], 1, strlen(emts[11]), of);

		char* cpub = d912pxy_helper::GetCPUBrandString();
		const char* gpun = "DX9";

		if (!dx9)
			gpun = d912pxy_s.dev.GetCurrentGPUName();

		fwrite(cpub, 1, strlen(cpub), of);
		fwrite(emts[12], 1, strlen(emts[12]), of);
		fwrite(gpun, 1, strlen(gpun), of);
		fwrite(emts[13], 1, strlen(emts[13]), of);
		fwrite(BUILD_VERSION_NAME, 1, strlen(BUILD_VERSION_NAME), of);
		fwrite(emts[14], 1, strlen(emts[14]), of);

		for (int i = 0; i != 15; ++i)
		{
			fwrite(footer[i], 1, strlen(footer[i]), of);
		}
	}

	fflush(of);
	fclose(of);

	for (int i = 0; i != 4; ++i)
		PXY_FREE(stringAcm[i]);
}

void d912pxy_performance_graph::ResetTime()
{
	frameTime.Reset();
}

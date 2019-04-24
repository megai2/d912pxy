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
	imgData = (UINT8*)malloc(PXY_PERFGRPH_BATCH_PTS * PXY_PERFGRPH_FRAMETIME_PTS * 3);

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

	DrawOverlay();

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

void d912pxy_performance_graph::DrawOverlay()
{
	//DrawLineH(1023);
	DrawLineH(768);
	DrawLineH(512);	
	DrawLineH(256);
	DrawLineH(128);
	DrawLineH(64);
	//DrawLineH(0);

	DrawLineV(256);	
	DrawLineV(512);
	DrawLineV(768);

	UINT texPosX[] = {
		1023,
		1023,
		1023,
		1023,
		1023,
		1023,
		1023,
		50,
		255,
		511,
		767
	};

	UINT texPosY[] = {
		1015,
		769,
		513,
		257,
		129,
		65,
		1,
		1015,
		1015,
		1015,
		1015
	};

	static const char* Hleg[] = {		
		" 8.9 FPS / 8192 DIP",
		"11.5 FPS",
		"16.3 FPS",
		"28.1 FPS",
		"43.9 FPS",
		"  61 FPS",
		" 100 FPS",
		"0 DIP",
		"2048 DIP",
		"4096 DIP",
		"6144 DIP"
	};

	for (int i = 0; i != 11; ++i)
	{
		DrawText(texPosX[i], texPosY[i], 1, (char*)Hleg[i]);
	}	

	DrawText(900, 10, 1, d912pxy_helper::GetCPUBrandString());

	if (!dx9)
		DrawText(900, 20, 1, d912pxy_s(dev)->GetCurrentGPUName());
	else 
		DrawText(900, 20, 1, (char*)"DX9");
}

void d912pxy_performance_graph::DrawLineH(UINT y)
{
	int mwpt = PXY_PERFGRPH_BATCH_PTS * y;
	for (int i = 0; i != PXY_PERFGRPH_BATCH_PTS; ++i)
	{		
		imgData[mwpt * 3 + 0] = 0xFF;
		imgData[mwpt * 3 + 1] = 0;
		imgData[mwpt * 3 + 2] = 0;
		++mwpt;
	}
}

void d912pxy_performance_graph::DrawLineV(UINT x)
{
	int mwpt = x;
	for (int i = 0; i != PXY_PERFGRPH_FRAMETIME_PTS; ++i)
	{
		imgData[mwpt * 3 + 0] = 0xFF;
		imgData[mwpt * 3 + 1] = 0;
		imgData[mwpt * 3 + 2] = 0;
		mwpt += PXY_PERFGRPH_BATCH_PTS;
	}
}

void d912pxy_performance_graph::DrawCharacter(UINT lx, UINT ty, UINT scale, CHAR v)
{
	if (v >= 128)
		v = 0;

	UINT64 bits = font[v];

	for (int j = 7; j >= 0; --j)
		for (int i = 7; i>=0; --i)		
		{
			UINT bw = bits & 1;
			bits = bits >> 1;

			if (bw)
				for (int k = 0; k != scale; ++k)
					for (int l = 0; l != scale; ++l)
					{
						int mwpt = (lx + i*scale + k) + (ty + j*scale + l) * PXY_PERFGRPH_BATCH_PTS;
			
						imgData[mwpt * 3 + 0] = 0xFF;
						imgData[mwpt * 3 + 1] = 0xAA;
						imgData[mwpt * 3 + 2] = 0x10;
					}
		}
}

void d912pxy_performance_graph::DrawText(UINT rx, UINT ty, UINT scale, char * txt)
{
	UINT strlen = lstrlenA(txt);

	UINT lx = rx - strlen * scale * 8;

	for (int i = 0; i != strlen; ++i)
	{
		DrawCharacter(lx, ty, scale, txt[i]);
		lx += scale * 8;
	}
}

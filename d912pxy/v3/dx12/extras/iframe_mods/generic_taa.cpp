/*
MIT License

Copyright(c) 2020 megai2

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

using namespace d912pxy::extras::IFrameMods;

GenericTAA::GenericTAA(const wchar_t* preUiLastDraw, const wchar_t* uiFirstDraw)
{
	uiPass = new PassDetector(preUiLastDraw, uiFirstDraw, 0, false);
	d912pxy_s.iframeMods.pushMod(uiPass);
	//TODO: make dx12 shader class
	//compile & load TAA shader
}

void d912pxy::extras::IFrameMods::GenericTAA::UnInit()
{
	delete taaShader;

	if (prevFrame)
		delete prevFrame;

	if (currentFrame)
		delete currentFrame;
}

void GenericTAA::RP_PreDraw(d912pxy_replay_item::dt_draw_indexed* rpItem, d912pxy_replay_thread_context* rpContext)
{
	if (uiPass->entered())
	{
		if (!prevFrame)
		{
			//create new RTs from uiPass.getSurf(true, true) params;			
		}
				
		//copy currentRt to currentFrame
		//transit prevFrame,currentFrame to SRV		
		//bind to shader as SRV
		//draw with TAA shader
		//return to normal rendering
	}
}

void GenericTAA::IFR_Start()
{
	//advance jitter idx
	jitterIdx = (jitterIdx + 1) % TAA_jitterSequenceLength;

	//update taa params in shader
	float taaParams[4] = { TAA_jitterSequence[jitterIdx * 2 + 0], TAA_jitterSequence[jitterIdx * 2 + 1], 0.0f, 0.0f };
	if (prevFrame)
	{
		taaParams[2] = 1.0f / prevFrame->GetDX9DescAtLevel(0).Width;
		taaParams[3] = 1.0f / prevFrame->GetDX9DescAtLevel(0).Height;
	}
	d912pxy_s.render.batch.SetShaderConstF(1, PXY_INNER_EXTRA_SHADER_CONST_TAA_PARAMS, 1, taaParams);
}
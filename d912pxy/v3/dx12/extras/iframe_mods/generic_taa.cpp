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

d912pxy_surface* GenericTAA::surfFromTempl(D3DSURFACE_DESC& descTempl)
{
	UINT levels = 1;
	d912pxy_surface* ret = d912pxy_surface::d912pxy_surface_com(
		descTempl.Width,
		descTempl.Height,
		descTempl.Format,
		descTempl.Usage,
		descTempl.MultiSampleType,
		descTempl.MultiSampleQuality,
		false,
		&levels,
		0,
		nullptr
	);

	//TODO: ensure we have rtds at first TAA draw call
	ret->GetSRVHeapIdRTDS();

	return ret;
}

void GenericTAA::resetAdditionalFrames(d912pxy_surface* from)
{
	D3DSURFACE_DESC descTempl = from->GetDX9DescAtLevel(0);

	if (prevFrame)
		prevFrame->Release();

	if (currentFrame)
		currentFrame->Release();
	
	prevFrame = surfFromTempl(descTempl);
	currentFrame = surfFromTempl(descTempl);

	jitterSeq.adjustInverseWH(descTempl.Width, descTempl.Height);
	jitterSeq.loadTo(jitterCBuf);
}

void GenericTAA::checkNativeDrawLoaded()
{
	if (taaDraw)
		return;

	jitterCBuf = d912pxy_vstream::d912pxy_vstream_com(jitterSeq.size, 0, 0, 0);

	//TODO: make dx12 shader & load it
	uint32_t indexData[6] = { 0, 1, 2, 2, 3, 0 };
	float vertexData[16] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		-1.0f,  1.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f, 0.0f
	};

	float cb0Data[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	taaDraw = new NativeDraw(nullptr,
		{
			MemoryArea(&indexData, sizeof(indexData)),
			MemoryArea(&vertexData, sizeof(vertexData)),
			MemoryArea(&cb0Data, sizeof(cb0Data))
		}
	);
}

GenericTAA::GenericTAA(const wchar_t* preUiLastDraw, const wchar_t* uiFirstDraw, int cbufferReg)
	: jitterCBufRSIdx(cbufferReg)
{
	uiPass = new PassDetector(preUiLastDraw, uiFirstDraw, 0, false);
	d912pxy_s.iframeMods.pushMod(uiPass);
}

void GenericTAA::setJitter(bool enable, d912pxy_replay_thread_context* ctx)
{
	ctx->cl->SetGraphicsRootConstantBufferView(
		jitterCBufRSIdx,
		jitterCBuf->GetVA_GPU() + jitterSeq.cbOffset(enable ? jitterIdx : -1)
	);
}

void d912pxy::extras::IFrameMods::GenericTAA::UnInit()
{
	delete taaShader;

	if (prevFrame)
		prevFrame->Release();

	if (currentFrame)
		currentFrame->Release();

	jitterCBuf->Release();
}

void GenericTAA::RP_PreDraw(d912pxy_replay_item::dt_draw_indexed* rpItem, d912pxy_replay_thread_context* rpContext)
{
	if (uiPass->entered())
	{
		d912pxy_surface* currentRt = uiPass->getSurf(true);

		if (!prevFrame || !currentFrame)
		{
			resetAdditionalFrames(currentRt);
		}
		else {
			D3DSURFACE_DESC actualDesc = currentRt->GetDX9DescAtLevel(0);
			D3DSURFACE_DESC oldDesc = prevFrame->GetDX9DescAtLevel(0);
			
			if ((oldDesc.Height != actualDesc.Height) || (oldDesc.Width != actualDesc.Width))
			{
				resetAdditionalFrames(currentRt);
			}
		}

		currentRt->BCopyToWStates(currentFrame, 3, rpContext->cl, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
			
		checkNativeDrawLoaded();
		//taaDraw->draw(*rpContext);

		currentFrame->BCopyToWStates(prevFrame, 3, rpContext->cl, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		
	}
}

void GenericTAA::IFR_Start()
{
	//advance jitter idx
	jitterSeq.advanceIdx(jitterIdx);
}
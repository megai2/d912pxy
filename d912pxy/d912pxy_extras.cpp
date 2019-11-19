/*
MIT License

Copyright(c) 2019 megai2

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

d912pxy_extras::d912pxy_extras()
{

}

d912pxy_extras::~d912pxy_extras()
{

}

void d912pxy_extras::Init()
{
	NonCom_Init(L"extras");

	targetFrameTime = d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_FPS_LIMIT);
	if (targetFrameTime)
		targetFrameTime = 1000 / targetFrameTime;

	waitEvent = CreateEvent(0, 0, 0, 0);

	frameTime.Reset();
	frTimeMs = 1;
	targetFrameTimeDelay = 0;

	//load config
	
	bShowFps = d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_SHOW_FPS);
	bShowDrawCount = d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_SHOW_DRAW_COUNT);
	bShowFpsGraph = d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_SHOW_FPS_GRAPH);

	if (bShowFpsGraph)
		fpsGraphData = new d912pxy_ringbuffer<float>(PXY_INNER_EXTRA_FPS_GRAPH_PTS, 0);

	bShowTimings = d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_SHOW_TIMINGS);
	bShowPSOCompileQue = d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_SHOW_PSO_COMPILE_QUE);
	bShowGCQue = d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_SHOW_GC_QUE);

	//imgui setup

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	
	ImGui::StyleColorsDark();

	d912pxy_swapchain* swpc = d912pxy_s.dev.GetPrimarySwapChain();

	d912pxy_dheap* srvHeap = d912pxy_s.dev.GetDHeap(PXY_INNER_HEAP_SRV);
	UINT32 srvHeapSlot = srvHeap->OccupySlot();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

	ImGui_ImplWin32_Init(swpc->GetTargetWindow());
	ImGui_ImplDX12_Init(
		d912pxy_s.dx12.dev,
		2,
		swpc->GetRenderBuffer()->GetSRVFormat(),
		srvHeap->GetHeapObj(),
		srvHeap->GetDHeapHandle(srvHeapSlot),
		srvHeap->GetGPUDHeapHandle(srvHeapSlot)
	);


}

void d912pxy_extras::UnInit()
{
	if (bShowFpsGraph)
		delete fpsGraphData;

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	d912pxy_noncom::UnInit();
}

static float fps_graph_buffer_transform(void* data, int idx)
{
	d912pxy_ringbuffer<float>* rb = (d912pxy_ringbuffer<float>*)data;

	return rb->GetElementOffset(idx);
}

void d912pxy_extras::OnPresent()
{
	ImGUI_Render_Start();
	
	ImGui::Begin("d912pxy overlay");                          // Create a window called "Hello, world!" and append into it.	
	
	if (bShowFps)
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	if (bShowTimings)
	{
		ImGui::Text("%4.2f ms/frame overhead", syncNexecTime.GetStopTime() / 1000.0f);
	}

	if (bShowDrawCount)
	{
		ImGui::Text("%6u DIPs", d912pxy_s.render.batch.GetBatchCount());
		ImGui::Text("%6u Batches", d912pxy_s.render.batch.GetBatchNum());
	}

	if (bShowGCQue)
		ImGui::Text("%6u GC", d912pxy_s.thread.cleanup.TotalWatchedItems());

	if (bShowPSOCompileQue)
		ImGui::Text("%6u PSO", d912pxy_s.render.db.pso.GetCompileQueueLength());

	if (bShowFpsGraph)
	{
		if (!fpsGraphData->HaveFreeSpace())
			fpsGraphData->Next();

		fpsGraphData->WriteElement(1000.0f / frTimeMs);

		ImGui::PlotLines("FPS", &fps_graph_buffer_transform, fpsGraphData, PXY_INNER_EXTRA_FPS_GRAPH_PTS, 0, 0, 0, 80, ImVec2(512, 256));
	}

	ImGui::End();

	ImGUI_Render_End();

	syncNexecTime.Reset();
}

void d912pxy_extras::OnReset()
{

}

void d912pxy_extras::WaitForTargetFrameTime()
{
	frTimeMs = frameTime.Elapsed() / 1000;
	frameTime.Reset();

	syncNexecTime.Stop();

	if (!targetFrameTime)
		return;

	//megai2: do it better	
	INT64 targetDelta = llabs(frTimeMs - targetFrameTime);
	
	if (frTimeMs < targetFrameTime)
	{
		if (targetDelta > 10)
			targetFrameTimeDelay += targetDelta;
		else
			targetFrameTimeDelay += 1;		
	}
	else {
		if (targetFrameTimeDelay > 1)
		{
			if ((targetDelta > 10) && (targetDelta < targetFrameTimeDelay))
				targetFrameTimeDelay -= targetDelta;
			else
				targetFrameTimeDelay -= 1;
		}
	}

	WaitForSingleObject(waitEvent, targetFrameTimeDelay);
}

void d912pxy_extras::ImGUI_Render_Start()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void d912pxy_extras::ImGUI_Render_End()
{
	ID3D12GraphicsCommandList* cl = d912pxy_s.dx12.cl->GID(CLG_SEQ);
	ID3D12DescriptorHeap* rtvHeap = d912pxy_s.dev.GetDHeap(PXY_INNER_HEAP_SRV)->GetHeapObj();

	D3D12_CPU_DESCRIPTOR_HANDLE rtvDsc = d912pxy_s.dev.GetPrimarySwapChain()->GetRenderBuffer()->GetDHeapHandle();
	cl->OMSetRenderTargets(1, &rtvDsc, 0, 0);
	cl->SetDescriptorHeaps(1, &rtvHeap);

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cl);
}

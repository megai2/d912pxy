/*
MIT License

Copyright(c) 2019-2020 megai2

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

d912pxy_extras::d912pxy_extras() : 
	overlayShowMode(eoverlay_show),
	hkDetected(false),
	gameActive(true)
{
	
}

d912pxy_extras::~d912pxy_extras()
{

}

void d912pxy_extras::Init()
{
	NonCom_Init(L"extras");

	activeTargetFrameTime = d912pxy_helper::SafeDiv(1000, d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_FPS_LIMIT));
	inactiveTargetFrameTime = d912pxy_helper::SafeDiv(1000, d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_FPS_LIMIT_INACTIVE));

	if (activeTargetFrameTime && !inactiveTargetFrameTime)
		inactiveTargetFrameTime = activeTargetFrameTime;

	waitEvent = CreateEvent(0, 0, 0, 0);

	frameTime.Reset();
	frTimeMs = 1;
	targetFrameTimeDelay = 0;

	//load config
	
	bShowFps = d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_SHOW_FPS);
	bShowDrawCount = d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_SHOW_DRAW_COUNT);
	bShowFpsGraph = d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_SHOW_FPS_GRAPH);
	bEnableConfigEditor = d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_CONFIG_EDITOR);

	if (bShowFpsGraph)
	{
		fpsGraph.Data = new d912pxy_ringbuffer<float>(PXY_INNER_EXTRA_FPS_GRAPH_PTS, 0);
		fpsGraph.h = (float)d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_FPS_GRAPH_H);
		fpsGraph.w = (float)d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_FPS_GRAPH_W);
		fpsGraph.max = (float)d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_FPS_GRAPH_MAX);
		fpsGraph.min = (float)d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_FPS_GRAPH_MIN);
	}

	bShowTimings = d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_SHOW_TIMINGS);
	bShowPSOCompileQue = d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_SHOW_PSO_COMPILE_QUE);
	bShowGCQue = d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_SHOW_GC_QUE);
	bShowConfigEditor = false;

	hkVKeyCode = d912pxy_s.config.GetValueUI32(PXY_CFG_EXTRAS_OVERLAY_TOGGLE_KEY);

	if (bEnableConfigEditor)
	{
		d912pxy_s.config.InitNewValueBuffers();
		d912pxy_s.config.ValueToNewValueBuffers();
	}

	//imgui setup

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	d912pxy_swapchain* swpc = d912pxy_s.dev.GetPrimarySwapChain();

	d912pxy_dheap* srvHeap = d912pxy_s.dev.GetDHeap(PXY_INNER_HEAP_SRV);
	UINT32 srvHeapSlot = srvHeap->OccupySlot();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	io.IniFilename = d912pxy_helper::GetFilePath(FP_IMGUI_INI)->s;
	io.LogFilename = d912pxy_helper::GetFilePath(FP_IMGUI_LOG)->s;

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
		delete fpsGraph.Data;

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	d912pxy_s.config.UnInitNewValueBuffers();

	d912pxy_noncom::UnInit();
}

static float fps_graph_buffer_transform(void* data, int idx)
{
	d912pxy_ringbuffer<float>* rb = (d912pxy_ringbuffer<float>*)data;

	return rb->GetElementOffset(idx);
}

void d912pxy_extras::OnPresent()
{
	if (overlayShowMode != eoverlay_hide)
		DrawOverlay();

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

	INT64 targetFrameTime = gameActive ? activeTargetFrameTime : inactiveTargetFrameTime;

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
			if (targetDelta > 10)
			{	
				if (targetDelta < targetFrameTimeDelay)
					targetFrameTimeDelay -= targetDelta;
				else
					targetFrameTimeDelay = targetFrameTimeDelay >> 1;

			} else
				targetFrameTimeDelay -= 1;
		}
	}

	WaitForSingleObject(waitEvent, (DWORD)targetFrameTimeDelay);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool d912pxy_extras::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYUP:
		case WM_KEYDOWN:
		{
			bool extraMod = 
				//d912pxy_helper::IsKeyDown(VK_SHIFT) &&
				d912pxy_helper::IsKeyDown(VK_MENU) &&
				d912pxy_helper::IsKeyDown(VK_CONTROL);

			bool targetKey = d912pxy_helper::IsKeyDown(hkVKeyCode);

			bool hkPressed = extraMod & targetKey;
			
			if (hkPressed && !hkDetected)
			{
				hkDetected = true;

				return true;
			}
			else if (hkDetected && !hkPressed)
			{
				hkDetected = false;
				OnHotkeyTriggered();

				return true;
			}
		}
		break;
		case WM_SETFOCUS:
		{
			gameActive = true;
		}
		break;
		case WM_KILLFOCUS:
		{
			gameActive = false;
		}
		break;
	}

	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	return false;
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

void d912pxy_extras::OnHotkeyTriggered()
{
	overlayShowMode = (overlayShowMode + 1) % eoverlay_modes_count;
}

void d912pxy_extras::DrawConfigEditor() 
{
	ImGui::Begin("d912pxy config editor", &bShowConfigEditor);
	
	for (int configIndex = 0; configIndex != PXY_CFG_CNT; ++configIndex)
	{
		d912pxy_config_value_dsc* iter = d912pxy_s.config.GetEntryRaw(d912pxy_config_value(configIndex));

		//Render parameter name
		ImGui::Text("%S", iter->name);
		ImGui::SameLine(250);

		/*Tooltip when hovered (DISABLED UNTIL DESCRIPTIONS ARE POPULATED)

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::Text("%S", iter->longDescription);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		*/
			
		//Render InputText box for newValues
		ImGui::PushID(configIndex);
		ImGui::PushItemWidth(-200);
		ImGui::InputText("##On", iter->newValue, 254);
		ImGui::PopItemWidth();
		ImGui::PopID();
	}

	if (bShowConfigEditorRestartMsg) 
	{
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImGui::SetWindowFontScale(1.2f);
		ImGui::Text("RESTART THE APPLICATION TO APPLY CONFIG CHANGES");
		ImGui::SetWindowFontScale(1.0f);
		ImGui::PopStyleColor();
	}

	if (ImGui::Button("Save"))
	{
		d912pxy_s.config.SaveConfig();
		bShowConfigEditorRestartMsg = true;
	}

	ImGui::SameLine();

	if (ImGui::Button("Reset"))
	{
		d912pxy_s.config.ValueToNewValueBuffers();
	}

	ImGui::End();
}

void d912pxy_extras::DrawOverlay()
{
	ImGUI_Render_Start();

	ImGui::Begin(BUILD_VERSION_NAME, nullptr, (overlayShowMode != eoverlay_edit) * (ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs));

	if (bShowFps)
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	if (bShowTimings)
		ImGui::Text("%4.2f ms/frame overhead", syncNexecTime.GetStopTime() / 1000.0f);

	if (bShowDrawCount)
		ImGui::Text("%6u draw batches", d912pxy_s.render.batch.GetBatchCount());

	if (bShowGCQue)
		ImGui::Text("%6u GC", d912pxy_s.thread.cleanup.TotalWatchedItems());

	if (bShowPSOCompileQue)
		ImGui::Text("%6u PSO", d912pxy_s.render.db.pso.GetCompileQueueLength());

	if (bShowFpsGraph)
	{
		if (!fpsGraph.Data->HaveFreeSpace())
			fpsGraph.Data->Next();

		fpsGraph.Data->WriteElement(1000.0f / frTimeMs);

		ImGui::PlotLines("FPS", &fps_graph_buffer_transform, fpsGraph.Data, PXY_INNER_EXTRA_FPS_GRAPH_PTS, 0, 0, fpsGraph.min, fpsGraph.max, ImVec2(fpsGraph.w, fpsGraph.h));
	}

	if (bEnableConfigEditor && (overlayShowMode == eoverlay_edit))
	{
		if (ImGui::Button("Edit config"))
			bShowConfigEditor = !bShowConfigEditor;
	}

	ImGui::End();

	if (bShowConfigEditor)
		DrawConfigEditor();

	ImGUI_Render_End();

}
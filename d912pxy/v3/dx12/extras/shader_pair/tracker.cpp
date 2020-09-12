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

using namespace d912pxy::extras::ShaderPair;

void Tracker::updateList()
{
	listData.reset();

	auto& list = d912pxy_s.render.replay.extras.pairTracker.read();

	for (intptr_t i = 1; i < list.headIdx(); ++i)
		listData.push(&d912pxy_s.spairInfo.find(list[i]));

	d912pxy_s.render.replay.extras.pairTracker.finishRead();
}

void Tracker::handleFreeze()
{
	bool toggleFreeze = ImGui::Button(freezeLastFrame ? "Unfreeze" : "Freeze");
	if (toggleFreeze)
		freezeLastFrame = !freezeLastFrame;

	if (!freezeLastFrame)
		updateList();
}

void d912pxy::extras::ShaderPair::Tracker::handleReload()
{
	ImGui::SameLine();
	if (ImGui::Button("Reload"))
	{
		d912pxy_s.spairInfo.reload();
		updateList();
	}
}

void d912pxy::extras::ShaderPair::Tracker::drawList()
{
	bool showSubgrp = true;
	int hiddenDepth = 0;
	int openedNodes = 0;
	int nestedDraws = 1;
	int passIdx = 0;
	
	for (intptr_t i = 1; i < listData.headIdx(); ++i)
	{		
		Info& v = *listData[i];		

		if ((i+1) < listData.headIdx())
		{
			if (listData[i + 1]->spair == v.spair)
			{
				++nestedDraws;
				continue;
			}				
		}

		switch (v.drawType)
		{
		case DrawType::simple:
			break;
		case DrawType::pass_start:
			char buf[255];
			sprintf(buf, "%S %i", v.name ? v.name : L"unknown", passIdx);
			++passIdx;
			if (showSubgrp)
			{
				showSubgrp = ImGui::TreeNode(buf);
				if (showSubgrp)
					++openedNodes;
			}
			if (!showSubgrp)
				++hiddenDepth;
			break;
		}

		if (showSubgrp)
		{
			ImGui::BulletText("%lli: <%016llX> %S (%u)", i, v.spair, v.name ? v.name : L"unknown", nestedDraws);
			char tgButton[255];
			sprintf(tgButton, "toggle##%016llX 0x%Ix", v.spair, i);
			ImGui::SameLine();
			{
				d912pxy::mt::containter::Ref<d912pxy_replay::ExtraFeatures::PairTracker::ExclusionStorage> excRef(d912pxy_s.render.replay.extras.pairTracker.exclusions, v.spair);				

				if (ImGui::Button(tgButton))
					excRef.val = !excRef.val;
				ImGui::SameLine();
				ImGui::Text(excRef.val ? "disabled" : "enabled");
			}
		}
		nestedDraws = 1;

		switch (v.drawType)
		{
		case DrawType::pass_end:
			if (hiddenDepth)
			{
				--hiddenDepth;
				if (!hiddenDepth)
					showSubgrp = true;
			}
			else if (openedNodes) {
				--openedNodes;
				ImGui::TreePop();
			}
			break;
		}
	}

	while (openedNodes)
	{
		--openedNodes;
		ImGui::TreePop();
	}
}

void d912pxy::extras::ShaderPair::Tracker::init()
{
	NonCom_Init(L"shader pair tracker ui");
}

void Tracker::draw()
{
	ImGui::Begin("ShaderPairTracker");

	handleFreeze();	
	handleReload();
	drawList();

	ImGui::End();
}

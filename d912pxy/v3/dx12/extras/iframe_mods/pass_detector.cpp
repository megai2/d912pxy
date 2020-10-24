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
#include "pass_detector.h"

using namespace d912pxy::extras::IFrameMods;

void PassDetector::enter(d912pxy_replay_thread_context* rpContext)
{
	justEntered = true;
	inPass = true;

	surfaces[SURF_TRACKED + SURF_DS] = rpContext->tracked.surfBind[0];
	surfaces[SURF_TRACKED + SURF_RT] = rpContext->tracked.surfBind[1];
}

void PassDetector::exit()
{
	justExited = true;
	inPass = false;
}

void PassDetector::neutral()
{
	justExited = false;
	justEntered = false;
}

PassDetector::PassDetector(const wchar_t* prevLastDrawMarker, const wchar_t* firstDrawMarker, int iRTDSmask, bool iTriggerOnSameTargets)
	: triggerOnSameRT(iTriggerOnSameTargets)
	, RTDSmask(iRTDSmask)
	, prevLastDrawSpair(0)
	, firstDrawSpair(0)
	, inPass(false)
	, justEntered(false)
	, justExited(false)
{
	if (!prevLastDrawMarker && !firstDrawMarker)
	{
		error::fatal(L"pass detector don't have markers to detect on");
	}
	prevLastDrawSpair = d912pxy_s.spairInfo.getSpairForMarker(prevLastDrawMarker);
	firstDrawSpair = d912pxy_s.spairInfo.getSpairForMarker(firstDrawMarker);
}

void PassDetector::RP_PreDraw(d912pxy_replay_item::dt_draw_indexed* rpItem, d912pxy_replay_thread_context* rpContext)
{
	if (rpContext->tracked.spair == firstDrawSpair)	
		enter(rpContext);	
}

void PassDetector::RP_PostDraw(d912pxy_replay_item::dt_draw_indexed* rpItem, d912pxy_replay_thread_context* rpContext)
{
	if (rpContext->tracked.spair == prevLastDrawSpair)
		enter(rpContext);
	else
		neutral();
}

void d912pxy::extras::IFrameMods::PassDetector::RP_RTDSChange(d912pxy_replay_item::dt_om_render_targets* rpItem, d912pxy_replay_thread_context* rpContext)
{
	surfaces[SURF_LAST + SURF_DS] = surfaces[SURF_DS];
	surfaces[SURF_LAST + SURF_RT] = surfaces[SURF_RT];
	surfaces[SURF_DS] = rpContext->tracked.surfBind[0];
	surfaces[SURF_RT] = rpContext->tracked.surfBind[1];

	if (inPass)
		exit();
	else if (
		triggerOnSameRT && 
		((surfaces[SURF_TRACKED + SURF_DS] == surfaces[SURF_DS]) || ((RTDSmask & 2) == 0)) &&
		((surfaces[SURF_TRACKED + SURF_RT] == surfaces[SURF_RT]) || ((RTDSmask & 1) == 0))
	)
	{
		enter(rpContext);
	}
}

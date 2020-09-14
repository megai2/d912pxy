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

d912pxy_replay_buffer::d912pxy_replay_buffer()
{
}

d912pxy_replay_buffer::~d912pxy_replay_buffer()
{
}

void d912pxy_replay_buffer::Init()
{
	NonCom_Init(L"replay_buffer");

	d912pxy_mem_block area = d912pxy_mem_block::allocZero(
		&base, 
		d912pxy_s.config.GetValueUI32(PXY_CFG_BATCHING_MAX_BATCHES_PER_IFRAME) * d912pxy_s.config.GetValueUI32(PXY_CFG_REPLAY_ITEMS_PER_BATCH)
	);
	bufferLimit = (intptr_t)area.block_end();

	Reset();
}

void d912pxy_replay_buffer::UnInit()
{
	PXY_FREE(base);

	d912pxy_noncom::UnInit();
}

void d912pxy_replay_buffer::Reset()
{
#ifdef ENABLE_METRICS
	d912pxy_s.log.metrics.TrackReplayItems(linearIdx);
#endif
	linearIdx = 0;
	current = (d912pxy_replay_item*)base;
	syncCurrent();
}

d912pxy_replay_item* d912pxy_replay_buffer::getCurrentExtern()
{
	return externCurrent;
}

d912pxy_replay_item* d912pxy_replay_buffer::getCurrent()
{
	return current;
}

d912pxy_replay_item* d912pxy_replay_buffer::getBase()
{
	return base;
}

void d912pxy_replay_buffer::syncCurrent()
{
	externCurrent = current;
}

void d912pxy_replay_buffer::CheckRange()
{
	if ((intptr_t)current >= bufferLimit)
		LOG_ERR_THROW2(-1, "replay buffer overflow, extend replay items per batch");
}

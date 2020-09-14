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
#pragma once
#include "stdafx.h"

class d912pxy_replay_buffer : public d912pxy_noncom
{
public:
	d912pxy_replay_buffer();
	~d912pxy_replay_buffer();

	void Init();
	void UnInit();

	void Reset();

	d912pxy_replay_item* getCurrentExtern();
	d912pxy_replay_item* getCurrent();
	d912pxy_replay_item* getBase();
	void syncCurrent();

	UINT getIndex() { return linearIdx; };

	template<class dataType>
	dataType* PushAction()
	{
		dataType* ret;
		current = current->Advance(&ret);
		++linearIdx;

#ifdef _DEBUG
		CheckRange();
#endif
		return ret;
	}

	void CheckRange();

private:
	UINT linearIdx;

	d912pxy_replay_item* base;
	d912pxy_replay_item* current;
	std::atomic<d912pxy_replay_item*> externCurrent;
	intptr_t bufferLimit=0;
};
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
#pragma once
#include "stdafx.h"

class d912pxy_thread_lock
{
public:
	d912pxy_thread_lock();	
	~d912pxy_thread_lock();
	
	UINT32 TryHold();
	void Hold();
	void Release();
	void Init();

	LONG LockedAdd(LONG val);
	void LockedSet(LONG val);

	void WaitHold(LONG cond);
	void HoldWait(LONG cond);

	LONG SpinOnce(LONG cond);

	LONG Add(LONG val);
	void Wait(LONG cond);
	UINT WaitTimeout(LONG cond, DWORD ms);	

	LONG GetValue();
	void SetValue(LONG val);
	void SetValueAsync(LONG val);

	void ResetLock();
	
private:	
	CRITICAL_SECTION cs;
	std::atomic<LONG> spinLock;

};


#pragma once
#include "stdafx.h"

class d912pxy_thread_lock
{
public:
	d912pxy_thread_lock();
	~d912pxy_thread_lock();

	void Hold();
	void Release();

private:
	CRITICAL_SECTION cs;
};


#include "stdafx.h"

d912pxy_thread_lock::d912pxy_thread_lock()
{
	InitializeCriticalSection(&cs);
}


d912pxy_thread_lock::~d912pxy_thread_lock()
{

}

void d912pxy_thread_lock::Hold()
{
	EnterCriticalSection(&cs);
}

void d912pxy_thread_lock::Release()
{
	LeaveCriticalSection(&cs);
}

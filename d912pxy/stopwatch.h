#pragma once
#include "stdafx.h"

class Stopwatch final
{
public:
	
	Stopwatch()
	{
		Reset();
	}

	void Reset()
	{
		QueryPerformanceCounter(&reset_time);
	}

	UINT64 Elapsed()
	{
		QueryPerformanceCounter(&stopTime);
		return stopTime.QuadPart - reset_time.QuadPart;
	}

	UINT64 GetStopTime()
	{
		return stopTime.QuadPart;
	}


	void Stop()
	{
		stopTime.QuadPart = Elapsed();
	}

private:

	LARGE_INTEGER stopTime;
	LARGE_INTEGER reset_time;
};

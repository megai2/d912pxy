#pragma once
#include "stdafx.h"

class Stopwatch final
{
public:
	
	Stopwatch()
	{
		QueryPerformanceFrequency(&Frequency);
		Reset();
	}

	void Reset()
	{
		QueryPerformanceCounter(&reset_time);
	}

	UINT64 Elapsed()
	{
		QueryPerformanceCounter(&stopTime);
		return (stopTime.QuadPart - reset_time.QuadPart) * 1000000 / Frequency.QuadPart;
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

	LARGE_INTEGER Frequency;
	LARGE_INTEGER stopTime;
	LARGE_INTEGER reset_time;
};

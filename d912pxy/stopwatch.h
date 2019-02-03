#pragma once
#include "stdafx.h"

class Stopwatch final
{
public:

	using elapsed_resolution = std::chrono::microseconds;

	Stopwatch()
	{
		Reset();
	}

	void Reset()
	{
		reset_time = clock.now();
	}

	elapsed_resolution Elapsed()
	{
		return std::chrono::duration_cast<elapsed_resolution>(clock.now() - reset_time);
	}

	elapsed_resolution GetStopTime()
	{
		return stopTime;
	}


	void Stop()
	{
		stopTime = Elapsed();
	}

private:

	elapsed_resolution stopTime;

	std::chrono::high_resolution_clock clock;
	std::chrono::high_resolution_clock::time_point reset_time;
};

#pragma once
#include <chrono>

//TODO: Implement Destructor that outputs elapsed time

class Timer
{
public:
	Timer()
		: startTime(std::chrono::steady_clock::now())
	{}
	//Returns the time from last time check
	float GetElapsedSeconds()
	{
		//Get time elapsed in seconds
		std::chrono::duration<float, std::ratio<1>> elapsed = std::chrono::steady_clock::now() - startTime;

		//Update startTime to current time
		startTime = std::chrono::steady_clock::now();

		return elapsed.count();
	}
private:
	std::chrono::time_point<std::chrono::steady_clock> startTime;
};
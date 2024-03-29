#pragma once
#include <chrono>
#include "SathwareEngine.h"

//TODO: Implement Destructor that outputs elapsed time

class SathwareAPI Timer
{
public:
	Timer()
		: startTime(std::chrono::steady_clock::now())
	{}
	//Returns the time elapsed from the last time this function was called
	//The first time this function is called, it returns the time elapsed since the Timer object was created
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
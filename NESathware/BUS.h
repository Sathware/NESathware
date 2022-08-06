#pragma once
#include "CommonTypes.h"
#include <array>

//Handles inter-component communication
class BUS
{
public:
	//2KB onboard ram
	std::array<ubyte, 0xffff> RAM = { 0 };
};


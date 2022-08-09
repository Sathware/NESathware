#pragma once
#include "CommonTypes.h"
#include <array>

//Handles inter-component communication
class BUS
{
public:
	//2KB onboard ram and rest of address space
	std::array<ubyte, 65536> RAM = { 0 };
};


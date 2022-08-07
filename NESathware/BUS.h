#pragma once
#include "CommonTypes.h"
#include <array>
#include <stdexcept>
#include "CPU_6052.h"
#include "Cartridge.h"

//Facilitates inter-component communication
class BUS
{
	//CPU memory map source: "https://www.nesdev.org/wiki/CPU_memory_map"
public:
	ubyte& Read(ubyte2 address);
	void Write(ubyte val, ubyte2 address);

	CPU_6052* Cpu;
	Cartridge* cartridge;

	//2KB onboard ram
	std::array<ubyte, 65536> RAM = { 0 };
};


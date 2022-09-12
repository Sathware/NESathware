#pragma once
#include "CommonTypes.h"
#include "CPU_6052.h"
#include "PPU_2C02.h"
#include "APU_2A03.h"
#include "Mapper.h"
#include <array>
#include <functional>

//Handles inter-component communication
class BUS
{
public:
	//Maps data appropriately to the various components depending on the address
	//Source: "https://www.nesdev.org/wiki/CPU_memory_map"
	ubyte ReadCPU(ubyte2 address);
	ubyte ReadPPU(ubyte2 address);
	void WriteCPU(ubyte val, ubyte2 address);
	void WritePPU(ubyte val, ubyte2 address);
	void InvokeNMI()
	{
		mpCPU->NMI();
	}
public:
	CPU_6052* mpCPU = nullptr;
	PPU_2C02* mpPPU = nullptr;
	APU_2A03* mpAPU = nullptr;
	Mapper* mpCartridge = nullptr;
	std::function<ubyte2(ubyte2)> Mirror;
	//2KB onboard ram and rest of address space
	std::array<ubyte, 0x0800u> mRAM = { 0 };
	//2KB onboard VRAM
	std::array<ubyte, 0x0800u> mVRAM = { 0 };
};


#pragma once
#include "CommonTypes.h"
#include "BUS.h"
#include "CPU_6052.h"
#include "Mapper.h"
#include "PPU_2C02.h"
#include "APU_2A03.h"
#include "Controller.h"

class NES
{
public:
	NES(std::string romFileName, class Graphics& gfx, class Audio& audio, class DesktopWindow& window)
		: mBus(), mCPU(mBus), mPPU(mBus, gfx), mAPU(mBus, audio), mController(mBus, window), mpCartridge(LoadRom(romFileName))
	{
		mBus.mpCartridge = mpCartridge.get();
		mBus.mpCPU = &mCPU;
		mBus.mpPPU = &mPPU;
		mBus.mpAPU = &mAPU;
		mBus.mpController = &mController;
		mCPU.Reset();
	}

	void Run(float dt)
	{
		constexpr float timePerCPUCycle = 0.00000055873f;
		if (dt > timePerCPUCycle)
			dt = timePerCPUCycle;

		for (; dt > 0; dt -= timePerCPUCycle)
		{
			//Poll user input
			mController.Execute();
			mCPU.Execute();
			mAPU.Execute();
			//3 PPU clock cycles = 1 CPU clock cycle
			mPPU.Execute();
			mPPU.Execute();
			mPPU.Execute();
		}
	}

	BUS mBus;
	CPU_6052 mCPU;//NES Central Processing Unit
	PPU_2C02 mPPU;//NES Picture Processing Unit
	APU_2A03 mAPU;//NES Audio Processing Unit
	Controller mController;
	std::unique_ptr<Mapper> mpCartridge;//NES Cartridge
private:

	std::unique_ptr<Mapper> LoadRom(std::string filename)
	{
		Header header;

		std::ifstream file(filename, std::ifstream::binary);
		file.exceptions(std::ifstream::badbit | std::ifstream::failbit);

		file.read(reinterpret_cast<char*>(&header), 16);

		//------------TODO: Do stuff with trainer if present

		ubyte mapperNum = (header.flags7 & 0xf0u) | (header.flags6 >> 4u);
		assert(mapperNum == 0);

		if (IsBitOn<0>(header.flags6))
		{
			//Horizontal arrangement
			mBus.Mirror = [](ubyte2 address) {return address % 0x800u; };
			mPPU.nextNametableOffset = 0x400u;
		}
		else
		{
			//Vertical Arrrangement
			mBus.Mirror = [](ubyte2 address) { return 0x400 * (address >= 0x2800u) + address % 0x400u; };
			mPPU.nextNametableOffset = 0x800u;
		}

		switch (mapperNum)
		{
		case 0: return std::make_unique<Mapper0>(header, file);
			//case 1: mpMapper = std::make_unique<Mapper1>(header, file); break;
		default: return nullptr;
		}
	}
};
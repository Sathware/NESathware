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
	NES(std::string romFileName, class Graphics& gfx, class DesktopWindow& window)
		: mBus(), mpCartridge(LoadRom(romFileName)), mCPU(mBus), mPPU(mBus, gfx), mAPU(mBus), mController(mBus, window)
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
		if (dt > 1)
			dt = 0.00000056f;

		for (; dt > 0; dt -= 0.00000056f)
		{
			//Poll user input
			mController.Execute();
			mCPU.Execute();
			//3 PPU clock cycles = 1 CPU clock cycle
			mPPU.Execute();
			mPPU.Execute();
			mPPU.Execute();
		}
	}

	BUS mBus;
	std::unique_ptr<Mapper> mpCartridge;//NES Cartridge
	CPU_6052 mCPU;//NES Central Processing Unit
	PPU_2C02 mPPU;//NES Picture Processing Unit
	APU_2A03 mAPU;//NES Audio Processing Unit
	Controller mController;
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
			mBus.Mirror = [](ubyte2 address) {return address % 0x800u; };
		}
		else
		{
			//mBus.Mirror = [](ubyte2 address) {return address % 0x800u; };
			mBus.Mirror = [](ubyte2 address) { return 0x400 * (address >= 0x2800u) + address % 0x400u; };
		}

		switch (mapperNum)
		{
		case 0: return std::make_unique<Mapper0>(header, file);
			//case 1: mpMapper = std::make_unique<Mapper1>(header, file); break;
		default: return nullptr;
		}
	}
};
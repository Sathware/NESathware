#pragma once
#include "CommonTypes.h"
#include "BUS.h"
#include "CPU_6052.h"
#include "Mapper.h"
#include "PPU_2C02.h"
#include "APU_2A03.h"

class NES
{
public:
	NES(std::string romFileName, class Graphics& gfx, ubyte2 cpuStartOverride)
		: mBus(), mpCartridge(LoadRom(romFileName)), mCPU(mBus, cpuStartOverride), mPPU(mBus, gfx), mAPU(mBus)
	{
		mBus.mpCartridge = mpCartridge.get();
		mBus.mpCPU = &mCPU;
		mBus.mpPPU = &mPPU;
		mBus.mpAPU = &mAPU;
	}

	void Run(float dt)
	{
		for (; dt > 0; dt -= 0.00000056f)
		{
			mCPU.Execute();
		}
	}

private:
	BUS mBus;

	std::unique_ptr<Mapper> LoadRom(std::string filename)
	{
		Header header;

		std::ifstream file(filename, std::ifstream::binary);
		file.exceptions(std::ifstream::badbit | std::ifstream::failbit);

		file.read(reinterpret_cast<char*>(&header), 16);

		//------------TODO: Do stuff with trainer if present

		ubyte mapperNum = (header.flags7 & 0xf0) | (header.flags6 >> 4);

		switch (mapperNum)
		{
		case 0: return std::make_unique<Mapper0>(header, file);
			//case 1: mpMapper = std::make_unique<Mapper1>(header, file); break;
		default: return nullptr;
		}
	}

	std::unique_ptr<Mapper> mpCartridge;//NES Cartridge
	CPU_6052 mCPU;//NES Central Processing Unit
	PPU_2C02 mPPU;//NES Picture Processing Unit
	APU_2A03 mAPU;//NES Audio Processing Unit
};
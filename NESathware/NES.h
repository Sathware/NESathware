#pragma once
#include "CommonTypes.h"
#include "BUS.h"
#include "CPU_6052.h"
#include "Cartridge.h"
#include "PPU_2C02.h"
#include "APU_2A03.h"

class NES
{
public:
	NES(std::string romFileName, ubyte2 cpuStartOverride)
		: mBus(), mCartridge(mBus, romFileName), mCPU(mBus, cpuStartOverride), mPPU(mBus), mAPU(mBus)
	{
		mBus.mpCartridge = &mCartridge;
		mBus.mpCPU = &mCPU;
		mBus.mpPPU = &mPPU;
		mBus.mpAPU = &mAPU;
	}

	size_t Execute()
	{
		mTotalCycles += mCPU.Execute();
		return mTotalCycles;
	}
private:
	BUS mBus;
	Cartridge mCartridge;//NES Cartridge
	CPU_6052 mCPU;//NES Central Processing Unit
	PPU_2C02 mPPU;//NES Picture Processing Unit
	APU_2A03 mAPU;//NES Audio Processing Unit
	size_t mTotalCycles = 0;
};
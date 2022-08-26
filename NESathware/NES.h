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
	NES(std::string romFileName, Graphics& gfx, ubyte2 cpuStartOverride)
		: mBus(), 
		mCartridge(mBus, romFileName), 
		mCPU(mBus, cpuStartOverride), 
		mPPU(mBus, gfx), 
		mAPU(mBus)
	{
		mBus.mpCartridge = &mCartridge;
		mBus.mpPPU = &mPPU;
		mBus.mpAPU = &mAPU;
		mBus.mpCPU = &mCPU;

		//mCPU.Reset();
	}

	NES(std::string romFileName, Graphics& gfx)
		: mBus(),
		mCartridge(mBus, romFileName),
		mCPU(mBus, 0),
		mPPU(mBus, gfx),
		mAPU(mBus)
	{
		mBus.mpCartridge = &mCartridge;
		mBus.mpPPU = &mPPU;
		mBus.mpAPU = &mAPU;
		mBus.mpCPU = &mCPU;

		mCPU.Reset();
	}

	//Execute steps depending on the time elapsed in seconds
	bool Execute()
	{
		//PPU runs 3 times faster than CPU
		mPPU.Execute();
		mPPU.Execute();
		mPPU.Execute();
		mCPU.Execute();
		//mPPU.Render();
		//mCPU.NMI();
		return mPPU.FrameReady();
	}
private:
	BUS mBus;
	Cartridge mCartridge;//NES Cartridge
	CPU_6052 mCPU;//NES Central Processing Unit
	PPU_2C02 mPPU;//NES Picture Processing Unit
	APU_2A03 mAPU;//NES Audio Processing Unit
	size_t mCPU_WaitCycles = 0;
	size_t mPPU_WaitCycles = 0;
};
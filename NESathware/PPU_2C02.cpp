#include "PPU_2C02.h"
#include "BUS.h"

ubyte PPU_2C02::ReadRegister(ubyte2 address)
{
	//Convert from CPU Memory address to PPU register index
	unsigned int regIndex = address % 0x0008;

	switch (regIndex)
	{
	case 2/*PPUSTATUS*/:
	{
		//fill bus latch
		mBusLatch = mPPUSTATUS;
		//turn off bit 7
		mPPUSTATUS &= 0xef;
		//reset address latch
		mAddressLatch = false;
		return mBusLatch; 
	}
	case 4/*OAMDATA*/:
	{
		//fill bus latch
		mBusLatch = mOAM[mOAMADDR];
		//address isn't incremented during vblank or forced blanking
		if (!isVBLANK())
			++mOAMADDR;
		return mBusLatch;
	}
	case 7/*PPUDATA*/: return ReadPPUADDR();
	default: return mBusLatch;
	}
}

void PPU_2C02::WriteRegister(ubyte val, ubyte2 address)
{
	//Convert from CPU Memory address to PPU register index
	unsigned int regIndex = address % 0x0008;
	//Writing to any register fills the bus latch
	mBusLatch = val;

	switch (regIndex)
	{
	case 0/*PPUCTRL*/: { mPPUCTRL = val; return; }
	case 1/*PPUMASK*/: { mPPUMASK = val; return; }
	case 2/*PPUSTATUS*/: return;
	case 3/*OAMADDR*/: { mOAMADDR = val; return; }
	case 4/*OAMDATA*/: { mOAM[mOAMADDR] = val; ++mOAMADDR; return; }
	case 5/*PPUSCROLL*/: 
	{
		//value being written changes depending on address latch
		if (mAddressLatch)
			mYPPUSCROLL = val;
		else
			mXPPUSCROLL = val;
		mAddressLatch = !mAddressLatch;//toggle latch
		return; 
	}
	case 6/*PPUADDR*/: 
	{
		//value being written changes depending on address latch
		if (mAddressLatch)
			mLowPPUADDR = val;
		else
			mHighPPUADDR = val;
		mAddressLatch = !mAddressLatch;//toggle latch
		return;
	}
	case 7/*PPUDATA*/: 
	{
		ubyte2 PPUAddress = CombineBytes(mHighPPUADDR, mLowPPUADDR);
		//Change internal palette or VRAM depending on address
		if (PPUAddress % 0x4000u >= 0x3f00)
			//Mirror address down into standard range 0 - 0xfff, then handle palette mirrors
			mPalette[(PPUAddress % 0x4000u - 0x3f00) % 0x0020u] = val;
		else
			Write(val, PPUAddress);
		//Increment address
		PPUAddress += isBitOn<2>(mPPUCTRL) ? 32 : 1;
		mHighPPUADDR = HighByte(PPUAddress);
		mLowPPUADDR = LowByte(PPUAddress);
		return;
	}
	}
}

ubyte PPU_2C02::ReadPPUADDR()
{
	ubyte2 PPUAddress = CombineBytes(mHighPPUADDR, mLowPPUADDR);
	//return palette data immediately else return read buffer
	if (PPUAddress % 0x4000u >= 0x3f00)
		//Mirror address down into standard range 0 - 0xfff, then handle palette mirrors
		mBusLatch = mPalette[(PPUAddress % 0x4000u - 0x3f00) % 0x0020u];
	else
	{
		mBusLatch = mReadBuffer;
		mReadBuffer = Read(PPUAddress);
	}
	//Increment Address register
	PPUAddress += isBitOn<2>(mPPUCTRL) ? 32 : 1;
	mHighPPUADDR = HighByte(PPUAddress);
	mLowPPUADDR = LowByte(PPUAddress);
	return mBusLatch;
}

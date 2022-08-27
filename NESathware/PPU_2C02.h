#pragma once
#include "CommonTypes.h"
#include "../SathwareEngine/Graphics.h"

class PPU_2C02
{
public:
	PPU_2C02(class BUS& bus, Graphics& gfx)
		: bus(bus), gfx(gfx)
	{}
	ubyte ReadRegister(ubyte2 address);
	void WriteRegister(ubyte val, ubyte2 address);
private:
	BUS& bus;
	Graphics& gfx;

	/* Helper Functions */
	bool isVBLANK()
	{
		return isBitOn<7>(mPPUSTATUS);
	}
	ubyte Read(ubyte2 address)
	{
		return bus.ReadPPU(address);
	}
	void Write(ubyte val, ubyte2 address)
	{
		bus.WritePPU(val, address);
	}
	
	/*Internal Functions*/
	//Simulate internal read operation of PPU
	ubyte ReadPPUADDR();

	//PPU Registers
	ubyte mPPUCTRL,
		  mPPUMASK,
		  mPPUSTATUS,
		  mOAMADDR,//byte offset in OAM
		  mOAMDATA,
		  mXPPUSCROLL,
		  mYPPUSCROLL,
		  mHighPPUADDR,
		  mLowPPUADDR;
	//Used to simulate the effects of the internal bus latch used by the NES
	ubyte mBusLatch;
	//Used to simulate address latch used by OAMADDR and PPUSCROLL
	bool mAddressLatch;
	//Used to simulate read buffer used internally in NES
	ubyte mReadBuffer;

	/*struct Sprite
	{
		ubyte PosYTop;
		ubyte TileIndex;
		ubyte Attributes;
		ubyte PosXLeft;
	};*/
	//Object Attribute Memory
	ubyte mOAM[256u];
	//Pallette
	ubyte mPalette[32u];
};
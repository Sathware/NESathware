#pragma once
#include "CommonTypes.h"
#include "../SathwareEngine/Graphics.h"

class PPU_2C02
{
public:
	PPU_2C02(class BUS& bus, Graphics& gfx);
	void Execute();
	//Source: "https://www.nesdev.org/wiki/PPU_registers"
	ubyte ReadRegister(ubyte2 address);
	//Source: "https://www.nesdev.org/wiki/PPU_registers"
	void WriteRegister(ubyte val, ubyte2 address);
private:
	BUS& bus;
	Graphics& gfx;

	/*Rendering*/
	unsigned int mCurrentScanLine = 0;
	unsigned int mCurrentCycle = 0;
	//Draws 8 pixel sliver, Background Info: "https://austinmorlan.com/posts/nes_rendering_overview/", "https://www.nesdev.org/wiki/Blargg_PPU", "https://www.nesdev.org/wiki/PPU_registers", "https://www.nesdev.org/wiki/PPU_nametables", "https://www.nesdev.org/wiki/PPU_pattern_tables"
	void inline DrawSliver();

	/* Helper Functions */
	bool isVBLANK()
	{
		return isBitOn<7>(mPPUSTATUS);
	}
	ubyte Read(ubyte2 address);
	void Write(ubyte val, ubyte2 address);

	//PPU Registers
	ubyte mPPUCTRL = 0,
		  mPPUMASK = 0,
		  mPPUSTATUS = 0,
		  mOAMADDR = 0,//byte offset in OAM
		  mOAMDATA = 0,
		  mXPPUSCROLL = 0,
		  mYPPUSCROLL = 0,
		  mHighPPUADDR = 0,
		  mLowPPUADDR = 0;
	//Used to simulate the effects of the internal bus latch used by the NES
	ubyte mBusLatch = 0;
	//Used to simulate address latch used by OAMADDR and PPUSCROLL
	bool mAddressLatch = false;
	//Used to simulate read buffer used internally in NES
	ubyte mReadBuffer = 0;

	/*struct Sprite
	{
		ubyte PosYTop;
		ubyte TileIndex;
		ubyte Attributes;
		ubyte PosXLeft;
	};*/
	//Object Attribute Memory
	ubyte mOAM[256u] = { 0 };
	//Pallette
	ubyte mPalette[32u] = { 0 };
};
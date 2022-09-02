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

	/* Emulation */
	int mWaitCycles = 0;
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
	};
    struct SubPalette
    {
        ubyte BackgroundIndex;
        ubyte Color1Index;
        ubyte Color2;
        ubyte Color3;
    };*/

	//Object Attribute Memory
	ubyte mOAM[256u] = { 0 };
	//Pallette
	ubyte mPalette[32u] = { 0 };

	const Color mSystemPalette[64u] =
	{
        {0x7C7C7CFF},
        {0x0000FCFF},
        {0x0000BCFF},
        {0x4428BCFF},
        {0x940084FF},
        {0xA80020FF},
        {0xA81000FF},
        {0x881400FF},
        {0x503000FF},
        {0x007800FF},
        {0x006800FF},
        {0x005800FF},
        {0x004058FF},
        {0x000000FF},
        {0x000000FF},
        {0x000000FF},
        {0xBCBCBCFF},
        {0x0078F8FF},
        {0x0058F8FF},
        {0x6844FCFF},
        {0xD800CCFF},
        {0xE40058FF},
        {0xF83800FF},
        {0xE45C10FF},
        {0xAC7C00FF},
        {0x00B800FF},
        {0x00A800FF},
        {0x00A844FF},
        {0x008888FF},
        {0x000000FF},
        {0x000000FF},
        {0x000000FF},
        {0xF8F8F8FF},
        {0x3CBCFCFF},
        {0x6888FCFF},
        {0x9878F8FF},
        {0xF878F8FF},
        {0xF85898FF},
        {0xF87858FF},
        {0xFCA044FF},
        {0xF8B800FF},
        {0xB8F818FF},
        {0x58D854FF},
        {0x58F898FF},
        {0x00E8D8FF},
        {0x787878FF},
        {0x000000FF},
        {0x000000FF},
        {0xFCFCFCFF},
        {0xA4E4FCFF},
        {0xB8B8F8FF},
        {0xD8B8F8FF},
        {0xF8B8F8FF},
        {0xF8A4C0FF},
        {0xF0D0B0FF},
        {0xFCE0A8FF},
        {0xF8D878FF},
        {0xD8F878FF},
        {0xB8F8B8FF},
        {0xB8F8D8FF},
        {0x00FCFCFF},
        {0xF8D8F8FF},
        {0x000000FF},
        {0x000000FF},
	};
		
};
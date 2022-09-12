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
	//Bulk transfer OAM Data from CPU RAM to PPU
	void WriteOAMDMA(ubyte* data);
	/*Debug*/
	void DisplayCHRROM();
private:
	BUS& bus;
	Graphics& gfx;

	//A subpalette is 4 bytes long, and houses indexes into the system palette
	struct SubPalette
	{
		//ColorIndexes[0] = background color index for mSystemPalette
		//ColorIndexes[1 to 3] = color indexes for mSystemPalette
		ubyte ColorIndexes[4];
	};
	//The OAM contains sprite data, 4 bytes per sprite, that describe the (x, y) position and pattern tile index and other flags
	struct Sprite
	{
		ubyte PosYTop;
		ubyte TileIndex;
		ubyte Attributes;
		ubyte PosXLeft;
	};

	/*Rendering*/
	unsigned int mCurrentScanLine = 0;
	unsigned int mCurrentCycle = 0;
	//Render background, Background Info: "https://austinmorlan.com/posts/nes_rendering_overview/", "https://www.nesdev.org/wiki/Blargg_PPU", "https://www.nesdev.org/wiki/PPU_registers", "https://www.nesdev.org/wiki/PPU_nametables", "https://www.nesdev.org/wiki/PPU_pattern_tables"
	void RenderBackground();
	//Render Sprites, Source: "https://famicom.party/book/10-spritegraphics/"
	void RenderSprites();
	void RenderSliver(const unsigned int pixel_xStart, const unsigned int pixel_y, const ubyte patternLow, const ubyte patternHigh, const SubPalette& subPalette);

	/* Helper Functions */
	bool isVBLANK()
	{
		return IsBitOn<7>(mPPUSTATUS);
	}
	bool createNMIOnVBLANK()
	{
		return IsBitOn<7>(mPPUCTRL);
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

	//Object Attribute Memory
	ubyte mOAM[256u] = { 0 };
	//Pallette
	ubyte mPaletteRAM[32u] = { 0 };
	ubyte mGreyPalette[4u] = { 15u, 32u, 16u, 0u };//black, white, grey, dark grey
	const Color mSystemPalette[64u] =
	{
		//{r,g,b,a}
		{0x7C,0x7C,0x7C,0xFF},
		{0x00,0x00,0xFC,0xFF},
		{0x00,0x00,0xBC,0xFF},
		{0x44,0x28,0xBC,0xFF},
		{0x94,0x00,0x84,0xFF},
		{0xA8,0x00,0x20,0xFF},
		{0xA8,0x10,0x00,0xFF},
		{0x88,0x14,0x00,0xFF},
		{0x50,0x30,0x00,0xFF},
		{0x00,0x78,0x00,0xFF},
		{0x00,0x68,0x00,0xFF},
		{0x00,0x58,0x00,0xFF},
		{0x00,0x40,0x58,0xFF},
		{0x00,0x00,0x00,0xFF},
		{0x00,0x00,0x00,0xFF},
		{0x00,0x00,0x00,0xFF},
		{0xBC,0xBC,0xBC,0xFF},
		{0x00,0x78,0xF8,0xFF},
		{0x00,0x58,0xF8,0xFF},
		{0x68,0x44,0xFC,0xFF},
		{0xD8,0x00,0xCC,0xFF},
		{0xE4,0x00,0x58,0xFF},
		{0xF8,0x38,0x00,0xFF},
		{0xE4,0x5C,0x10,0xFF},
		{0xAC,0x7C,0x00,0xFF},
		{0x00,0xB8,0x00,0xFF},
		{0x00,0xA8,0x00,0xFF},
		{0x00,0xA8,0x44,0xFF},
		{0x00,0x88,0x88,0xFF},
		{0x00,0x00,0x00,0xFF},
		{0x00,0x00,0x00,0xFF},
		{0x00,0x00,0x00,0xFF},
		{0xF8,0xF8,0xF8,0xFF},
		{0x3C,0xBC,0xFC,0xFF},
		{0x68,0x88,0xFC,0xFF},
		{0x98,0x78,0xF8,0xFF},
		{0xF8,0x78,0xF8,0xFF},
		{0xF8,0x58,0x98,0xFF},
		{0xF8,0x78,0x58,0xFF},
		{0xFC,0xA0,0x44,0xFF},
		{0xF8,0xB8,0x00,0xFF},
		{0xB8,0xF8,0x18,0xFF},
		{0x58,0xD8,0x54,0xFF},
		{0x58,0xF8,0x98,0xFF},
		{0x00,0xE8,0xD8,0xFF},
		{0x78,0x78,0x78,0xFF},
		{0x00,0x00,0x00,0xFF},
		{0x00,0x00,0x00,0xFF},
		{0xFC,0xFC,0xFC,0xFF},
		{0xA4,0xE4,0xFC,0xFF},
		{0xB8,0xB8,0xF8,0xFF},
		{0xD8,0xB8,0xF8,0xFF},
		{0xF8,0xB8,0xF8,0xFF},
		{0xF8,0xA4,0xC0,0xFF},
		{0xF0,0xD0,0xB0,0xFF},
		{0xFC,0xE0,0xA8,0xFF},
		{0xF8,0xD8,0x78,0xFF},
		{0xD8,0xF8,0x78,0xFF},
		{0xB8,0xF8,0xB8,0xFF},
		{0xB8,0xF8,0xD8,0xFF},
		{0x00,0xFC,0xFC,0xFF},
		{0xF8,0xD8,0xF8,0xFF},
		{0x00,0x00,0x00,0xFF},
		{0x00,0x00,0x00,0xFF}
	};
		
};
#pragma once
#include "CommonTypes.h"
#include <array>

class PPU_2C02
{
public:
	PPU_2C02(class BUS& bus)
		: Bus(bus)
	{}
	ubyte ReadRegister(ubyte index);
	void WriteRegister(ubyte val, ubyte index);
	ubyte PaletteRead(ubyte2 index);
private:
	BUS& Bus;
	ubyte Read(ubyte2 address);
	void Write(ubyte val, ubyte2 address);
	/* PPU Registers */
	//Source: "https://www.nesdev.org/wiki/PPU_registers"
	enum Registers : ubyte
	{
		PPUCTRL   = 0,
		PPUMASK   = 1,
		PPUSTATUS = 2,
		OAMADDR   = 3,
		OAMDATA   = 4,
		PPUSCROLL = 5,
		PPUADDR   = 6,
		PPUDATA   = 7,
	};

	ubyte InternalBusLatch = 0;
	ubyte InternalReadBuffer = 0;//Emulate internal read buffer of 2C02
	bool latchToggle = 0;//Emulate toggle used by scroll and address

	ubyte ControlRegister;
	ubyte MaskRegister;
	ubyte StatusFlags;
	ubyte ScrollX;
	ubyte ScrollY;
	ubyte AddressHigh;
	ubyte AddressLow; 

	/*Helper Functions */
	bool IsVBLANK();

	/* Palette */
	std::array<ubyte, 32u> PaletteColors;//Program selected colors from Palette
	//Internal Full Color Palette
	const std::array<ubyte, 64u> Palette = 
	{
		0x7C7C7C,
		0x0000FC,
		0x0000BC,
		0x4428BC,
		0x940084,
		0xA80020,
		0xA81000,
		0x881400,
		0x503000,
		0x007800,
		0x006800,
		0x005800,
		0x004058,
		0x000000,
		0x000000,
		0x000000,
		0xBCBCBC,
		0x0078F8,
		0x0058F8,
		0x6844FC,
		0xD800CC,
		0xE40058,
		0xF83800,
		0xE45C10,
		0xAC7C00,
		0x00B800,
		0x00A800,
		0x00A844,
		0x008888,
		0x000000,
		0x000000,
		0x000000,
		0xF8F8F8,
		0x3CBCFC,
		0x6888FC,
		0x9878F8,
		0xF878F8,
		0xF85898,
		0xF87858,
		0xFCA044,
		0xF8B800,
		0xB8F818,
		0x58D854,
		0x58F898,
		0x00E8D8,
		0x787878,
		0x000000,
		0x000000,
		0xFCFCFC,
		0xA4E4FC,
		0xB8B8F8,
		0xD8B8F8,
		0xF8B8F8,
		0xF8A4C0,
		0xF0D0B0,
		0xFCE0A8,
		0xF8D878,
		0xD8F878,
		0xB8F8B8,
		0xB8F8D8,
		0x00FCFC,
		0xF8D8F8,
		0x000000,
		0x000000
	};

	/* OAM */
	struct SpriteInfo
	{
		ubyte PosTopY;
		ubyte TileIndex;
		ubyte Attributes;
		ubyte PosLeftX;
	};
	
	std::array<ubyte, 256u> OAM;
	ubyte AddressOAM;
};


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

	ubyte ControlRegister = 0;
	ubyte MaskRegister = 0;
	ubyte StatusFlags = 0;
	ubyte ScrollX = 0;
	ubyte ScrollY = 0;
	ubyte AddressHigh = 0;
	ubyte AddressLow = 0; 

	/*Helper Functions */
	bool IsVBLANK();

	/* Palette */
	std::array<ubyte, 32u> PaletteColors = { 0 };//Program selected colors from Palette
	//Internal Full Color Palette in RGBA format
	const std::array<unsigned int, 64u> Palette = 
	{
		0x7C7C7CFFu,
		0x0000FCFFu,
		0x0000BCFFu,
		0x4428BCFFu,
		0x940084FFu,
		0xA80020FFu,
		0xA81000FFu,
		0x881400FFu,
		0x503000FFu,
		0x007800FFu,
		0x006800FFu,
		0x005800FFu,
		0x004058FFu,
		0x000000FFu,
		0x000000FFu,
		0x000000FFu,
		0xBCBCBCFFu,
		0x0078F8FFu,
		0x0058F8FFu,
		0x6844FCFFu,
		0xD800CCFFu,
		0xE40058FFu,
		0xF83800FFu,
		0xE45C10FFu,
		0xAC7C00FFu,
		0x00B800FFu,
		0x00A800FFu,
		0x00A844FFu,
		0x008888FFu,
		0x000000FFu,
		0x000000FFu,
		0x000000FFu,
		0xF8F8F8FFu,
		0x3CBCFCFFu,
		0x6888FCFFu,
		0x9878F8FFu,
		0xF878F8FFu,
		0xF85898FFu,
		0xF87858FFu,
		0xFCA044FFu,
		0xF8B800FFu,
		0xB8F818FFu,
		0x58D854FFu,
		0x58F898FFu,
		0x00E8D8FFu,
		0x787878FFu,
		0x000000FFu,
		0x000000FFu,
		0xFCFCFCFFu,
		0xA4E4FCFFu,
		0xB8B8F8FFu,
		0xD8B8F8FFu,
		0xF8B8F8FFu,
		0xF8A4C0FFu,
		0xF0D0B0FFu,
		0xFCE0A8FFu,
		0xF8D878FFu,
		0xD8F878FFu,
		0xB8F8B8FFu,
		0xB8F8D8FFu,
		0x00FCFCFFu,
		0xF8D8F8FFu,
		0x000000FFu,
		0x000000FFu
	};

	/* OAM */
	struct SpriteInfo
	{
		ubyte PosTopY;
		ubyte TileIndex;
		ubyte Attributes;
		ubyte PosLeftX;
	};
	
	std::array<ubyte, 256u> OAM = { 0 };
	ubyte AddressOAM = 0;
};


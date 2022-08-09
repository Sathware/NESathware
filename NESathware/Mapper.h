#pragma once
#include "CommonTypes.h"
#include <array>
#include <fstream>

constexpr size_t NES_CATRIDGE_SIZE = 0xbfe0u;
struct Header
{
	ubyte Type[4];
	ubyte size_PRGRom;//size in 16KB units
	ubyte size_CHRRom;//size in 8KB units
	ubyte flags6;
	ubyte flags7;
	ubyte flags8;
	ubyte flags9;
	ubyte flags10;
	ubyte Padding[5];//Not an actual variable, just padding to make sizeof(header) == 16
};



struct Mapper
{
	virtual ~Mapper() = default;
	Mapper(std::array<ubyte, NES_CATRIDGE_SIZE>& CartridgeMemory, Header& header)
		: CartridgeMemory(CartridgeMemory), header(header)
	{}
	virtual ubyte& Read(ubyte2 address) = 0;
	std::array<ubyte, NES_CATRIDGE_SIZE>& CartridgeMemory;
	Header header;
};

struct Mapper000 : public Mapper
{
	Mapper000(std::array<ubyte, NES_CATRIDGE_SIZE>& CartridgeMemory, Header& header, std::ifstream& fileStream)
		: Mapper(CartridgeMemory, header)
	{
		fileStream.read(reinterpret_cast<char*>(&CartridgeMemory.data()[0x3fe0u]), (size_t)header.size_PRGRom * 16384);//iNES mapper 0 corresponds to the program rom beginning at 0x8000 from the CPU's view, which is internally stored in catridge space at 0x3fe0u
	}
	
	ubyte& Read(ubyte2 address) override
	{
		if (address >= 0x3fe0u)//If trying to access program ROM (0x8000) -> (0x8000 - 0x4020), absolute and internal cartridge address respectively
			return CartridgeMemory.at(size_t(address - 0x3fe0u) % 0x4000 + 0x3fe0u);//Memory Mirroring
		else
			return CartridgeMemory.at(address);
	}
};
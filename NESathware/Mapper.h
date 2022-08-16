#pragma once
#include "CommonTypes.h"
#include <array>
#include <fstream>

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
	Mapper(Header& header)
		: header(header)
	{}
	virtual ubyte& Read(ubyte2 address) = 0;
	Header header;
};

//Source: "https://www.nesdev.org/wiki/NROM"
//Convert from CPU address to internal storage indexes by subtracting 0x4020 i.e. the range [0x4020, 0xffff] -> [0x0000, 0xbfdf]
struct Mapper0 : public Mapper
{
	Mapper0(Header& header, std::ifstream& fileStream)
		: Mapper(header)
	{
		fileStream.read(reinterpret_cast<char*>(&ROM.data()[0x3fe0u]), (size_t)header.size_PRGRom * 16384);//iNES mapper 0 corresponds to the program rom beginning at 0x8000 from the CPU's view, which is internally stored in catridge space at 0x3fe0u
	}
	
	ubyte& Read(ubyte2 address) override
	{
		if (address >= 0x8000u)//If trying to access program ROM (0x8000) -> (0x8000 - 0x4020), absolute and internal cartridge address respectively
			return ROM.at((size_t)(address % (header.size_PRGRom > 1 ? 0x8000 : 0x4000)) + 0x3fe0u);//Memory Mirroring according to the iNES mapper 0 specification
		else
			return ROM.at(address);
	}

	std::array<ubyte, 0xbfe0u> ROM = { 0 };
};

////Source: "https://www.nesdev.org/wiki/MMC1"
//struct Mapper1 : public Mapper
//{
//	Mapper1(Header& header, std::ifstream& fileStream)
//		:Mapper(header)
//	{
//
//	}
//
//
//};
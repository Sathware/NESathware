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
	virtual ubyte ReadCPU(ubyte2 address) const = 0;
	virtual ubyte ReadPPU(ubyte2 address) const = 0;
	virtual void WriteCPU(ubyte val, ubyte2 address) = 0;
	virtual void WritePPU(ubyte val, ubyte2 address) = 0;
	Header header;
};

//Source: "https://www.nesdev.org/wiki/NROM"
//Convert from CPU address to internal storage indexes by subtracting 0x4020 i.e. the range [0x4020, 0xffff] -> [0x0000, 0xbfdf]
struct Mapper0 : public Mapper
{
	Mapper0(Header& header, std::ifstream& fileStream)
		: Mapper(header)
	{
		//Fill PRG ROM
		fileStream.read(reinterpret_cast<char*>(PRG_ROM), (size_t)header.size_PRGRom * 0x4000u);
		//Fill CHR ROM
		fileStream.read(reinterpret_cast<char*>(CHR_ROM), (size_t)header.size_CHRRom * 0x2000u);
	}

	ubyte ReadCPU(ubyte2 address) const override
	{
		//CPU memory addresses 0x8000 - 0xffff map to PRG ROM
		assert(address >= 0x8000u && address <= 0xffff);

		//If PRG ROM is 16KB then mirror
		ubyte2 internalAddress = (address - 0x8000u) % ((ubyte2)header.size_PRGRom * 0x4000u);
		return PRG_ROM[internalAddress];
	}

	void WriteCPU(ubyte val, ubyte2 address) override 
	{
		////CPU memory addresses 0x8000 - 0xffff map to PRG ROM
		//assert(address >= 0x8000u && address <= 0xffff);

		////If PRG ROM is 16KB then mirror
		//ubyte2 internalAddress = (address - 0x8000u) % ((ubyte2)header.size_PRGRom * 0x4000u);
		//PRG_ROM[internalAddress] = val;
	}

	ubyte ReadPPU(ubyte2 address) const override
	{
		//Normally PPU memory addresses 0x0000 - 0x1fff map to CHR ROM
		assert(address < 0x2000);
		return CHR_ROM[address];
	}

	void WritePPU(ubyte val, ubyte2 address) override
	{
		////Normally PPU memory addresses 0x0000 - 0x1fff map to CHR ROM
		//assert(address < 0x2000);
		//CHR_ROM[address] = val;
	}

	//32KB PRG ROM
	ubyte PRG_ROM[0x8000u];
	//8KB CHR ROM
	ubyte CHR_ROM[0x2000u];
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
#pragma once
#include "CommonTypes.h"
#include <array>
#include <vector>
#include <fstream>
#include <functional>

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
	Mapper(Header& header, std::vector<ubyte>& PRGROM, std::vector<ubyte>& CHRROM)
		: header(header), PRGROM(PRGROM), CHRROM(CHRROM)
	{}
	virtual ubyte& ReadCPU(ubyte2 address) = 0;
	virtual void WriteCPU(ubyte val, ubyte2 address) = 0;
	virtual ubyte& ReadPPU(ubyte2 address) = 0;
	virtual void WritePPU(ubyte val, ubyte2 address) = 0;
	Header header;
	std::vector<ubyte>& PRGROM;
	std::vector<ubyte>& CHRROM;
};



//Source: "https://www.nesdev.org/wiki/NROM"
//Convert from CPU address to internal storage indexes by subtracting 0x4020 i.e. the range [0x4020, 0xffff] -> [0x0000, 0xbfdf]
struct Mapper0 : public Mapper
{
	Mapper0(Header& header, std::vector<ubyte>& PRGROM, std::vector<ubyte>& CHRROM, std::ifstream& fileStream)
		: Mapper(header, PRGROM, CHRROM)
	{
		if (isBitOn<3>(header.flags6))
		{
			VRAM.resize(0x1000);
			//Four screen mirroring
			Mirroring = [this](ubyte2 address)->ubyte&
			{
				return VRAM.at(address); 
			};
		}
		else if (isBitOn<0>(header.flags6))
		{
			VRAM.resize(0x0800);
			//Vertical mirroring
			Mirroring = [this](ubyte2 address)->ubyte&
			{
				return VRAM.at(address % 0x0800); 
			};
		}
		else
		{
			VRAM.resize(0x0800);
			//Horizontal mirroring
			Mirroring = [this](ubyte2 address)->ubyte&
			{
				return VRAM.at(((address / 0x2800) * 0x0400) | (address & 0x00ff)); 
			};
		}
	}

	ubyte& ReadCPU(ubyte2 address) override
	{
		if (address >= 0x8000u)//If trying to access program ROM (0x8000) -> (0x8000 - 0x4020), absolute and internal cartridge address respectively
			return PRGROM.at((size_t)(address % (header.size_PRGRom > 1 ? 0x8000 : 0x4000)));//Memory Mirroring according to the iNES mapper 0 specification
		else
			return PRGROM.at(address);
	}

	void WriteCPU(ubyte val, ubyte2 address)
	{
	}

	ubyte& ReadPPU(ubyte2 address)
	{
		return Mirroring(address);
	}

	void WritePPU(ubyte val, ubyte2 address)
	{
		Mirroring(address) = val;
	}

	std::function<ubyte& (ubyte2)> Mirroring;
	std::vector<ubyte> VRAM;
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
#include "Cartridge.h"
#include <fstream>
#include <cassert>

void Cartridge::Load(std::string filename)
{
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
	assert(sizeof(Header) == 16);//Sanity check

	Header header;
	std::ifstream file(filename, std::ifstream::binary);

	file.read(reinterpret_cast<char*>(&header), 16);

	//assert(std::string(reinterpret_cast<char*>(header.Type)) == "NES");

	int mapperNum = (header.flags7 & 0xF0) | (header.flags6 >> 4);

	assert(mapperNum == 0);
	mpMapper = std::make_unique<Mapper000>(Memory);
	file.read(reinterpret_cast<char*>(&Memory[0x8000]), (size_t)header.size_PRGRom * 16384);
}

ubyte& Cartridge::Read(ubyte2 address)
{
	return mpMapper->Read(address);
}

void Cartridge::Write(ubyte val, ubyte2 address)
{
	return mpMapper->Write(val, address);
}
#include "Cartridge.h"
#include "BUS.h"
#include <fstream>

Cartridge::Cartridge(BUS& bus, std::string filename)
	: bus(bus)
{
	static_assert(sizeof(Header) == 16);

	Header header;

	std::ifstream file(filename, std::ifstream::binary);
	file.exceptions(std::ifstream::badbit | std::ifstream::failbit);

	file.read(reinterpret_cast<char*>(&header), 16);

	ubyte mapperNum = (header.flags7 & 0xf0) | (header.flags6 >> 4);

	switch (mapperNum)
	{
	case 0: mpMapper = std::make_unique<Mapper0>(header, file); break;
	//case 1: mpMapper = std::make_unique<Mapper1>(header, file); break;
	}
}

ubyte& Cartridge::Read(ubyte2 address)
{
	return mpMapper->Read(address);
}

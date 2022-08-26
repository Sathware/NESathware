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

	if (isBitOn<2>(header.flags6))
		file.seekg(512, file.cur);//Skip past trainer if present

	size_t SizeInBytesPRGROM = (size_t)header.size_PRGRom * 16384;
	PRGROM.resize(SizeInBytesPRGROM);
	file.read(reinterpret_cast<char*>(PRGROM.data()), SizeInBytesPRGROM);//0x8000 is mapped to 0x3fe0 in cartridge space

	size_t SizeInBytesCHRROM = (size_t)header.size_CHRRom * 8192;
	CHRROM.resize(SizeInBytesCHRROM);
	file.read(reinterpret_cast<char*>(CHRROM.data()), SizeInBytesCHRROM);

	ubyte mapperNum = (header.flags7 & 0xf0) | (header.flags6 >> 4);

	switch (mapperNum)
	{
	case 0: mpMapper = std::make_unique<Mapper0>(header, PRGROM, CHRROM, file); break;
	//case 1: mpMapper = std::make_unique<Mapper1>(header, file); break;
	}
}

ubyte& Cartridge::ReadCPU(ubyte2 address)
{
	return mpMapper->ReadCPU(address);
}

void Cartridge::WriteCPU(ubyte val, ubyte2 address)
{
	mpMapper->WriteCPU(val, address);
}

ubyte& Cartridge::ReadPPU(ubyte2 address)
{
	return mpMapper->ReadPPU(address);
}

void Cartridge::WritePPU(ubyte val, ubyte2 address)
{
	mpMapper->WritePPU(val, address);
}

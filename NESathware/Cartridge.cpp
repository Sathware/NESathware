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

	mpMapper = std::make_unique<Mapper000>(ROM, header, file);
}

ubyte& Cartridge::Read(ubyte2 address)
{
	return mpMapper->Read(address);
}

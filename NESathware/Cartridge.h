#pragma once
#include "CommonTypes.h"
#include <array>
#include "Mapper.h"

class Cartridge
{
public:
	Cartridge(class BUS& bus, std::string filename);
	ubyte& Read(ubyte2 address);
private:
	BUS& bus;
	std::array<ubyte, 0xbfe0u> ROM = { 0 };
	std::unique_ptr<Mapper> mpMapper;
};
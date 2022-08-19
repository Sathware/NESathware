#pragma once
#include "CommonTypes.h"
#include <array>
#include "Mapper.h"

class Cartridge
{
public:
	Cartridge(class BUS& bus, std::string filename);
	ubyte& ReadCPU(ubyte2 address);
	void WriteCPU(ubyte val, ubyte2 address);
	ubyte& ReadPPU(ubyte2 address);
	void WritePPU(ubyte val, ubyte2 address);
private:
	BUS& bus;
	std::unique_ptr<Mapper> mpMapper;
	std::vector<ubyte> PRGROM;
	std::vector<ubyte> CHRROM;
};
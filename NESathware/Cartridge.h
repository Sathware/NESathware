#pragma once
#include "CommonTypes.h"
#include <string>
#include "Mapper.h"
#include <memory>

//virtual implementation of NES cartidges
class Cartridge
{
public:
	void Load(std::string filename);
	ubyte& Read(ubyte2 address);
	void Write(ubyte val, ubyte2 address);
private:
	std::unique_ptr<Mapper> mpMapper;
	ubyte Memory[0xbfe0] = { 0 };
};


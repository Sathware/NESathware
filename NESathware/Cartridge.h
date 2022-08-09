#pragma once
#include "CommonTypes.h"
#include <string>
#include "Mapper.h"
#include <memory>
#include <array>

//virtual implementation of NES cartidges
class Cartridge
{
public:
	void Load(std::string filename);
	ubyte& Read(ubyte2 address);
private:
	std::unique_ptr<Mapper> mpMapper;
	std::array<ubyte, 0xbfe0> Memory = { 0 };
};


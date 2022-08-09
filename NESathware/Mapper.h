#pragma once
#include "CommonTypes.h"
#include <cassert>
#include <array>

struct Mapper
{
	virtual ~Mapper() = default;
	virtual ubyte& Read(ubyte2 address) = 0;
};

//CRTP Pattern
template<typename T>
struct Mapper_Base : Mapper
{
	Mapper_Base(std::array<ubyte, 0xbfe0>& CartridgeMemory)
		:CartridgeMemory(CartridgeMemory)
	{}
	ubyte& Read(ubyte2 address)
	{
		assert(address >= 0 && address <=0xbfdf);

		return static_cast<T*>(this)->Read(address);
	}
	std::array<ubyte, 0xbfe0>& CartridgeMemory;
};

struct Mapper000 : public Mapper_Base<Mapper000>
{
	Mapper000(std::array<ubyte, 0xbfe0>& CartridgeMemory)
		:Mapper_Base<Mapper000>(CartridgeMemory)
	{}
	ubyte& Read(ubyte2 address)
	{
		address = (address & 0x3fff);//Mirror addresses
		return CartridgeMemory.at(address);
	}
};
#pragma once
#include "CommonTypes.h"
#include <cassert>

struct Mapper
{
	virtual ~Mapper() = default;
	virtual ubyte& Read(ubyte2 address) = 0;
	virtual void Write(ubyte val, ubyte2 address) = 0;
};

//CRTP Pattern
template<typename T>
struct Mapper_Base : Mapper
{
	Mapper_Base(ubyte* CartridgeMemory)
		:CartridgeMemory(CartridgeMemory)
	{}
	ubyte& Read(ubyte2 address)
	{
		assert(address >= 0x4020 && address <=0xffff);

		return static_cast<T*>(this)->Read(address);
	}
	void Write(ubyte val, ubyte2 address)
	{
		assert(address >= 0x4020 && address <= 0xffff);

		static_cast<T*>(this)->Write(val, address);
	}
	ubyte* CartridgeMemory;
};

struct Mapper000 : public Mapper_Base<Mapper000>
{
	Mapper000(ubyte* CartridgeMemory)
		:Mapper_Base<Mapper000>(CartridgeMemory)
	{}
	ubyte& Read(ubyte2 address)
	{
		assert(address >= 0x8000 && address <= 0xffff);
		address = (address & 0x3fff) + 0x8000;//Mirror addresses
		return CartridgeMemory[address];
	}
	void Write(ubyte val, ubyte2 address)
	{
		assert(address >= 0x8000 && address <= 0xffff);
		address = (address & 0x3fff) + 0x8000;//Mirror addresses
		CartridgeMemory[address] = val;
	}
};
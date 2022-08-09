#include "BUS.h"

ubyte& BUS::Read(ubyte2 address)
{
	if (address < 0 || address >= 65536)
		throw std::runtime_error("Out of bounds read, adress must be in [0,65536] range!");

	if (address < 0x2000)
	{
		address &= 0x07ff;//address % 0x0800, because memory is mirrored
		return RAM.at(address);//bounds checking has already been done before hand
	}
	else if (address < 0x4000)
	{
		address = (address & 0x0007) + 0x2000;//address % 0x0008, PPU registers are mirrored
	}
	else if (address < 0x4017)
	{
		//APU and I/O Registers - not implemented
		throw std::runtime_error("APU and I/O registeres not implemented yet!");
	}
	else if (address < 0x401f)
	{
		//APU and I/O functionality that is normally disabled
		throw std::runtime_error("Tried to access disabled APU & I/O functionality!");
	}
	else if (address <= 0xffff)
	{
		cartridge->Read(address - 0x4020);//Convert from CPU Memory index to Cartridge memory index
	}
}

void BUS::Write(ubyte val, ubyte2 address)
{
	Read(address) = val;
}

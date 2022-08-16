#include "BUS.h"

ubyte& BUS::Read(ubyte2 address)
{
	assert(address >= 0 && address <= 0xffff);//Sanity check
	if (address < 0x2000)
	{
		//2KB internal RAM and mirrors
		ubyte2 index = address % 0x0800u;
		return mRAM.at(index);
	}
	else if (address < 0x4000)
	{
		//PPU registers and mirrors
		ubyte2 index = address % 0x8u;
	}
	else if (address < 0x4018)
	{
		//APU and I/O registers
	}
	else if (address < 0x4020)
	{
		//Disabled
		throw std::runtime_error("Tried to read disabled APU and I/O address");
	}
	else if (address <= 0xffff)
	{
		//Cartridge space
		return mpCartridge->Read(address);
	}
}

void BUS::Write(ubyte val, ubyte2 address)
{
	//Temporary
	Read(address) = val;
}

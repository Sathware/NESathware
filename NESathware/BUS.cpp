#include "BUS.h"

ubyte BUS::ReadCPU(ubyte2 address)
{
	assert(address <= 0xffff);//Sanity check
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
		return mpCartridge->ReadCPU(address);
	}
}

void BUS::WriteCPU(ubyte val, ubyte2 address)
{
	assert(address <= 0xffff);//Sanity check
	if (address < 0x2000)
	{
		//2KB internal RAM and mirrors
		ubyte2 index = address % 0x0800u;
		mRAM.at(index) = val;
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
		mpCartridge->WriteCPU(val, address);
	}
}

ubyte BUS::ReadPPU(ubyte2 address)
{
	assert(address <= 0x3fffu);
	if (address <= 0x1fffu)
	{
		//pattern tables
	}
	else if (address <= 0x2fffu)
	{
		//nametables
	}
	else if (address <= 0x3effu)
	{
		//mirrors of 0x2000 - 0x2eff
	}
	else if (address <= 0x3f1fu)
	{
		//palette ram indexes
	}
	else if (address <= 0x3fffu)
	{
		//mirrors of 0x3f00 - 0x3f1f
	}
}

void BUS::WritePPU(ubyte val, ubyte2 address)
{
	assert(address <= 0x3fffu);
	if (address <= 0x1fffu)
	{
		//pattern tables
	}
	else if (address <= 0x2fffu)
	{
		//nametables
	}
	else if (address <= 0x3effu)
	{
		//mirrors of 0x2000 - 0x2eff
	}
	else if (address <= 0x3f1fu)
	{
		//palette ram indexes
	}
	else if (address <= 0x3fffu)
	{
		//mirrors of 0x3f00 - 0x3f1f
	}
}
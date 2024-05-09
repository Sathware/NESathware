#include "BUS.h"

ubyte BUS::ReadCPU(ubyte2 address)
{
	if (address < 0x2000u)
	{
		//2KB internal RAM and mirrors
		ubyte2 index = address % 0x0800u;
		return mRAM.at(index);
	}
	else if (address < 0x4000u)
	{
		//PPU registers and mirrors
		return mpPPU->ReadRegister(address);
	}
	else if (address < 0x4018u)
	{
		//APU and I/O registers
		if (address == 0x4016u)
			return mpController->ReadCPU();
		if (address == 0x4017u)
			return mpZapper->ReadCPU();
	}
	else if (address < 0x4020u)
	{
		//Disabled
		throw std::runtime_error("Tried to read disabled APU and I/O address");
		return 0;
	}
	else if (address <= 0xffffu)
	{
		//Cartridge space
		return mpCartridge->ReadCPU(address);
	}
}

void BUS::WriteCPU(ubyte val, ubyte2 address)
{
	if (address < 0x2000)
	{
		//2KB internal RAM and mirrors
		ubyte2 index = address % 0x0800u;
		mRAM.at(index) = val;
	}
	else if (address < 0x4000u)
	{
		//PPU registers and mirrors
		mpPPU->WriteRegister(val, address);
	}
	else if (address < 0x4018u)
	{
		//APU and I/O registers
		//if (address < 0x4004u)
		//{
		//	// Square 1
		//}
		//if (address < 0x4008u)
		//{
		//	// Square 2
		//}
		//if (address < 0x400cu)
		//{
		//	// Triangle
		//}
		//if (address < 0x4010u)
		//{
		//	// Noise
		//}
		//if (address < 0x4014u)
		//{
		//	// DMC
		//}
		
		if (address == 0x4014u)
			mpPPU->WriteOAMDMA(&mRAM[(unsigned int)val << 8u]);
		else if (address == 0x4016u)
		{
			mpController->WriteCPU(val);
			mpZapper->WriteCPU(val);
		}
		else
			mpAPU->WriteRegister(val, address);
	}
	else if (address < 0x4020u)
	{
		//Disabled
		throw std::runtime_error("Tried to read disabled APU and I/O address");
	}
	else if (address <= 0xffffu)
	{
		//Cartridge space
		mpCartridge->WriteCPU(val, address);
	}
}

ubyte BUS::ReadPPU(ubyte2 address)
{
	//mirror addresses down
	if (address > 0x3fffu)
		address %= 0x4000u;

	//Adresses 0x3f00u - 0x3effu are addresses of 0x2f00u - 0x2effu
	if (address >= 0x3f00u && address <= 0x3effu)
		address -= 0x1000u;

	if (address <= 0x1fffu)
	{
		//pattern tables
		return mpCartridge->ReadPPU(address);
	}
	else if (address <= 0x2fffu)
	{
		//nametables
		return mVRAM.at(Mirror(address));
	}
	//The rest are not neccessary, the if guard prevents accessing the nametable mirrors, and palettes are internal to the ppu
	//else if (address <= 0x3effu)
	//{
	//	//mirrors of 0x2000 - 0x2eff
	//}
	//else if (address <= 0x3f1fu)
	//{
	//	//palette ram indexes
	//}
	//else if (address <= 0x3fffu)
	//{
	//	//mirrors of 0x3f00 - 0x3f1f
	//}
}

void BUS::WritePPU(ubyte val, ubyte2 address)
{
	//mirror addresses down
	if (address > 0x3fffu)
		address %= 0x4000u;

	//Adresses 0x3f00u - 0x3effu are addresses of 0x2f00u - 0x2effu
	if (address >= 0x3f00u && address <= 0x3effu)
		address -= 0x1000u;

	if (address <= 0x1fffu)
	{
		//pattern tables
		mpCartridge->WritePPU(val, address);
	}
	else if (address <= 0x2fffu)
	{
		//nametables
		mVRAM.at(Mirror(address)) = val;
	}
}
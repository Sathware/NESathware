#include "BUS.h"

ubyte BUS::ReadCPU(ubyte2 address)
{
	if (address < 0x2000)
	{
		//2KB internal RAM and mirrors
		ubyte2 index = address % 0x0800u;
		return mRAM.at(index);
	}
	else if (address < 0x4000)
	{
		//PPU registers and mirrors
		ubyte index = address % 0x8u;
		return mpPPU->ReadRegister(index);
	}
	else if (address < 0x4018)
	{
		//APU and I/O registers
	}
	else if (address < 0x4020)
	{
		//Disabled
		throw std::runtime_error("Tried to read a disabled APU and I/O address");
		return 0;
	}
	else if (address <= 0xffff)
	{
		//Cartridge space
		return mpCartridge->ReadCPU(address);
	}
}

ubyte* BUS::GetCPUData(ubyte2 address)
{
	//0x2000 - 256
	if (address < 0x1f00)
	{
		ubyte2 index = address % 0x0800u;
		return &mRAM.at(index);
	}
	else if (address < 0x4020)
	{
		throw std::runtime_error("Tried to use invalid DMA buffer copy");
	}
	else if (address <= 0xff00)
	{
		return &mpCartridge->ReadCPU(address);
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
	else if (address < 0x4000)
	{
		//PPU registers and mirrors
		ubyte index = address % 0x8u;
		mpPPU->WriteRegister(val, index);
	}
	else if (address < 0x4018)
	{
		//APU and I/O registers
		if (address == 0x4014)
			mpPPU->WriteOAM_DMA(val);
	}
	else if (address < 0x4020)
	{
		//Disabled
		throw std::runtime_error("Tried to write to a disabled APU and I/O address");
	}
	else if (address <= 0xffff)
	{
		//Cartridge space
		mpCartridge->WriteCPU(val, address);
	}
}

ubyte BUS::ReadPPU(ubyte2 address)
{
	address %= 0x4000;//Addresses are mirrored down to the range 0x0000 to 0x3fff

	//Address 0x3000 to ox3eff are mirrors of 0x2000 to 0x2eff
	if (address >= 0x3000 && address <= 0x3eff)
		address -= 0x1000;

	if (address < 0x1000)
	{
		//Pattern table 0 in CHR ROM
		return mpCartridge->ReadPPU(address);
	}
	else if (address < 0x2000)
	{
		//pattern table 1 in CHR ROM
		return mpCartridge->ReadPPU(address);
	}
	else if (address < 0x2400)
	{
		//Nametable 0 in VRAM
		return mpCartridge->ReadPPU(address);
	}
	else if (address < 0x2800)
	{
		//Nametable 1 in VRAM
		return mpCartridge->ReadPPU(address);
	}
	else if (address < 0x2c00)
	{
		//Nametable 2 in VRAM
		return mpCartridge->ReadPPU(address);
	}
	else if (address < 0x3000)
	{
		//nametable 3 in VRAM
		return mpCartridge->ReadPPU(address);
	}
	else if (address < 0x4000)
	{
		//Palette RAM indexes 0x3f00 to 0x3f1f in PPU itself
		ubyte2 index = address % 32u;//mirror 0x3f20-0x3fff to 0x3f00-0x3f1f
		return mpPPU->PaletteRead(index);
	}
}

ubyte* BUS::GetPPUData(ubyte2 address)
{
	return &mpCartridge->ReadPPU(address);
}

void BUS::WritePPU(ubyte val, ubyte2 address)
{
	address %= 0x4000;//Addresses are mirrored down to the range 0x0000 to 0x3fff

	//Address 0x3000 to ox3eff are mirrors of 0x2000 to 0x2eff
	if (address >= 0x3000 && address <= 0x3eff)
		address -= 0x1000;

	if (address < 0x1000)
	{
		//Pattern table 0 in CHR ROM
	}
	else if (address < 0x2000)
	{
		//pattern table 1 in CHR ROM
	}
	else if (address < 0x2400)
	{
		//Nametable 0 in VRAM
		mpCartridge->WritePPU(val, address);
	}
	else if (address < 0x2800)
	{
		//Nametable 1 in VRAM
		mpCartridge->WritePPU(val, address);
	}
	else if (address < 0x2c00)
	{
		//Nametable 2 in VRAM
		mpCartridge->WritePPU(val, address);
	}
	else if (address < 0x3000)
	{
		//nametable 3 in VRAM
		mpCartridge->WritePPU(val, address);
	}
	else if (address < 0x4000)
	{
		////Palette RAM indexes 0x3f00 to 0x3f1f in PPU itself
		//ubyte2 index = address % 32u;//mirror 0x3f20-0x3fff to 0x3f00-0x3f1f
		//return mpPPU->PaletteRead(index);
	}
}

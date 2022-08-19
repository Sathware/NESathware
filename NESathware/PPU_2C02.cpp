#include "PPU_2C02.h"
#include "BUS.h"

ubyte PPU_2C02::Read(ubyte2 address)
{
	return Bus.ReadPPU(address);
}

ubyte PPU_2C02::PaletteRead(ubyte2 index)
{
	//Palette reads immediately fill the bus, bypassing the internal read buffer
	InternalBusLatch = FramePalette.at(index);
	return InternalBusLatch;
}

void PPU_2C02::WriteOAM_DMA(ubyte highByte)
{
	ubyte2 startAddress = CombineBytes(highByte, 0x0000);
	memcpy(Bus.GetCPUData(startAddress), OAM.data(), 256u);
}

void PPU_2C02::Write(ubyte val, ubyte2 address)
{
	return Bus.WritePPU(val, address);
}

bool PPU_2C02::IsVBLANK()
{
	return GetMSB(StatusFlags);
}

ubyte PPU_2C02::ReadRegister(ubyte index)
{
	//Trying to read a nominally write only register returns the internal address latch contents of the PPU
	//Reading any nominally read only register fills the address latch with the bits read
	switch (index)
	{
	case PPUSTATUS:
	{
		InternalBusLatch = StatusFlags;
		StatusFlags &= 0x7f;//clear vblank bit
		latchToggle = 0;//reset latch toggle
		return InternalBusLatch;
	}
	case OAMDATA:
	{
		InternalBusLatch = OAM[AddressOAM];
		if (!IsVBLANK())
			++AddressOAM;
		return InternalBusLatch;
	}
	case PPUDATA:
	{
		ubyte2 AddressPPU = CombineBytes(AddressHigh, AddressLow);
		InternalBusLatch = InternalReadBuffer;
		InternalReadBuffer = Read(AddressPPU);
		AddressPPU += isBitOn<2>(ControlRegister) ? 32 : 1;//Increment address based on ControlRegister
		AddressHigh = HighByte(AddressPPU);
		AddressLow = LowByte(AddressPPU);
		return InternalBusLatch;
	}
	default:
	{
		return InternalBusLatch;
	}
	}
}

void PPU_2C02::WriteRegister(ubyte val, ubyte index)
{
	InternalBusLatch = val;
	switch (index)
	{
	case PPUCTRL:
	{
		ControlRegister = val;
		return;
	}
	case PPUMASK:
	{
		MaskRegister = val;
		return;
	}
	case PPUSTATUS:
		return;
	case OAMADDR:
	{
		AddressOAM = val;
	}
	case OAMDATA:
	{
		OAM[AddressOAM] = val;
		++AddressOAM;
	}
	case PPUSCROLL:
	{
		latchToggle ? ScrollY = val : ScrollX = val;
		latchToggle ^= 1;
		return;
	}
	case PPUADDR:
	{
		latchToggle ? AddressLow = val : AddressHigh = val;
		latchToggle ^= 1;
		return;
	}
	case PPUDATA:
	{
		ubyte2 AddressPPU = CombineBytes(AddressHigh, AddressLow);
		Write(val, AddressPPU);
		AddressPPU += isBitOn<2>(ControlRegister) ? 32 : 1;
		AddressHigh = HighByte(AddressPPU);
		AddressLow = LowByte(AddressPPU);
	}
	}
}

int PPU_2C02::Render()
{
	struct Tile
	{
		//Low 8 bytes
		ubyte LowBytes[8u];
		//High 8 bytes
		ubyte HighBytes[8u];
	};
	const ubyte* NameTable = Bus.GetPPUData((ControlRegister & 0x03u) * 0x0400u + 0x2000u);
	const Tile* PatternTable = reinterpret_cast<Tile*>(Bus.GetPPUData(isBitOn<4>(ControlRegister) ? 0x1000u : 0x0000u));
	
	for (ubyte y = 0; y <= 0x1d; ++y)
		for (ubyte x = 0; x <= 0x1f; ++x)
		{
			ubyte PatternTableIndex = NameTable[y * 30 + x];
			
			const Tile& tile = PatternTable[PatternTableIndex];

			ubyte Pattern[8u] = { 0 };
			for (ubyte i = 0; i < 8u; ++i)
				Pattern[i] = tile.LowBytes[i] | tile.HighBytes[i];
		}

	StatusFlags |= 0x80;//Set VBLANK bit

	return 0;
}

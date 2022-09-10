#include "PPU_2C02.h"
#include "BUS.h"

PPU_2C02::PPU_2C02(BUS& bus, Graphics& gfx)
	: bus(bus), gfx(gfx)
{}

void PPU_2C02::Execute()
{
	if (mCurrentScanLine == 240u && mCurrentCycle == 0)
	{
		//Create NMI and set VBLANK bit
		mPPUSTATUS |= 0x80;
		if (createNMIOnVBLANK())
			bus.InvokeNMI();

		RenderBackground();
		gfx.Render();
		gfx.ClearBuffer();
	}

	//Each scanline has only 340 cycles
	mCurrentCycle = (mCurrentCycle + 1) % 340u;
	//Every 340 cycles is a new scanline, so a zero cycle marks a new scanline
	if (mCurrentCycle == 0)
		mCurrentScanLine = (mCurrentScanLine + 1) % 260u;
}

ubyte PPU_2C02::ReadRegister(ubyte2 address)
{
	//Convert from CPU Memory address to PPU register index
	unsigned int regIndex = address % 0x0008;

	switch (regIndex)
	{
	case 2/*PPUSTATUS*/:
	{
		//fill bus latch
		mBusLatch = mPPUSTATUS;
		//turn off bit 7
		mPPUSTATUS &= 0x7f;
		//reset address latch
		mAddressLatch = false;
		return mBusLatch; 
	}
	case 4/*OAMDATA*/:
	{
		//fill bus latch
		mBusLatch = mOAM[mOAMADDR];
		//address isn't incremented during vblank or forced blanking
		if (!isVBLANK())
			++mOAMADDR;
		return mBusLatch;
	}
	case 7/*PPUDATA*/: 
	{
		ubyte2 PPUAddress = CombineBytes(mHighPPUADDR, mLowPPUADDR);
		//return palette data immediately else return read buffer
		if (PPUAddress % 0x4000u >= 0x3f00)
			//Mirror address down into standard range 0 - 0xfff, then handle palette mirrors
			mBusLatch = mPaletteRAM[PPUAddress % 0x0020u];
		else
		{
			mBusLatch = mReadBuffer;
			mReadBuffer = Read(PPUAddress);
		}
		//Increment Address register
		PPUAddress += IsBitOn<2>(mPPUCTRL) ? 32 : 1;
		mHighPPUADDR = HighByte(PPUAddress);
		mLowPPUADDR = LowByte(PPUAddress);
		return mBusLatch;
	}
	default: return mBusLatch;
	}
}

void PPU_2C02::WriteRegister(ubyte val, ubyte2 address)
{
	//Convert from CPU Memory address to PPU register index
	unsigned int regIndex = address % 0x0008;
	//Writing to any register fills the bus latch
	mBusLatch = val;

	switch (regIndex)
	{
	case 0/*PPUCTRL*/: { mPPUCTRL = val; return; }
	case 1/*PPUMASK*/: { mPPUMASK = val; return; }
	case 2/*PPUSTATUS*/: return;
	case 3/*OAMADDR*/: { mOAMADDR = val; return; }
	case 4/*OAMDATA*/: { mOAM[mOAMADDR] = val; ++mOAMADDR; return; }
	case 5/*PPUSCROLL*/: 
	{
		//value being written changes depending on address latch
		if (mAddressLatch)
			mYPPUSCROLL = val;
		else
			mXPPUSCROLL = val;
		mAddressLatch = !mAddressLatch;//toggle latch
		return; 
	}
	case 6/*PPUADDR*/: 
	{
		//value being written changes depending on address latch
		if (mAddressLatch)
			mLowPPUADDR = val;
		else
			mHighPPUADDR = val;
		mAddressLatch = !mAddressLatch;//toggle latch
		return;
	}
	case 7/*PPUDATA*/: 
	{
		ubyte2 PPUAddress = CombineBytes(mHighPPUADDR, mLowPPUADDR);
		//Change internal palette or VRAM depending on address
		if (PPUAddress % 0x4000u >= 0x3f00)
			//Mirror address down into standard range 0 - 0xfff, then handle palette mirrors
			mPaletteRAM[PPUAddress % 0x0020u] = val;
		else
			Write(val, PPUAddress);
		//Increment address
		PPUAddress += IsBitOn<2>(mPPUCTRL) ? 32 : 1;
		mHighPPUADDR = HighByte(PPUAddress);
		mLowPPUADDR = LowByte(PPUAddress);
		return;
	}
	}
}

void PPU_2C02::RenderBackground()
{
	//Bits 0 - 1 of PPUCTRL register gives the base nametable address
	ubyte2 baseNametableAddress = 0x2000u + 0x0400u * (mPPUCTRL & 0x03u);
	//Each nametable is 1024 bytes, and a nametable's attribute table sits at the last 64 bits
	ubyte2 attributeTableStart = baseNametableAddress + 0x3c0;
	//Bit 4 of PPUCTRL register gives the base pattern table address for background
	ubyte2 basePatternTableAddress = IsBitOn<4>(mPPUCTRL) * 0x1000;
	for (unsigned int y = 0; y < 240; ++y)
	{
		for (unsigned int x = 0; x < 256; x += 8u)
		{

			//Map screen coordinates (0 - 255, 0 - 239) to nametable coordinates (0 - 31, 0 - 29)
			ubyte2 nametableIndex = ((y / 8u) * 32u) + (x / 8u);
			//Each nametable entry is 1 byte, so the address of the pattern table index is base + nametableindex * sizeof(byte)
			ubyte2 patternTableIndex = Read(baseNametableAddress + nametableIndex);
			//Each pattern table entry (i.e. bitplane) is 16 bytes, so the address of the pattern bit plane is base + index * sizeof(bitplane)
			//The address of the low pattern byte is (bit plane address) + (tile row)
			//The address of the high pattern byte is (address of the low pattern byte) + 8
			ubyte2 tileRow = y % 8;
			ubyte patternLow = Read(basePatternTableAddress + patternTableIndex * 16 + tileRow);
			ubyte patternHigh = Read(basePatternTableAddress + patternTableIndex * 16 + tileRow + 8);

			ubyte2 attributeTableIndex = ((y / 32u) * 8u) + (x / 32u);
			//Each entry stores palette data about a 4x4 tile area, each quadrant is 2x2 tiles
			//bits 0-1 => topleft quadrant, bits 2-3 topright quadrant, bits 4-5 => bottomleft quadrant, bits 6-7 => bottomright quadrant
			ubyte palette4x4Tiles = Read(attributeTableStart + attributeTableIndex);
			//0b00 = topleft, 0b01 = topright, 0b10 = bottomleft, 0b11 = bottomright
			ubyte quadrant = (((y / 8u) % 2u) << 2u) | (x / 8u) % 2;

			ubyte subPaletteIndex = (palette4x4Tiles >> (quadrant * 2)) & (0x03);

			SubPalette& subPalette = reinterpret_cast<SubPalette*>(mPaletteRAM)[subPaletteIndex];

			RenderSliver(x, y, patternLow, patternHigh, subPalette);
		}
	}
}

void PPU_2C02::RenderSliver(unsigned int pixel_xStart, unsigned int pixel_y, ubyte patternLow, ubyte patternHigh, SubPalette& subPalette)
{
	for (unsigned int i = 0; i < 8; ++i)
	{
		ubyte subPaletteColorIndex = ((ubyte)IsBitOn(7 - i, patternHigh) << 1u) | (ubyte)IsBitOn(7 - i, patternLow);
		//background color indexes in subpalette are mirrors of background index in subpalette 0
		ubyte systemPaletteIndex = (subPaletteColorIndex == 0) ? mPaletteRAM[0] : subPalette.ColorIndexes[subPaletteColorIndex];

		gfx.PutPixel(pixel_xStart + i, pixel_y, mSystemPalette[systemPaletteIndex]);
	}
}

void PPU_2C02::DisplayCHRROM()
{
	for (unsigned int patternIndex = 0; patternIndex < 512u; ++patternIndex)
	{
		unsigned int x = (patternIndex % 32) * 8;
		unsigned int y = (patternIndex / 32) * 8;
		for (unsigned int tileRow = 0; tileRow < 8u; ++tileRow)
		{
			ubyte patternLow = Read(patternIndex * 16 + tileRow);
			ubyte patternHigh = Read(patternIndex * 16 + 8 + tileRow);
			for (unsigned int tileCol = 0; tileCol < 8u; ++tileCol)
			{
				ubyte colorIndex = ((ubyte)IsBitOn(7 - tileCol, patternHigh) << 1u) | (ubyte)IsBitOn(7 - tileCol, patternLow);
				gfx.PutPixel(x + tileCol, y + tileRow, mSystemPalette[greyPalette[colorIndex]]);
			}
		}
	}
}

ubyte PPU_2C02::Read(ubyte2 address)
{
	return bus.ReadPPU(address);
}

void PPU_2C02::Write(ubyte val, ubyte2 address)
{
	bus.WritePPU(val, address);
}

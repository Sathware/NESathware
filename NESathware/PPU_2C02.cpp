#include "PPU_2C02.h"
#include "BUS.h"

PPU_2C02::PPU_2C02(BUS& bus, Graphics& gfx)
	: bus(bus), gfx(gfx)
{}

void PPU_2C02::Execute()
{
	if (mWaitCycles > 0)
	{
		--mWaitCycles;
		return;
	}

	//scanlines 0 - 239 is when any rendering takes place
	if (mCurrentScanLine <= 239u)
	{
		//Only cycles 0 - 255 in each scanline draw pixels to the screen, one pixel per cycle
		if (mCurrentCycle <= 255u)
		{
			//Draw 8 Pixel sliver
			DrawSliver();

			//Since one pixel corresponds to one cycle, drawing 8 pixels mean 8 cycles have passed
			mCurrentCycle += 8;
			mWaitCycles += 8;
			return;
		}
		else
		{
			//Do nothing, cycles do garbage reads, no rendering is done
		}
	}
	else if (mCurrentScanLine == 240u && mCurrentCycle == 0)
	{
		//Create NMI and set VBLANK bit
		mPPUSTATUS |= 0x80;
		bus.InvokeNMI();
		gfx.Render();
		gfx.ClearBuffer();
	}
	else
	{
		//Do nothing, PPU is in VBLANK
	}

	//Each scanline has only 340 cycles
	mCurrentCycle = (mCurrentCycle + 1) % 340u;
	//Every 340 cycles is a new scanline, so a zero cycle marks a new scanline
	if (mCurrentCycle == 0)
		mCurrentScanLine = (mCurrentScanLine + 1) % 260u;

	mWaitCycles += 1;
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
			mBusLatch = mPalette[PPUAddress % 0x0020u];
		else
		{
			mBusLatch = mReadBuffer;
			mReadBuffer = Read(PPUAddress);
		}
		//Increment Address register
		PPUAddress += isBitOn<2>(mPPUCTRL) ? 32 : 1;
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
			mPalette[PPUAddress % 0x0020u] = val;
		else
			Write(val, PPUAddress);
		//Increment address
		PPUAddress += isBitOn<2>(mPPUCTRL) ? 32 : 1;
		mHighPPUADDR = HighByte(PPUAddress);
		mLowPPUADDR = LowByte(PPUAddress);
		return;
	}
	}
}

bool IsBitOn(unsigned int bit, ubyte val)
{
	return (val & (1u << bit)) != 0u;
}

void inline PPU_2C02::DrawSliver()
{
	//If draw sliver is called mCurrentScanLine is between 0 - 239 and mCurrentCycle is between 0 - 255
	unsigned int y = mCurrentScanLine;
	unsigned int x = mCurrentCycle;

	//Map screen coordinates (0 - 255, 0 - 239) to nametable coordinates (0 - 31, 0 - 29)
	ubyte2 nametableIndex = ((y / 8u) * 32u) + (x / 8u);
	//Bits 0 - 1 of PPUCTRL register gives the base nametable address
	ubyte2 baseNametableAddress = 0x2000u + 0x0400u * (mPPUCTRL & 0x03u);
	//Each nametable entry is 1 byte, so the address of the pattern table index is base + nametableindex * sizeof(byte)
	ubyte2 patternTableIndex = Read(baseNametableAddress + nametableIndex * 1);
	//Bit 4 of PPUCTRL register gives the base pattern table address
	ubyte2 basePatternTableAddress = isBitOn<4>(mPPUCTRL) * 0x1000;

	//Each nametable is 1024 bytes, and a nametable's attribute table sits at the last 64 bits
	ubyte2 attributeTableStart = baseNametableAddress + 0x3c0;
	ubyte2 attributeTableIndex = ((y / 32u) * 8u) + (x / 32u);

	//Each pattern table entry (i.e. bitplane) is 16 bytes, so the address of the pattern bit plane is base + index * sizeof(16 bytes)
	//The address of the low pattern byte is (bit plane address) + (tile row)
	//The address of the high pattern byte is (address of the low pattern byte) + 8
	ubyte2 tileRow = y % 8;
	ubyte patternLow = Read(basePatternTableAddress + patternTableIndex * 16 + tileRow);
	ubyte patternHigh = Read(basePatternTableAddress + patternTableIndex * 16 + tileRow + 8);

	//Each entry stores palette data about a 4x4 tile area, each quadrant is 2x2 tiles
	//bits 0-1 => topleft quadrant, bits 2-3 topright quadrant, bits 4-5 => bottomleft quadrant, bits 6-7 => bottomright quadrant
	ubyte palette4x4Tiles = Read(attributeTableStart + attributeTableIndex);
	//0b00 = topleft, 0b01 = topright, 0b10 = bottomleft, 0b11 = bottomright
	ubyte quadrant = (((y / 8u) % 2u) << 2u) | (x / 8u) % 2;
	
	//A subpalette is 4 bytes long, and houses indexes into the system palette
	struct SubPalette
	{
		//ColorIndexes[0] = background color index for mSystemPalette
		//ColorIndexes[1 to 3] = color indexes for mSystemPalette
		ubyte ColorIndexes[4];
	};
	static_assert(sizeof(SubPalette) == 4);
	//background color indexes in subpalette are mirrors of background index in subpalette 0

	ubyte subPaletteIndex = (palette4x4Tiles >> (quadrant * 2)) & (0x03);

	SubPalette& subPalette = reinterpret_cast<SubPalette*>(mPalette)[subPaletteIndex];

	//Draw sliver
	for (unsigned int i = 0; i < 8; ++i)
	{
		ubyte subPaletteColorIndexIndex = (2 * IsBitOn(i, patternHigh)) | IsBitOn(i, patternLow);

		ubyte systemPaletteIndex = subPaletteColorIndexIndex == 0 ? mPalette[0] : subPalette.ColorIndexes[subPaletteColorIndexIndex];

		Color c = mSystemPalette[systemPaletteIndex];

		gfx.PutPixel(x + i, y, c);
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

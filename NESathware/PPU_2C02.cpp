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

	if (mCurrentScanLine == 240u && mCurrentCycle == 0)
	{
		//Render background (also
		RenderBackground();
		gfx.Render();
		gfx.ClearBuffer();

		//Create NMI and set VBLANK bit
		mPPUSTATUS |= 0x80;
		bus.InvokeNMI();
	}

	//Each scanline has only 340 cycles
	mCurrentCycle = (mCurrentCycle + 1) % 340u;
	//Every 340 cycles is a new scanline, so a zero cycle marks a new scanline
	if (mCurrentCycle == 0)
		mCurrentScanLine = (mCurrentScanLine + 1) % 260u;

	mWaitCycles += 1;
}

//TODO---------------------------------- Reading Palette data shoudl fill bus latch with mirrored nametable data "underneath" it
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

void PPU_2C02::WriteOAMDMA(ubyte* dataBuffer)
{
	memcpy(mOAM, dataBuffer, 256u);
}

static bool IsBitOn(ubyte bit, ubyte val)
{
	return (val & (1u << bit)) != 0u;
}

void PPU_2C02::RenderBackground()
{
	//A subpalette is 4 bytes long, and houses indexes into the system palette
	struct SubPalette
	{
		//ColorIndexes[0] = background color index for mSystemPalette
		//ColorIndexes[1 to 3] = color indexes for mSystemPalette
		ubyte ColorIndexes[4];
	};
	static_assert(sizeof(SubPalette) == 4);

	//Bits 0 - 1 of PPUCTRL register gives the base nametable address
	ubyte2 baseNametableAddress = 0x2000u + 0x0400u * (mPPUCTRL & 0x03u);
	//Each nametable is 1024 bytes, and a nametable's attribute table sits at the last 64 bits
	ubyte2 attributeTableAddress = baseNametableAddress + 0x3c0;
	//Bit 4 of PPUCTRL register gives the base pattern table address
	ubyte2 basePatternTableAddress = isBitOn<4>(mPPUCTRL) * 0x1000;

	for (ubyte2 nametableIndex = 0; nametableIndex < 960; ++nametableIndex)
	{
		//2D tile space coordinates
		ubyte tileX = nametableIndex % 32; ubyte tileY = nametableIndex / 32;
		//Each nametable entry is 1 byte, so the address of the pattern table index is base + nametableindex * sizeof(byte)
		ubyte2 patternTableIndex = Read(baseNametableAddress + nametableIndex * sizeof(byte));
		//Get corresponding attribute table index for nametable index
		ubyte2 attributeTableIndex = (tileX / 4) + (tileY / 4) * 8;
		//Each entry stores palette data about a 4x4 tile area, each quadrant is 2x2 tiles
		//bits 0-1 => topleft quadrant, bits 2-3 topright quadrant, bits 4-5 => bottomleft quadrant, bits 6-7 => bottomright quadrant
		ubyte palette4x4Tiles = Read(attributeTableAddress + attributeTableIndex);
		//Draw Tile 8x8 pixels
		for (ubyte tileRow = 0; tileRow < 8; ++tileRow)
		{
			//Each pattern table entry (i.e. bitplane) is 16 bytes, so the address of the pattern bit plane is base + index * sizeof(16 bytes)
			//The address of the low pattern byte is (bit plane address) + (tile row)
			//The address of the high pattern byte is (address of the low pattern byte) + 8
			ubyte patternLow = Read(basePatternTableAddress + patternTableIndex * 16 + tileRow);
			ubyte patternHigh = Read(basePatternTableAddress + patternTableIndex * 16 + tileRow + 8);

			//0b00 = topleft, 0b01 = topright, 0b10 = bottomleft, 0b11 = bottomright
			ubyte quadrant = ((tileY % 2u) << 1u) | (tileX % 2);
			//Get sub palette index from attribute table byte
			ubyte subPaletteIndex = (palette4x4Tiles >> (quadrant * 2)) & (0x03);

			SubPalette& subPalette = reinterpret_cast<SubPalette*>(mPalette)[subPaletteIndex];

			//Draw 8 pixel Sliver
			for (ubyte tileCol = 0; tileCol < 8; ++tileCol)
			{
				ubyte subPaletteColorIndexIndex = ((ubyte)IsBitOn(tileCol, patternHigh) << 1u) | (ubyte)IsBitOn(tileCol, patternLow);
				//background color indexes in all subpalette are mirrors of background index in subpalette 0
				ubyte systemPaletteIndex = (subPaletteColorIndexIndex == 0) ? mPalette[0] : subPalette.ColorIndexes[subPaletteColorIndexIndex];

				gfx.PutPixel(tileX*8 + tileCol, tileY*8 + tileRow, mSystemPalette[systemPaletteIndex]);
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

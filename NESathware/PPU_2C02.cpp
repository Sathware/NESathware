#include "PPU_2C02.h"
#include "BUS.h"

PPU_2C02::PPU_2C02(BUS& bus, Graphics& gfx)
	: Bus(bus), gfx(gfx)
{}

void PPU_2C02::Execute()
{
	if (mCurrentScanLine == 240u && mCurrentCycle == 0)
	{
		//Create NMI and set VBLANK bit
		mPPUSTATUS |= 0x80;
		if (createNMIOnVBLANK())
			Bus.InvokeNMI();

		RenderBackground();
		RenderSprites();
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

void PPU_2C02::WriteOAMDMA(ubyte* data)
{
	memcpy(mOAM, data, 256u);
}

void PPU_2C02::RenderBackground()
{
	//Bits 0 - 1 of PPUCTRL register gives the base nametable address
	const ubyte2 baseNametableAddress = 0x2000u + 0x0400u * (mPPUCTRL & 0x03u);
	//Each nametable is 1024 bytes, and a nametable's attribute table sits at the last 64 bits
	const ubyte2 attributeTableStart = baseNametableAddress + 0x3c0;
	//Bit 4 of PPUCTRL register gives the base pattern table address for background
	const ubyte2 basePatternTableAddress = IsBitOn<4>(mPPUCTRL) * 0x1000;
	for (unsigned int y = 0; y < 240; ++y)
	{
		for (unsigned int x = 0; x < 256; x += 8u)
		{
			//Map screen coordinates (0 - 255, 0 - 239) to nametable coordinates (0 - 31, 0 - 29)
			const ubyte2 nametableIndex = ((y / 8u) * 32u) + (x / 8u);
			//Each nametable entry is 1 byte, so the address of the pattern table index is base + nametableindex * sizeof(byte)
			const ubyte2 patternTableIndex = Read(baseNametableAddress + nametableIndex);
			//Each pattern table entry (i.e. bitplane) is 16 bytes, so the address of the pattern bit plane is base + index * sizeof(bitplane)
			//The address of the low pattern byte is (bit plane address) + (tile row)
			//The address of the high pattern byte is (address of the low pattern byte) + 8
			const ubyte2 tileRow = y % 8u;
			const ubyte patternLow = Read(basePatternTableAddress + patternTableIndex * 16u + tileRow);
			const ubyte patternHigh = Read(basePatternTableAddress + patternTableIndex * 16u + tileRow + 8u);

			const ubyte2 attributeTableIndex = ((y / 32u) * 8u) + (x / 32u);
			//Each entry stores palette data about a 4x4 tile area, each quadrant is 2x2 tiles
			//bits 0-1 => topleft quadrant, bits 2-3 topright quadrant, bits 4-5 => bottomleft quadrant, bits 6-7 => bottomright quadrant
			const ubyte palette4x4Tiles = Read(attributeTableStart + attributeTableIndex);
			//0b00 = topleft, 0b01 = topright, 0b10 = bottomleft, 0b11 = bottomright
			const ubyte quadrant = (((y / 16u) % 2u) << 1u) | (x / 16u) % 2u;
			//Calculate mPaletteRam index by getting appropriate quadrant values from attribute table data (i.e. palette4x4tiles)
			const ubyte subPaletteIndex = (palette4x4Tiles >> (quadrant * 2u)) & (0x03);

			const SubPalette& subPalette = reinterpret_cast<SubPalette*>(mPaletteRAM)[subPaletteIndex];

			for (unsigned int tileCol = 0; tileCol < 8; ++tileCol)
			{
				ubyte subPaletteColorIndex = ((ubyte)IsBitOn(7 - tileCol, patternHigh) << 1u) | (ubyte)IsBitOn(7 - tileCol, patternLow);
				//background color indexes in subpalette are mirrors of background index in subpalette 0
				ubyte systemPaletteIndex = (subPaletteColorIndex == 0) ? mPaletteRAM[0] : subPalette.ColorIndexes[subPaletteColorIndex];

				gfx.PutPixel(x + tileCol, y, mSystemPalette[systemPaletteIndex]);
			}
		}
	}
}

void PPU_2C02::RenderSprites()
{
	//OAM data as Sprite array
	const Sprite* const sprites = reinterpret_cast<Sprite*>(&mOAM[mOAMADDR]);
	//Base pattern table address
	const ubyte2 basePatternTableAddress = IsBitOn<3>(mPPUCTRL) * 0x1000;
	//There is a maximum of 64 sprites on the screen
	for (unsigned int i = 0; i < (64 - mOAMADDR / 4u); ++i)
	{
		//Skip if sprite is overbounds, TODO - Jusr display part of sprite that is not overbounds
		if (sprites[i].PosYTop > 232u)
			continue;

		//flip horizontally flag
		const bool hFlip = IsBitOn<6>(sprites[i].Attributes);
		//flip vertically flag
		const bool vFlip = IsBitOn<7>(sprites[i].Attributes);

		//if vFlip is set, tileRow will gor from 7 to 0 else tileCol will go from 0 to 7
		const int rowEnd = (vFlip ? -1 : 8);
		const int deltaRow = (vFlip ? -1 : 1);
		for (int tileRow = (vFlip ? 7 : 0), yOffset = 0; tileRow != rowEnd; tileRow += deltaRow, ++yOffset)
		{
			const ubyte patternLow = Read(basePatternTableAddress + sprites[i].TileIndex * 16u + tileRow);
			const ubyte patternHigh = Read(basePatternTableAddress + sprites[i].TileIndex * 16u + tileRow + 8u);
			
			//bits 0-1 of sprite attributes contain sub palette index, and sprite sub palettes start at 16 bytes into palette ram
			const SubPalette& subPalette = reinterpret_cast<SubPalette*>(&mPaletteRAM[16u])[sprites[i].Attributes & 0x03];

			//if hFlip is set, tileCol will go from 7 to 0 else tileCol will go from 0 to 7
			const int colEnd = (hFlip ? -1:8);
			const int deltaCol = (hFlip ? -1:1);
			for (int tileCol = (hFlip ? 7:0), xOffset = 0; tileCol != colEnd; tileCol += deltaCol, ++xOffset)
			{
				ubyte subPaletteColorIndex = ((ubyte)IsBitOn(7 - tileCol, patternHigh) << 1u) | (ubyte)IsBitOn(7 - tileCol, patternLow);
				//background color indexes in subpalette are mirrors of background index in subpalette 0
				if (subPaletteColorIndex != 0)
				{
					ubyte systemPaletteIndex = subPalette.ColorIndexes[subPaletteColorIndex];
					gfx.PutPixel(sprites[i].PosXLeft + xOffset, sprites[i].PosYTop + yOffset, mSystemPalette[systemPaletteIndex]);
				}
			}
		}
	}
}

void PPU_2C02::RenderSliver(const unsigned int pixel_xStart, const unsigned int pixel_y, const ubyte patternLow, const ubyte patternHigh, const SubPalette& subPalette)
{
	for (unsigned int tileCol = 0; tileCol < 8; ++tileCol)
	{
		ubyte subPaletteColorIndex = ((ubyte)IsBitOn(7 - tileCol, patternHigh) << 1u) | (ubyte)IsBitOn(7 - tileCol, patternLow);
		//background color indexes in subpalette are mirrors of background index in subpalette 0
		ubyte systemPaletteIndex = (subPaletteColorIndex == 0) ? mPaletteRAM[0] : subPalette.ColorIndexes[subPaletteColorIndex];

		gfx.PutPixel(pixel_xStart + tileCol, pixel_y, mSystemPalette[systemPaletteIndex]);
	}
}

void PPU_2C02::DisplayCHRROM()
{
	for (unsigned int patternIndex = 0x0u; patternIndex < 0x200u; ++patternIndex)
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
				gfx.PutPixel(x + tileCol, y + tileRow, mSystemPalette[mGreyPalette[colorIndex]]);
			}
		}
	}
}

ubyte PPU_2C02::Read(ubyte2 address)
{
	return Bus.ReadPPU(address);
}

void PPU_2C02::Write(ubyte val, ubyte2 address)
{
	Bus.WritePPU(val, address);
}

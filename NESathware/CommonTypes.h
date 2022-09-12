#pragma once
#include <stdexcept>
#include <cassert>

typedef __int8 sbyte;
typedef __int16 sbyte2;

typedef unsigned __int8 ubyte;
typedef unsigned __int16 ubyte2;

template<ubyte bit>
static bool IsBitOn(const ubyte2 data)
{
	return (data & (1 << bit)) != 0;
}

static bool IsBitOn(unsigned int bit, ubyte val)
{
	return (val & (1u << bit)) != 0u;
}

//get most significant bit, or sign bit in two's complement representation
static bool GetMSB(const ubyte data)
{
	return (data >> 7);
}

//Return lower byte of byte2 e.g. Low(0xAABB) = 0xBB
static ubyte LowByte(const ubyte2 val)
{
	return (ubyte)(val & 0x00FF);
}

//Return higher byte byte2 e.g. High(0xAABB) = 0xAA
static ubyte HighByte(const ubyte2 val)
{
	return (ubyte)(val >> 8);
}

static ubyte2 CombineBytes(const ubyte2 high, const ubyte2 low)
{
	return (high << 8) | low;
}
#pragma once
#include <stdexcept>
#include <cassert>

typedef __int8 byte;
typedef __int16 byte2;

typedef unsigned __int8 ubyte;
typedef unsigned __int16 ubyte2;

template<ubyte bit>
static bool isBitOn(const ubyte2 data)
{
	return (data & (1 << bit)) != 0;
}

//get most significant bit, or sign bit in two's complement representation
static bool GetMSB(const ubyte data)
{
	return (data >> 7);
}

//Return lower byte of byte2 e.g. Low(0xAABB) = 0xBB
static ubyte Low(const ubyte2 val)
{
	return (ubyte)(val & 0x00FF);
}

//Return higher byte byte2 e.g. High(0xAABB) = 0xAA
static ubyte High(const ubyte2 val)
{
	return (ubyte)(val >> 8);
}

static ubyte2 Combine(const ubyte2 high, const ubyte2 low)
{
	return (high << 8) | low;
}
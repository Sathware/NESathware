#pragma once
#include <assert.h>
#include "SathwareEngine.h"

struct SathwareAPI Color
{
	static const Color White;
	static const Color Red;
	static const Color Blue;
	static const Color Green;

	//Endianness can cause color values to be flipped, so use reinterpret cast to be more portable
	//Source: In Remarks Section "https://docs.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format"
	unsigned __int32 rgba;

	constexpr Color()
		: rgba(0)
	{}

	Color(unsigned __int8 r, unsigned __int8 g, unsigned __int8 b, unsigned __int8 a)
	{
		reinterpret_cast<__int8*>(&rgba)[0] = r;
		reinterpret_cast<__int8*>(&rgba)[1] = g;
		reinterpret_cast<__int8*>(&rgba)[2] = b;
		reinterpret_cast<__int8*>(&rgba)[3] = a;
	}

	void SetR(unsigned __int8 r)
	{
		reinterpret_cast<__int8*>(&rgba)[0] = r;
		//rgba |= r;
	}
	void SetG(unsigned __int8 g)
	{
		reinterpret_cast<__int8*>(&rgba)[1] = g;
		//rgba |= (g << 8);
	}
	void SetB(unsigned __int8 b)
	{
		reinterpret_cast<__int8*>(&rgba)[2] = b;
		//rgba |= (b << 16);
	}
	void SetA(unsigned __int8 a)
	{
		reinterpret_cast<__int8*>(&rgba)[3] = a;
		//rgba |= (a << 24);
	}
};
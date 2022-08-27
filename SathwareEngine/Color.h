#pragma once
#include <assert.h>
#include "SathwareEngine.h"

struct SathwareAPI Color
{
	//Endianness can cause color values to be flipped, so use reinterpret cast to be more portable
	//Source: In Remarks Section "https://docs.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format"
	unsigned __int32 rgba;

	Color()
		:Color(0, 0, 0, 255)
	{}

	Color(unsigned __int8 r, unsigned __int8 g, unsigned __int8 b, unsigned __int8 a)
	{
		SetR(r);
		SetG(g);
		SetB(b);
		SetA(a);
	}

	void SetR(unsigned __int8 r)
	{
		assert(r >= 0 && r <= 255);
		reinterpret_cast<__int8*>(&rgba)[0] = r;
		//rgba |= r;
	}
	void SetG(unsigned __int8 g)
	{
		assert(g >= 0 && g <= 255);
		reinterpret_cast<__int8*>(&rgba)[1] = g;
		//rgba |= (g << 8);
	}
	void SetB(unsigned __int8 b)
	{
		assert(b >= 0 && b <= 255);
		reinterpret_cast<__int8*>(&rgba)[2] = b;
		//rgba |= (b << 16);
	}
	void SetA(unsigned __int8 a)
	{
		assert(a >= 0 && a <= 255);
		reinterpret_cast<__int8*>(&rgba)[3] = a;
		//rgba |= (a << 24);
	}
};
#pragma once
#include "../SathwareEngine/Graphics.h"

class PPU_2C02
{
public:
	PPU_2C02(class BUS& bus, Graphics& gfx)
		: bus(bus), gfx(gfx)
	{}
private:
	BUS& bus;
	Graphics& gfx;
};


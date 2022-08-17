#pragma once
class PPU_2C02
{
public:
	PPU_2C02(class BUS& bus)
		: bus(bus)
	{}
private:
	BUS& bus;
};


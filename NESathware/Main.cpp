#include <iostream>
#include "CommonTypes.h"
#include "BUS.h"
#include "CPU_6052.h"
#include "Cartridge.h"
#include "PPU_2C02.h"
#include "APU_2A03.h"

int main()
{
	BUS bus;
	Cartridge cartridge(bus, "nestest.nes");
	CPU_6052 Cpu(bus, 0xc000);

	bus.mpCartridge = &cartridge;
	bus.mpCPU = &Cpu;

	while (true)
	{
		Cpu.Execute();
	}
}
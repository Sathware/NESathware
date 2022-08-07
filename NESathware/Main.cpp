#include <iostream>
#include "CommonTypes.h"
#include "CPU_6052.h"
#include "BUS.h"
#include <string>
#include <fstream>
#include <cassert>

int main()
{
	BUS bus;

	Cartridge cartridge;
	cartridge.Load("nestest.nes");
	bus.cartridge = &cartridge;

	//LoadProgram("nestest.nes", bus);
	//ubyte Program[] = {0xA2,0xFF,0x9A,0xA0,0x04,0xA9,0x00,0xEE,0x02,0x00,0x88,0xD0,0xFA};
	//memcpy(&bus.RAM[0xc000], Program, sizeof(Program));
	CPU_6052 Cpu(bus, 0xc000);

	bus.Cpu = &Cpu;

	while (true)
	{
		Cpu.Execute();
	}
}
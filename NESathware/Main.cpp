#include <iostream>
#include "CommonTypes.h"
#include "CPU_6052.h"
#include "BUS.h"
#include <string>
#include <fstream>
#include <cassert>

void LoadProgram(std::string filename, BUS& bus)
{
	struct Header
	{
		ubyte Type[4];
		ubyte size_PRGRom;//size in 16KB units
		ubyte size_CHRRom;//size in 8KB units
		ubyte flags6;
		ubyte flags7;
		ubyte flags8;
		ubyte flags9;
		ubyte flags10;
		ubyte Padding[5];//Not an actual variable, just padding to make sizeof(header) == 16
	};

	Header header;

	assert(sizeof(Header) == 16);

	std::ifstream file(filename, std::ifstream::binary);

	file.read(reinterpret_cast<char*>(&header), 16);

	file.read(reinterpret_cast<char*>(&bus.RAM[0x8000]), (size_t)header.size_PRGRom * 16384);
	memcpy(reinterpret_cast<char*>(&bus.RAM[0xc000]), reinterpret_cast<char*>(&bus.RAM[0x8000]), (size_t)header.size_PRGRom * 16384);//Mirror
}

int main()
{
	BUS bus;

	LoadProgram("nestest.nes", bus);
	//ubyte Program[] = {0xA2,0xFF,0x9A,0xA0,0x04,0xA9,0x00,0xEE,0x02,0x00,0x88,0xD0,0xFA};
	//memcpy(&bus.RAM[0xc000], Program, sizeof(Program));
	CPU_6052 Cpu(bus, 0xc000);

	while (true)
	{
		/*for (int i = 0; i < 20; i++)
		{*/
			Cpu.Execute();
		//}
	}
}
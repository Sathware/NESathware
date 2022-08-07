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
	CPU_6052 Cpu(bus);

	

	Cpu.Execute();
}
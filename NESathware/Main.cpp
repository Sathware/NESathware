#include <iostream>
#include "CommonTypes.h"
#include "CPU_6052.h"
#include "BUS.h"

int main()
{
	BUS bus;
	CPU_6052 Cpu(bus);

	Cpu.Execute();

	//ubyte Accumulator = 0xfe;
	//ubyte data = 0x01;
	//bool Carry = 1;

	//byte2 temp = (byte2)(byte)Accumulator - (byte2)(byte)data - (byte2)(byte)(!Carry);

	////Set if borrowing did not occur, else clear
	//Carry = (temp & (1 << 8)) != 0;//Possible bug, Carry might not be set if least significant byte of temp == 0, i.e. accumulator - data = 0
	//bool Overflow = temp > 127 || temp < -128;
	//bool Zero = (temp & 0x00ff) == 0;
	//bool Negative = (temp & (1 << 7)) != 0;

	//Accumulator = temp & 0x00ff;
	//std::cout << std::hex << temp << std::endl;
}
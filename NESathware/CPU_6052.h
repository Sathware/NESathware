#pragma once
#include "CommonTypes.h"
#include "BUS.h"
#include <variant>

//Implementation of the 6502 8-Bit CPU
//Source: Technical overview "https://en.wikipedia.org/wiki/MOS_Technology_6502"
//More Technical Overview "https://ia803008.us.archive.org/9/items/mos_6500_mpu_preliminary_may_1976/mos_6500_mpu_preliminary_may_1976.pdf"
class CPU_6052
{
private:
	//THIS CPU IS LITTLE ENDIAN

	ubyte Accumulator;//Accumulator register
	ubyte Y_Register;//Index register
	ubyte X_Register;//Index register
	ubyte2 ProgramCounter;//Program Counter
	ubyte StackPointer;//Stack Pointer
	ubyte Status;//Flags

	enum Flag : ubyte
	{
		Carry = 0b00000001,
		Zero = 0b00000010,
		InterruptDisable = 0b00000100,
		Decimal = 0b00001000,//Decimal NOT AVAILABLE IN NES VERSION OF 6502
		Break = 0b00010000,
		Reserved = 0b00100000,//Reserved (BLANK - NEVER USED)
		Overflow = 0b01000000,
		Negative = 0b10000000
	};

	struct Instruction
	{
		char Name[4];
		std::variant<void(*)(byte&),void(*)(byte2)> Operation;
		std::variant<byte&(*)(byte&), byte2(*)(byte&)> Data;
		ubyte numCycles;
	};
};


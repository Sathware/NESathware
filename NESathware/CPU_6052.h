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
		std::variant<void(CPU_6052::*)(ubyte&),void(CPU_6052::*)(ubyte2)> Operation;
		std::variant<ubyte&(CPU_6052::*)(ubyte&), ubyte2(CPU_6052::*)(ubyte&)> Data;
		ubyte numCycles;
	};

	using c = CPU_6052;
	//Function pointer array indexed by hex opcode - source: opcode matrix -> "http://archive.6502.org/datasheets/rockwell_r650x_r651x.pdf"
	const Instruction Instructions[256] =
	{							//col,row
		{"BRK",&c::BRK,&c::IMP,7},//0,0
		{"ORA",&c::ORA,&c::IIX,6},//1,0
		{"NUL",&c::NUL,&c::ERR,0},//2,0
		{"NUL",&c::NUL,&c::ERR,0},//3,0
		{"NUL",&c::NUL,&c::ERR,0},//4,0
		{"ORA",&c::ORA,&c::ZPA,3},//5,0
		{"ASL",&c::ASL,&c::ZPA,5},//6,0
		{"NUL",&c::NUL,&c::ERR,0},//7,0
		{"PHP",&c::PHP,&c::IMP,3},//8,0
		{"ORA",&c::ORA,&c::IMM,2},//9,0
		{"ASL",&c::ASL,&c::ACA,2},//A,0
		{"NUL",&c::NUL,&c::ERR,0},//B,0
		{"NUL",&c::NUL,&c::ERR,0},//C,0
		{"ORA",&c::ORA,&c::ABS,4},//D,0
		{"ASL",&c::ASL,&c::ABS,6},//E,0
		{"NUL",&c::NUL,&c::ERR,0},//F,0
	
		{"BPL",&c::BPL,&c::REL,2},//0,1
		{"ORA",&c::ORA,&c::IIY,5},//1,1
		{"NUL",&c::NUL,&c::ERR,0},//2,1
		{"NUL",&c::NUL,&c::ERR,0},//3,1
		{"NUL",&c::NUL,&c::ERR,0},//4,1
		{"ORA",&c::ORA,&c::ZPX,4},//5,1
		{"ASL",&c::ASL,&c::ZPX,6},//6,1
		{"NUL",&c::NUL,&c::ERR,0},//7,1
		{"CLC",&c::CLC,&c::IMP,2},//8,1
		{"ORA",&c::ORA,&c::IAY,4},//9,1
		{"NUL",&c::NUL,&c::ERR,0},//A,1
		{"NUL",&c::NUL,&c::ERR,0},//B,1
		{"NUL",&c::NUL,&c::ERR,0},//C,1
		{"ORA",&c::ORA,&c::IAX,4},//D,1
		{"ASL",&c::ASL,&c::IAX,7},//E,1
		{"NUL",&c::NUL,&c::ERR,0},//F,1
	
		{"JSR",&c::JSR,&c::ABJ,6},//0,2
		{"AND",&c::AND,&c::IIX,6},//1,2
		{"NUL",&c::NUL,&c::ERR,0},//2,2
		{"NUL",&c::NUL,&c::ERR,0},//3,2
		{"BIT",&c::BIT,&c::ZPA,3},//4,2
		{"AND",&c::AND,&c::ZPA,3},//5,2
		{"ROL",&c::ROL,&c::ZPA,5},//6,2
		{"NUL",&c::NUL,&c::ERR,0},//7,2
		{"PLP",&c::PLP,&c::IMP,4},//8,2
		{"AND",&c::AND,&c::IMM,2},//9,2
		{"ROL",&c::ROL,&c::ACA,2},//A,2
		{"NUL",&c::NUL,&c::ERR,0},//B,2
		{"BIT",&c::BIT,&c::ABS,4},//C,2
		{"AND",&c::AND,&c::ABS,4},//D,2
		{"ROL",&c::ROL,&c::ABS,6},//E,2
		{"NUL",&c::NUL,&c::ERR,0},//F,2
	
		{"BMI",&c::BMI,&c::REL,2},//0,3
		{"AND",&c::AND,&c::IIY,5},//1,3
		{"NUL",&c::NUL,&c::ERR,0},//2,3
		{"NUL",&c::NUL,&c::ERR,0},//3,3
		{"NUL",&c::NUL,&c::ERR,0},//4,3
		{"AND",&c::AND,&c::ZPX,4},//5,3
		{"ROL",&c::ROL,&c::ZPX,6},//6,3
		{"NUL",&c::NUL,&c::ERR,0},//7,3
		{"SEC",&c::SEC,&c::IMP,2},//8,3
		{"AND",&c::AND,&c::IAY,4},//9,3
		{"NUL",&c::NUL,&c::ERR,0},//A,3
		{"NUL",&c::NUL,&c::ERR,0},//B,3
		{"NUL",&c::NUL,&c::ERR,0},//C,3
		{"AND",&c::AND,&c::IAX,4},//D,3
		{"ROL",&c::ROL,&c::IAX,7},//E,3
		{"NUL",&c::NUL,&c::ERR,0},//F,3
	
		{"RTI",&c::RTI,&c::IMP,6},//0,4
		{"EOR",&c::EOR,&c::IIX,6},//1,4
		{"NUL",&c::NUL,&c::ERR,0},//2,4
		{"NUL",&c::NUL,&c::ERR,0},//3,4
		{"NUL",&c::NUL,&c::ERR,0},//4,4
		{"EOR",&c::EOR,&c::ZPA,3},//5,4
		{"LSR",&c::LSR,&c::ZPA,5},//6,4
		{"NUL",&c::NUL,&c::ERR,0},//7,4
		{"PHA",&c::PHA,&c::IMP,3},//8,4
		{"EOR",&c::EOR,&c::IMM,2},//9,4
		{"LSR",&c::LSR,&c::ACA,2},//A,4
		{"NUL",&c::NUL,&c::ERR,0},//B,4
		{"JMP",&c::JMP,&c::ABJ,3},//C,4
		{"EOR",&c::EOR,&c::ABS,4},//D,4
		{"LSR",&c::LSR,&c::ABS,6},//E,4
		{"NUL",&c::NUL,&c::ERR,0},//F,4
	
		{"BVC",&c::BVC,&c::REL,2},//0,5
		{"EOR",&c::EOR,&c::IIY,5},//1,5
		{"NUL",&c::NUL,&c::ERR,0},//2,5
		{"NUL",&c::NUL,&c::ERR,0},//3,5
		{"NUL",&c::NUL,&c::ERR,0},//4,5
		{"EOR",&c::EOR,&c::ZPX,4},//5,5
		{"LSR",&c::LSR,&c::ZPX,6},//6,5
		{"NUL",&c::NUL,&c::ERR,0},//7,5
		{"CLI",&c::CLI,&c::IMP,2},//8,5
		{"EOR",&c::EOR,&c::IAY,4},//9,5
		{"NUL",&c::NUL,&c::ERR,0},//A,5
		{"NUL",&c::NUL,&c::ERR,0},//B,5
		{"NUL",&c::NUL,&c::ERR,0},//C,5
		{"EOR",&c::EOR,&c::IAX,4},//D,5
		{"LSR",&c::LSR,&c::IAX,7},//E,5
		{"NUL",&c::NUL,&c::ERR,0},//F,5
	
		{"RTS",&c::RTS,&c::IMP,6},//0,6
		{"ADC",&c::ADC,&c::IIX,6},//1,6
		{"NUL",&c::NUL,&c::ERR,0},//2,6
		{"NUL",&c::NUL,&c::ERR,0},//3,6
		{"NUL",&c::NUL,&c::ERR,0},//4,6
		{"ADC",&c::ADC,&c::ZPA,3},//5,6
		{"ROR",&c::ROR,&c::ZPA,5},//6,6
		{"NUL",&c::NUL,&c::ERR,0},//7,6
		{"PLA",&c::PLA,&c::IMP,4},//8,6
		{"ADC",&c::ADC,&c::IMM,2},//9,6
		{"ROR",&c::ROR,&c::ACA,2},//A,6
		{"NUL",&c::NUL,&c::ERR,0},//B,6
		{"JMP",&c::JMP,&c::ABI,5},//C,6
		{"ADC",&c::ADC,&c::ABS,4},//D,6
		{"ROR",&c::ROR,&c::ABS,6},//E,6
		{"NUL",&c::NUL,&c::ERR,0},//F,6
	
		{"BVS",&c::BVS,&c::REL,2},//0,7
		{"ADC",&c::ADC,&c::IIY,5},//1,7
		{"NUL",&c::NUL,&c::ERR,0},//2,7
		{"NUL",&c::NUL,&c::ERR,0},//3,7
		{"NUL",&c::NUL,&c::ERR,0},//4,7
		{"ADC",&c::ADC,&c::ZPX,4},//5,7
		{"ROR",&c::ROR,&c::ZPX,6},//6,7
		{"NUL",&c::NUL,&c::ERR,0},//7,7
		{"SEI",&c::SEI,&c::IMP,2},//8,7
		{"ADC",&c::ADC,&c::IAY,4},//9,7
		{"NUL",&c::NUL,&c::ERR,0},//A,7
		{"NUL",&c::NUL,&c::ERR,0},//B,7
		{"NUL",&c::NUL,&c::ERR,0},//C,7
		{"ADC",&c::ADC,&c::IAX,4},//D,7
		{"ROR",&c::ROR,&c::IAX,7},//E,7
		{"NUL",&c::NUL,&c::ERR,0},//F,7
	
		{"NUL",&c::NUL,&c::ERR,0},//0,8
		{"STA",&c::STA,&c::IIX,6},//1,8
		{"NUL",&c::NUL,&c::ERR,0},//2,8
		{"NUL",&c::NUL,&c::ERR,0},//3,8
		{"STY",&c::STY,&c::ZPA,3},//4,8
		{"STA",&c::STA,&c::ZPA,3},//5,8
		{"STX",&c::STX,&c::ZPA,3},//6,8
		{"NUL",&c::NUL,&c::ERR,0},//7,8
		{"DEY",&c::DEY,&c::IMP,2},//8,8
		{"NUL",&c::NUL,&c::ERR,0},//9,8
		{"TXA",&c::TXA,&c::IMP,2},//A,8
		{"NUL",&c::NUL,&c::ERR,0},//B,8
		{"STY",&c::STY,&c::ABS,4},//C,8
		{"STA",&c::STA,&c::ABS,4},//D,8
		{"STX",&c::STX,&c::ABS,4},//E,8
		{"NUL",&c::NUL,&c::ERR,0},//F,8
	
		{"BCC",&c::BCC,&c::REL,2},//0,9
		{"STA",&c::STA,&c::IIY,6},//1,9
		{"NUL",&c::NUL,&c::ERR,0},//2,9
		{"NUL",&c::NUL,&c::ERR,0},//3,9
		{"STY",&c::STY,&c::ZPX,4},//4,9
		{"STA",&c::STA,&c::ZPX,4},//5,9
		{"STX",&c::STX,&c::ZPY,4},//6,9
		{"NUL",&c::NUL,&c::ERR,0},//7,9
		{"TYA",&c::TYA,&c::IMP,2},//8,9
		{"STA",&c::STA,&c::IAY,5},//9,9
		{"TXS",&c::TXS,&c::IMP,2},//A,9
		{"NUL",&c::NUL,&c::ERR,0},//B,9
		{"NUL",&c::NUL,&c::ERR,0},//C,9
		{"STA",&c::STA,&c::IAX,5},//D,9
		{"NUL",&c::NUL,&c::ERR,0},//E,9
		{"NUL",&c::NUL,&c::ERR,0},//F,9
	
		{"LDY",&c::LDY,&c::IMM,2},//0,A
		{"LDA",&c::LDA,&c::IIX,6},//1,A
		{"LDX",&c::LDX,&c::IMM,2},//2,A
		{"NUL",&c::NUL,&c::ERR,0},//3,A
		{"LDY",&c::LDY,&c::ZPA,3},//4,A
		{"LDA",&c::LDA,&c::ZPA,3},//5,A
		{"LDX",&c::LDX,&c::ZPA,3},//6,A
		{"NUL",&c::NUL,&c::ERR,0},//7,A
		{"TAY",&c::TAY,&c::IMP,2},//8,A
		{"LDA",&c::LDA,&c::IMM,2},//9,A
		{"TAX",&c::TAX,&c::IMP,2},//A,A
		{"NUL",&c::NUL,&c::ERR,0},//B,A
		{"LDY",&c::LDY,&c::ABS,4},//C,A
		{"LDA",&c::LDA,&c::ABS,4},//D,A
		{"LDX",&c::LDX,&c::ABS,4},//E,A
		{"NUL",&c::NUL,&c::ERR,0},//F,A
	
		{"BCS",&c::BCS,&c::REL,2},//0,B
		{"LDA",&c::LDA,&c::IIY,5},//1,B
		{"NUL",&c::NUL,&c::ERR,0},//2,B
		{"NUL",&c::NUL,&c::ERR,0},//3,B
		{"LDY",&c::LDY,&c::ZPX,4},//4,B
		{"LDA",&c::LDA,&c::ZPX,4},//5,B
		{"LDX",&c::LDX,&c::ZPY,4},//6,B
		{"NUL",&c::NUL,&c::ERR,0},//7,B
		{"CLV",&c::CLV,&c::IMP,2},//8,B
		{"LDA",&c::LDA,&c::IAY,4},//9,B
		{"TSX",&c::TSX,&c::IMP,2},//A,B
		{"NUL",&c::NUL,&c::ERR,0},//B,B
		{"LDY",&c::LDY,&c::IAX,4},//C,B
		{"LDA",&c::LDA,&c::IAX,4},//D,B
		{"LDX",&c::LDX,&c::IAY,4},//E,B
		{"NUL",&c::NUL,&c::ERR,0},//F,B
	
		{"CPY",&c::CPY,&c::IMM,2},//0,C
		{"CMP",&c::CMP,&c::IIX,6},//1,C
		{"NUL",&c::NUL,&c::ERR,0},//2,C
		{"NUL",&c::NUL,&c::ERR,0},//3,C
		{"CPY",&c::CPY,&c::ZPA,3},//4,C
		{"CMP",&c::CMP,&c::ZPA,3},//5,C
		{"DEC",&c::DEC,&c::ZPA,5},//6,C
		{"NUL",&c::NUL,&c::ERR,0},//7,C
		{"INY",&c::INY,&c::IMP,2},//8,C
		{"CMP",&c::CMP,&c::IMM,2},//9,C
		{"DEX",&c::DEX,&c::IMP,2},//A,C
		{"NUL",&c::NUL,&c::ERR,0},//B,C
		{"CPY",&c::CPY,&c::ABS,4},//C,C
		{"CMP",&c::CMP,&c::ABS,4},//D,C
		{"DEC",&c::DEC,&c::ABS,6},//E,C
		{"NUL",&c::NUL,&c::ERR,0},//F,C
	
		{"BNE",&c::BNE,&c::REL,2},//0,D
		{"CMP",&c::CMP,&c::IIY,5},//1,D
		{"NUL",&c::NUL,&c::ERR,0},//2,D
		{"NUL",&c::NUL,&c::ERR,0},//3,D
		{"NUL",&c::NUL,&c::ERR,0},//4,D
		{"CMP",&c::CMP,&c::ZPX,4},//5,D
		{"DEC",&c::DEC,&c::ZPX,6},//6,D
		{"NUL",&c::NUL,&c::ERR,0},//7,D
		{"CLD",&c::CLD,&c::IMP,2},//8,D
		{"CMP",&c::CMP,&c::IAY,4},//9,D
		{"NUL",&c::NUL,&c::ERR,0},//A,D
		{"NUL",&c::NUL,&c::ERR,0},//B,D
		{"NUL",&c::NUL,&c::ERR,0},//C,D
		{"CMP",&c::CMP,&c::IAX,4},//D,D
		{"DEC",&c::DEC,&c::IAX,7},//E,D
		{"NUL",&c::NUL,&c::ERR,0},//F,D
		
		{"CPX",&c::CPX,&c::IMM,2},//0,E
		{"SBC",&c::SBC,&c::IIX,6},//1,E
		{"NUL",&c::NUL,&c::ERR,0},//2,E
		{"NUL",&c::NUL,&c::ERR,0},//3,E
		{"CPX",&c::CPX,&c::ZPA,3},//4,E
		{"SBC",&c::SBC,&c::ZPA,3},//5,E
		{"INC",&c::INC,&c::ZPA,5},//6,E
		{"NUL",&c::NUL,&c::ERR,0},//7,E
		{"INX",&c::INX,&c::IMP,2},//8,E
		{"SBC",&c::SBC,&c::IMM,2},//9,E
		{"NOP",&c::NOP,&c::IMP,2},//A,E
		{"NUL",&c::NUL,&c::ERR,0},//B,E
		{"CPX",&c::CPX,&c::ABS,4},//C,E
		{"SBC",&c::SBC,&c::ABS,4},//D,E
		{"INC",&c::INC,&c::ABS,6},//E,E
		{"NUL",&c::NUL,&c::ERR,0},//F,E
	
		{"BEQ",&c::BEQ,&c::REL,2},//0,F
		{"SBC",&c::SBC,&c::IIY,5},//1,F
		{"NUL",&c::NUL,&c::ERR,0},//2,F
		{"NUL",&c::NUL,&c::ERR,0},//3,F
		{"NUL",&c::NUL,&c::ERR,0},//4,F
		{"SBC",&c::SBC,&c::ZPX,4},//5,F
		{"INC",&c::INC,&c::ZPX,6},//6,F
		{"NUL",&c::NUL,&c::ERR,0},//7,F
		{"SED",&c::SED,&c::IMP,2},//8,F
		{"SBC",&c::SBC,&c::IAY,4},//9,F
		{"NUL",&c::NUL,&c::ERR,0},//A,F
		{"NUL",&c::NUL,&c::ERR,0},//B,F
		{"NUL",&c::NUL,&c::ERR,0},//C,F
		{"SBC",&c::SBC,&c::IAX,4},//D,F
		{"INC",&c::INC,&c::IAX,7},//E,F
		{"NUL",&c::NUL,&c::ERR,0} //F,F
	};

	/* Helper Functions */

//Return lower 4 bits of byte e.g. Low(0xAB) = 0x0B
	ubyte Low(ubyte val)
	{
		return (val & 0x0F);
	}

	//Return higher 4 bits byte e.g. High(0xAB) = 0x0A
	ubyte High(ubyte val)
	{
		return (val >> 4);
	}

	bool IsSet(Flag flag)
	{
		return (Status & flag) != 0;
	}

	void SetFlag(Flag flag)
	{
		Status |= flag;
	}

	void RemoveFlag(Flag flag)
	{
		Status &= ~flag;
	}

	void ToggleFlag(Flag flag)
	{
		Status ^= flag;
	}

	//Read byte from 16-bit address
	ubyte& Read(ubyte2 index);

	//Write byte to 16-bit address
	void Write(ubyte val, ubyte2 index);

	/* Addressing Mode Functions */
	//All of these assume that when they're invoked, program counter is still pointing to the opcode
	//for convenience raw data is stored as unsigned bytes however the representation used in the data 
	//is not reflective of the type assigned to it. The data returned as ubytes will probably actually be signed 2'complement
	ubyte& ACA(ubyte& cycles);//Accumulator Addressing
	ubyte& IMM(ubyte& cycles);//Immediate Address
	ubyte& ABS(ubyte& cycles);//Absolute Addressing
	ubyte& ZPA(ubyte& cycles);//Zero Page Addressing
	ubyte& ZPX(ubyte& cycles);//Indexed Zero Page Addressing X
	ubyte& ZPY(ubyte& cycles);//Indexed Zero Page Addressing Y
	ubyte& IAX(ubyte& cycles);//Indexed Absolute Addressing X
	ubyte& IAY(ubyte& cycles);//Indexed Absolute Addressing Y
	ubyte& IMP(ubyte& cycles);//Implied Addressing
	ubyte2 REL(ubyte& cycles);//Relative Addressing return absolute address to jump to if condition passes, used exclusively by branch instructions
	ubyte& IIX(ubyte& cycles);//Indexed Indirect Addressing
	ubyte& IIY(ubyte& cycles);//Indirect Indexed Addressing
	ubyte2 ABI(ubyte& cycles);//Absolute Indirect
	ubyte2 ABJ(ubyte& cycles);//Special Addressing Mode for JMP/JSR that take in 16-bit input absolute address
	ubyte2 ERR(ubyte&);//Handle cases where an invalid opcode is called

	/*Instructions*/
	//Source: "https://csh.rit.edu/~moffitt/docs/6502.html#DETAIL"

	//Add Memory to A with Carry
	void ADC(ubyte& data);
	//Bitwise - AND A with Memory
	void AND(ubyte& data);
	//Arithmetic Shift Left
	void ASL(ubyte& data);
	//Branch iff P.C is CLEAR
	void BCC(ubyte2 address);
	//Branch iff P.C is SET
	void BCS(ubyte2 address);
	//Branch iff P.Z is SET
	void BEQ(ubyte2 address);
	//Test bits in A with M
	void BIT(ubyte& data);
	//Branch iff P.N is SET
	void BMI(ubyte2 address);
	//Branch iff P.Z is CLEAR
	void BNE(ubyte2 address);
	//Branch iff P.N is CLEAR
	void BPL(ubyte2 address);
	//Simulate Interrupt ReQuest(IRQ)
	void BRK(ubyte& data);
	//Branch iff P.V is CLEAR
	void BVC(ubyte2 address);
	//Branch iff P.V is SET
	void BVS(ubyte2 address);
	//Branch iff P.V is CLEAR
	void CLC(ubyte& data);
	//Clear Decimal Flag(P.D)
	void CLD(ubyte& data);
	//Clear Interrupt(disable) Flag(P.I)
	void CLI(ubyte& data);
	//Clear oVerflow Flag(P.V)
	void CLV(ubyte& data);
	//Compare A with Memory
	void CMP(ubyte& data);
	//Compare X with Memory
	void CPX(ubyte& data);
	//Compare Y with Memory
	void CPY(ubyte& data);
	//Decrement Memory by one
	void DEC(ubyte& data);
	//Decrement X by one
	void DEX(ubyte& data);
	//Decrement Y by one
	void DEY(ubyte& data);
	//Bitwise - EXclusive - OR A with Memory
	void EOR(ubyte& data);
	//Increment Memory by one
	void INC(ubyte& data);
	//Increment X by one
	void INX(ubyte& data);
	//Increment Y by one
	void INY(ubyte& data);
	//GOTO Address
	void JMP(ubyte2 data);
	//Jump to SubRoutine
	void JSR(ubyte2 data);
	//Load A with Memory
	void LDA(ubyte& data);
	//Load X with Memory
	void LDX(ubyte& data);
	//Load Y with Memory
	void LDY(ubyte& data);
	//Logical Shift Right
	void LSR(ubyte& data);
	//No OPeration
	void NOP(ubyte& data);
	//Bitwise-OR A with Memory
	void ORA(ubyte& data);
	// PusH A onto Stack
	void PHA(ubyte& data);
	//PusH P onto Stack
	void PHP(ubyte& data);
	//PulL from Stack to A
	void PLA(ubyte& data);
	//PulL from Stack to P
	void PLP(ubyte& data);
	//ROtate Left
	void ROL(ubyte& data);
	//ROtate Right
	void ROR(ubyte& data);
	//ReTurn from Interrupt
	void RTI(ubyte& data);
	//ReTurn from Subroutine
	void RTS(ubyte& data);
	//Subtract Memory from A with Borrow
	void SBC(ubyte& data);
	//Subtract Memory from A with Borrow
	void SEC(ubyte& data);
	//Set Binary Coded Decimal Flag (P.D)
	void SED(ubyte& data);
	//Set Interrupt (disable) Flag (P.I)
	void SEI(ubyte& data);
	//Set Interrupt (disable) Flag (P.I)
	void STA(ubyte& data);
	//Store X in Memory
	void STX(ubyte& data);
	//Store Y in Memory
	void STY(ubyte& data);
	//Transfer A to X
	void TAX(ubyte& data);
	//Transfer A to Y
	void TAY(ubyte& data);
	//Transfer Stack Pointer to X
	void TSX(ubyte& data);
	//Transfer X to A
	void TXA(ubyte& data);
	//Transfer X to Stack Pointer
	void TXS(ubyte& data);
	//Transfer Y to A
	void TYA(ubyte& data);
	//Handles cases where an invalid opcode is called
	void NUL(ubyte& data);
};


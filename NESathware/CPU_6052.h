#pragma once
#include "CommonTypes.h"
#include <variant>

//Implementation of the 6502 8-Bit CPU
//Source: Technical overview "https://en.wikipedia.org/wiki/MOS_Technology_6502"
//More Technical Overview "https://ia803008.us.archive.org/9/items/mos_6500_mpu_preliminary_may_1976/mos_6500_mpu_preliminary_may_1976.pdf"
//Programming reference: "http://archive.6502.org/datasheets/synertek_programming_manual.pdf"
class CPU_6052
{
public:
	CPU_6052(class BUS& bus, ubyte2 programStartOverride)
		:Bus(bus),
		Accumulator(0),
		Y_Register(0),
		X_Register(0),
		ProgramCounter(0),
		StackPointer(0xff),
		Status(0)
	{
		//Reset();//Initialize CPU, simulates startup sequence
		SetFlag(InterruptDisable);
		ProgramCounter = programStartOverride;
	}

	//Execute current instruction and move to next instruction
	//return number of cycles to wait for executed instruction to complete
	ubyte Execute();

	//Initializes the CPU to begin Program execution as per specification
	//Only initializes the ProgramCounter and sets InterruptDisable flag
	void Reset();

	//Interrupt function for simulating Interrupts and non maskable interrupts as per specification
	ubyte NMI();
	ubyte IRQ();
private:
	//THIS CPU IS LITTLE ENDIAN

	BUS& Bus;

	ubyte Accumulator;//Accumulator register
	ubyte Y_Register;//Index register
	ubyte X_Register;//Index register
	ubyte2 ProgramCounter;//Program Counter, always points to next instruction to be executed
	ubyte StackPointer;//Stack Pointer, always points to the next available memory slot
	ubyte Status;//Flags

	enum Flag : ubyte
	{
		Carry =            0b00000001,
		Zero =             0b00000010,
		InterruptDisable = 0b00000100,
		Decimal =          0b00001000,//Decimal NOT AVAILABLE IN NES VERSION OF 6502
		Break =            0b00010000,
		Reserved =         0b00100000,//Reserved (BLANK - NEVER USED)
		Overflow =         0b01000000,
		Negative =         0b10000000
	};

	struct Operand
	{
		ubyte2 address;//6052 memory address of data
		ubyte deltaCycles;//change in cycle count from the base cycle count
	};

	using OperationPtr  = void (CPU_6052::*)(Operand&);
	using GetOperandPtr = Operand (CPU_6052::*)();
	struct Instruction
	{
		char Name[4];
		OperationPtr Operation;
		GetOperandPtr GetOperand;
		ubyte baseCycles;
	};

	using c = CPU_6052;
	//Function pointer array indexed by hex opcode - source: opcode matrix -> "http://archive.6502.org/datasheets/rockwell_r650x_r651x.pdf"
	const Instruction Instructions[256] =
	{							//col,row
		{"BRK",&c::BRK,&c::IMP,7},//0,0
		{"ORA",&c::ORA,&c::IIX,6},//1,0
		{"???",nullptr,nullptr,0},//2,0
		{"???",nullptr,nullptr,0},//3,0
		{"???",nullptr,nullptr,0},//4,0
		{"ORA",&c::ORA,&c::ZPA,3},//5,0
		{"ASL",&c::ASL,&c::ZPA,5},//6,0
		{"???",nullptr,nullptr,0},//7,0
		{"PHP",&c::PHP,&c::IMP,3},//8,0
		{"ORA",&c::ORA,&c::IMM,2},//9,0
		{"ASL",&c::ASLA,&c::IMP,2},//A,0 --------
		{"???",nullptr,nullptr,0},//B,0
		{"???",nullptr,nullptr,0},//C,0
		{"ORA",&c::ORA,&c::ABS,4},//D,0
		{"ASL",&c::ASL,&c::ABS,6},//E,0
		{"???",nullptr,nullptr,0},//F,0
	
		{"BPL",&c::BPL,&c::REL,2},//0,1
		{"ORA",&c::ORA,&c::IIY,5},//1,1
		{"???",nullptr,nullptr,0},//2,1
		{"???",nullptr,nullptr,0},//3,1
		{"???",nullptr,nullptr,0},//4,1
		{"ORA",&c::ORA,&c::ZPX,4},//5,1
		{"ASL",&c::ASL,&c::ZPX,6},//6,1
		{"???",nullptr,nullptr,0},//7,1
		{"CLC",&c::CLC,&c::IMP,2},//8,1
		{"ORA",&c::ORA,&c::IAY,4},//9,1
		{"???",nullptr,nullptr,0},//A,1
		{"???",nullptr,nullptr,0},//B,1
		{"???",nullptr,nullptr,0},//C,1
		{"ORA",&c::ORA,&c::IAX,4},//D,1
		{"ASL",&c::ASL,&c::IAX,7},//E,1
		{"???",nullptr,nullptr,0},//F,1
	
		{"JSR",&c::JSR,&c::ABJ,6},//0,2
		{"AND",&c::AND,&c::IIX,6},//1,2
		{"???",nullptr,nullptr,0},//2,2
		{"???",nullptr,nullptr,0},//3,2
		{"BIT",&c::BIT,&c::ZPA,3},//4,2
		{"AND",&c::AND,&c::ZPA,3},//5,2
		{"ROL",&c::ROL,&c::ZPA,5},//6,2
		{"???",nullptr,nullptr,0},//7,2
		{"PLP",&c::PLP,&c::IMP,4},//8,2
		{"AND",&c::AND,&c::IMM,2},//9,2
		{"ROL",&c::ROLA,&c::IMP,2},//A,2 -----------
		{"???",nullptr,nullptr,0},//B,2
		{"BIT",&c::BIT,&c::ABS,4},//C,2
		{"AND",&c::AND,&c::ABS,4},//D,2
		{"ROL",&c::ROL,&c::ABS,6},//E,2
		{"???",nullptr,nullptr,0},//F,2
	
		{"BMI",&c::BMI,&c::REL,2},//0,3
		{"AND",&c::AND,&c::IIY,5},//1,3
		{"???",nullptr,nullptr,0},//2,3
		{"???",nullptr,nullptr,0},//3,3
		{"???",nullptr,nullptr,0},//4,3
		{"AND",&c::AND,&c::ZPX,4},//5,3
		{"ROL",&c::ROL,&c::ZPX,6},//6,3
		{"???",nullptr,nullptr,0},//7,3
		{"SEC",&c::SEC,&c::IMP,2},//8,3
		{"AND",&c::AND,&c::IAY,4},//9,3
		{"???",nullptr,nullptr,0},//A,3
		{"???",nullptr,nullptr,0},//B,3
		{"???",nullptr,nullptr,0},//C,3
		{"AND",&c::AND,&c::IAX,4},//D,3
		{"ROL",&c::ROL,&c::IAX,7},//E,3
		{"???",nullptr,nullptr,0},//F,3
	
		{"RTI",&c::RTI,&c::IMP,6},//0,4
		{"EOR",&c::EOR,&c::IIX,6},//1,4
		{"???",nullptr,nullptr,0},//2,4
		{"???",nullptr,nullptr,0},//3,4
		{"???",nullptr,nullptr,0},//4,4
		{"EOR",&c::EOR,&c::ZPA,3},//5,4
		{"LSR",&c::LSR,&c::ZPA,5},//6,4
		{"???",nullptr,nullptr,0},//7,4
		{"PHA",&c::PHA,&c::IMP,3},//8,4
		{"EOR",&c::EOR,&c::IMM,2},//9,4
		{"LSR",&c::LSRA,&c::IMP,2},//A,4
		{"???",nullptr,nullptr,0},//B,4
		{"JMP",&c::JMP,&c::ABJ,3},//C,4
		{"EOR",&c::EOR,&c::ABS,4},//D,4
		{"LSR",&c::LSR,&c::ABS,6},//E,4
		{"???",nullptr,nullptr,0},//F,4
	
		{"BVC",&c::BVC,&c::REL,2},//0,5
		{"EOR",&c::EOR,&c::IIY,5},//1,5
		{"???",nullptr,nullptr,0},//2,5
		{"???",nullptr,nullptr,0},//3,5
		{"???",nullptr,nullptr,0},//4,5
		{"EOR",&c::EOR,&c::ZPX,4},//5,5
		{"LSR",&c::LSR,&c::ZPX,6},//6,5
		{"???",nullptr,nullptr,0},//7,5
		{"CLI",&c::CLI,&c::IMP,2},//8,5
		{"EOR",&c::EOR,&c::IAY,4},//9,5
		{"???",nullptr,nullptr,0},//A,5
		{"???",nullptr,nullptr,0},//B,5
		{"???",nullptr,nullptr,0},//C,5
		{"EOR",&c::EOR,&c::IAX,4},//D,5
		{"LSR",&c::LSR,&c::IAX,7},//E,5
		{"???",nullptr,nullptr,0},//F,5
	
		{"RTS",&c::RTS,&c::IMP,6},//0,6
		{"ADC",&c::ADC,&c::IIX,6},//1,6
		{ "???",nullptr,nullptr,0 },//2,6
		{ "???",nullptr,nullptr,0 },//3,6
		{ "???",nullptr,nullptr,0 },//4,6
		{"ADC",&c::ADC,&c::ZPA,3},//5,6
		{"ROR",&c::ROR,&c::ZPA,5},//6,6
		{ "???",nullptr,nullptr,0 },//7,6
		{"PLA",&c::PLA,&c::IMP,4},//8,6
		{"ADC",&c::ADC,&c::IMM,2},//9,6
		{"ROR",&c::RORA,&c::IMP,2},//A,6
		{ "???",nullptr,nullptr,0 },//B,6
		{"JMP",&c::JMP,&c::ABI,5},//C,6
		{"ADC",&c::ADC,&c::ABS,4},//D,6
		{"ROR",&c::ROR,&c::ABS,6},//E,6
		{ "???",nullptr,nullptr,0 },//F,6
	
		{"BVS",&c::BVS,&c::REL,2},//0,7
		{"ADC",&c::ADC,&c::IIY,5},//1,7
		{ "???",nullptr,nullptr,0 },//2,7
		{ "???",nullptr,nullptr,0 },//3,7
		{ "???",nullptr,nullptr,0 },//4,7
		{"ADC",&c::ADC,&c::ZPX,4},//5,7
		{"ROR",&c::ROR,&c::ZPX,6},//6,7
		{ "???",nullptr,nullptr,0 },//7,7
		{"SEI",&c::SEI,&c::IMP,2},//8,7
		{"ADC",&c::ADC,&c::IAY,4},//9,7
		{ "???",nullptr,nullptr,0 },//A,7
		{ "???",nullptr,nullptr,0 },//B,7
		{ "???",nullptr,nullptr,0 },//C,7
		{"ADC",&c::ADC,&c::IAX,4},//D,7
		{"ROR",&c::ROR,&c::IAX,7},//E,7
		{ "???",nullptr,nullptr,0 },//F,7
	
		{ "???",nullptr,nullptr,0 },//0,8
		{"STA",&c::STA,&c::IIX,6},//1,8
		{ "???",nullptr,nullptr,0 },//2,8
		{ "???",nullptr,nullptr,0 },//3,8
		{"STY",&c::STY,&c::ZPA,3},//4,8
		{"STA",&c::STA,&c::ZPA,3},//5,8
		{"STX",&c::STX,&c::ZPA,3},//6,8
		{ "???",nullptr,nullptr,0 },//7,8
		{"DEY",&c::DEY,&c::IMP,2},//8,8
		{ "???",nullptr,nullptr,0 },//9,8
		{"TXA",&c::TXA,&c::IMP,2},//A,8
		{ "???",nullptr,nullptr,0 },//B,8
		{"STY",&c::STY,&c::ABS,4},//C,8
		{"STA",&c::STA,&c::ABS,4},//D,8
		{"STX",&c::STX,&c::ABS,4},//E,8
		{ "???",nullptr,nullptr,0 },//F,8
	
		{"BCC",&c::BCC,&c::REL,2},//0,9
		{"STA",&c::STA,&c::IIY,6},//1,9
		{ "???",nullptr,nullptr,0 },//2,9
		{ "???",nullptr,nullptr,0 },//3,9
		{"STY",&c::STY,&c::ZPX,4},//4,9
		{"STA",&c::STA,&c::ZPX,4},//5,9
		{"STX",&c::STX,&c::ZPY,4},//6,9
		{ "???",nullptr,nullptr,0 },//7,9
		{"TYA",&c::TYA,&c::IMP,2},//8,9
		{"STA",&c::STA,&c::IAY,5},//9,9
		{"TXS",&c::TXS,&c::IMP,2},//A,9
		{ "???",nullptr,nullptr,0 },//B,9
		{ "???",nullptr,nullptr,0 },//C,9
		{"STA",&c::STA,&c::IAX,5},//D,9
		{ "???",nullptr,nullptr,0 },//E,9
		{ "???",nullptr,nullptr,0 },//F,9
	
		{"LDY",&c::LDY,&c::IMM,2},//0,A
		{"LDA",&c::LDA,&c::IIX,6},//1,A
		{"LDX",&c::LDX,&c::IMM,2},//2,A
		{ "???",nullptr,nullptr,0 },//3,A
		{"LDY",&c::LDY,&c::ZPA,3},//4,A
		{"LDA",&c::LDA,&c::ZPA,3},//5,A
		{"LDX",&c::LDX,&c::ZPA,3},//6,A
		{ "???",nullptr,nullptr,0 },//7,A
		{"TAY",&c::TAY,&c::IMP,2},//8,A
		{"LDA",&c::LDA,&c::IMM,2},//9,A
		{"TAX",&c::TAX,&c::IMP,2},//A,A
		{ "???",nullptr,nullptr,0 },//B,A
		{"LDY",&c::LDY,&c::ABS,4},//C,A
		{"LDA",&c::LDA,&c::ABS,4},//D,A
		{"LDX",&c::LDX,&c::ABS,4},//E,A
		{ "???",nullptr,nullptr,0 },//F,A
	
		{"BCS",&c::BCS,&c::REL,2},//0,B
		{"LDA",&c::LDA,&c::IIY,5},//1,B
		{ "???",nullptr,nullptr,0 },//2,B
		{ "???",nullptr,nullptr,0 },//3,B
		{"LDY",&c::LDY,&c::ZPX,4},//4,B
		{"LDA",&c::LDA,&c::ZPX,4},//5,B
		{"LDX",&c::LDX,&c::ZPY,4},//6,B
		{ "???",nullptr,nullptr,0 },//7,B
		{"CLV",&c::CLV,&c::IMP,2},//8,B
		{"LDA",&c::LDA,&c::IAY,4},//9,B
		{"TSX",&c::TSX,&c::IMP,2},//A,B
		{ "???",nullptr,nullptr,0 },//B,B
		{"LDY",&c::LDY,&c::IAX,4},//C,B
		{"LDA",&c::LDA,&c::IAX,4},//D,B
		{"LDX",&c::LDX,&c::IAY,4},//E,B
		{ "???",nullptr,nullptr,0 },//F,B
	
		{"CPY",&c::CPY,&c::IMM,2},//0,C
		{"CMP",&c::CMP,&c::IIX,6},//1,C
		{ "???",nullptr,nullptr,0 },//2,C
		{ "???",nullptr,nullptr,0 },//3,C
		{"CPY",&c::CPY,&c::ZPA,3},//4,C
		{"CMP",&c::CMP,&c::ZPA,3},//5,C
		{"DEC",&c::DEC,&c::ZPA,5},//6,C
		{ "???",nullptr,nullptr,0 },//7,C
		{"INY",&c::INY,&c::IMP,2},//8,C
		{"CMP",&c::CMP,&c::IMM,2},//9,C
		{"DEX",&c::DEX,&c::IMP,2},//A,C
		{ "???",nullptr,nullptr,0 },//B,C
		{"CPY",&c::CPY,&c::ABS,4},//C,C
		{"CMP",&c::CMP,&c::ABS,4},//D,C
		{"DEC",&c::DEC,&c::ABS,6},//E,C
		{ "???",nullptr,nullptr,0 },//F,C
	
		{"BNE",&c::BNE,&c::REL,2},//0,D
		{"CMP",&c::CMP,&c::IIY,5},//1,D
		{ "???",nullptr,nullptr,0 },//2,D
		{ "???",nullptr,nullptr,0 },//3,D
		{ "???",nullptr,nullptr,0 },//4,D
		{"CMP",&c::CMP,&c::ZPX,4},//5,D
		{"DEC",&c::DEC,&c::ZPX,6},//6,D
		{ "???",nullptr,nullptr,0 },//7,D
		{"CLD",&c::CLD,&c::IMP,2},//8,D
		{"CMP",&c::CMP,&c::IAY,4},//9,D
		{ "???",nullptr,nullptr,0 },//A,D
		{ "???",nullptr,nullptr,0 },//B,D
		{ "???",nullptr,nullptr,0 },//C,D
		{"CMP",&c::CMP,&c::IAX,4},//D,D
		{"DEC",&c::DEC,&c::IAX,7},//E,D
		{ "???",nullptr,nullptr,0 },//F,D
		
		{"CPX",&c::CPX,&c::IMM,2},//0,E
		{"SBC",&c::SBC,&c::IIX,6},//1,E
		{ "???",nullptr,nullptr,0 },//2,E
		{ "???",nullptr,nullptr,0 },//3,E
		{"CPX",&c::CPX,&c::ZPA,3},//4,E
		{"SBC",&c::SBC,&c::ZPA,3},//5,E
		{"INC",&c::INC,&c::ZPA,5},//6,E
		{ "???",nullptr,nullptr,0 },//7,E
		{"INX",&c::INX,&c::IMP,2},//8,E
		{"SBC",&c::SBC,&c::IMM,2},//9,E
		{"NOP",&c::NOP,&c::IMP,2},//A,E
		{ "???",nullptr,nullptr,0 },//B,E
		{"CPX",&c::CPX,&c::ABS,4},//C,E
		{"SBC",&c::SBC,&c::ABS,4},//D,E
		{"INC",&c::INC,&c::ABS,6},//E,E
		{ "???",nullptr,nullptr,0 },//F,E
	
		{"BEQ",&c::BEQ,&c::REL,2},//0,F
		{"SBC",&c::SBC,&c::IIY,5},//1,F
		{ "???",nullptr,nullptr,0 },//2,F
		{ "???",nullptr,nullptr,0 },//3,F
		{ "???",nullptr,nullptr,0 },//4,F
		{"SBC",&c::SBC,&c::ZPX,4},//5,F
		{"INC",&c::INC,&c::ZPX,6},//6,F
		{ "???",nullptr,nullptr,0 },//7,F
		{"SED",&c::SED,&c::IMP,2},//8,F
		{"SBC",&c::SBC,&c::IAY,4},//9,F
		{ "???",nullptr,nullptr,0 },//A,F
		{ "???",nullptr,nullptr,0 },//B,F
		{ "???",nullptr,nullptr,0 },//C,F
		{"SBC",&c::SBC,&c::IAX,4},//D,F
		{"INC",&c::INC,&c::IAX,7},//E,F
		{ "???",nullptr,nullptr,0 } //F,F
	};

	/* Helper Functions */

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

	void SetFlagTo(Flag flag, bool condition)
	{
		if (condition)
			SetFlag(flag);
		else
			RemoveFlag(flag);
	}

	//Push onto Stack and update stack pointer
	void PushOntoStack(ubyte val)
	{
		Write(val, (ubyte2)StackPointer + 0x0100);//StackPointer MSB is always 0x01
		--StackPointer;//stack grows toward lower memory addresses, it starts at 0x01xx and goes to 0x0100
	}

	//Pop value off stack and update stack pointer
	ubyte PopOffStack()
	{
		++StackPointer;
		return Read((ubyte2)StackPointer + 0x0100);
	}

	//Read byte from 16-bit address
	ubyte Read(ubyte2 address);

	//Write byte to 16-bit address
	void Write(ubyte val, ubyte2 address);

	/* Addressing Mode Functions */
	//Source: "https://www.middle-engine.com/blog/posts/2020/06/23/programming-the-nes-the-6502-in-detail"
	//All of these assume that when they're invoked, program counter is still pointing to the opcode
	//for convenience raw data is stored as unsigned bytes however the representation used in the data 
	//is not reflective of the type assigned to it. The data returned as ubytes will probably actually be signed 2'complement
	Operand IMM();//Immediate Address
	Operand ABS();//Absolute Addressing
	Operand ZPA();//Zero Page Addressing
	Operand ZPX();//Indexed Zero Page Addressing X
	Operand ZPY();//Indexed Zero Page Addressing Y
	Operand IAX();//Indexed Absolute Addressing X, affects deltaCycles
	Operand IAY();//Indexed Absolute Addressing Y, affects deltaCycles
	Operand IMP();//Implied Addressing
	Operand REL();//Relative Addressing return absolute address to jump to if condition passes, used exclusively by branch instructions, affects deltaCycles
	Operand IIX();//Indexed Indirect Addressing
	Operand IIY();//Indirect Indexed Addressing, affects deltaCycles
	Operand ABI();//Absolute Indirect
	Operand ABJ();//Special Addressing Mode for JMP/JSR that take in 16-bit input absolute address

	/*Instructions*/
	//Source: "https://www.middle-engine.com/blog/posts/2020/06/23/programming-the-nes-the-6502-in-detail"

	//Add Memory to A with Carry
	void ADC(Operand&);
	//Bitwise - AND A with Memory
	void AND(Operand&);
	//Arithmetic Shift Left
	void ASL(Operand&);
	//Arithmetic Shift Left for accumulator
	void ASLA(Operand&);
	//Branch iff P.C is CLEAR
	void BCC(Operand&);
	//Branch iff P.C is SET
	void BCS(Operand&);
	//Branch iff P.Z is SET
	void BEQ(Operand&);
	//Test bits in A with M
	void BIT(Operand&);
	//Branch iff P.N is SET
	void BMI(Operand&);
	//Branch iff P.Z is CLEAR
	void BNE(Operand&);
	//Branch iff P.N is CLEAR
	void BPL(Operand&);
	//Simulate Interrupt ReQuest(IRQ)
	void BRK(Operand&);
	//Branch iff P.V is CLEAR
	void BVC(Operand&);
	//Branch iff P.V is SET
	void BVS(Operand&);
	//Clear Carry Flag
	void CLC(Operand&);
	//Clear Decimal Flag(P.D)
	void CLD(Operand&);
	//Clear Interrupt(disable) Flag(P.I)
	void CLI(Operand&);
	//Clear oVerflow Flag(P.V)
	void CLV(Operand&);
	//Compare A with Memory
	void CMP(Operand&);
	//Compare X with Memory
	void CPX(Operand&);
	//Compare Y with Memory
	void CPY(Operand&);
	//Decrement Memory by one
	void DEC(Operand&);
	//Decrement X by one
	void DEX(Operand&);
	//Decrement Y by one
	void DEY(Operand&);
	//Bitwise - EXclusive - OR A with Memory
	void EOR(Operand&);
	//Increment Memory by one
	void INC(Operand&);
	//Increment X by one
	void INX(Operand&);
	//Increment Y by one
	void INY(Operand&);
	//GOTO Address
	void JMP(Operand&);
	//Jump to SubRoutine
	void JSR(Operand&);
	//Load A with Memory
	void LDA(Operand&);
	//Load X with Memory
	void LDX(Operand&);
	//Load Y with Memory
	void LDY(Operand&);
	//Logical Shift Right
	void LSR(Operand&);
	//Logical shift Right for accumulator
	void LSRA(Operand&);
	//No OPeration
	void NOP(Operand&);
	//Bitwise-OR A with Memory
	void ORA(Operand&);
	// PusH A onto Stack
	void PHA(Operand&);
	//PusH P onto Stack
	void PHP(Operand&);
	//PulL from Stack to A
	void PLA(Operand&);
	//PulL from Stack to P
	void PLP(Operand&);
	//ROtate Left
	void ROL(Operand&);
	//Rotate Left for Accumulator
	void ROLA(Operand&);
	//ROtate Right
	void ROR(Operand&);
	//ROtate Right for Accumulator
	void RORA(Operand&);
	//ReTurn from Interrupt
	void RTI(Operand&);
	//ReTurn from Subroutine
	void RTS(Operand&);
	//Subtract Memory from A with Borrow
	void SBC(Operand&);
	//Set Carry Flag
	void SEC(Operand&);
	//Set Binary Coded Decimal Flag (P.D)
	void SED(Operand&);
	//Set Interrupt (disable) Flag (P.I)
	void SEI(Operand&);
	//Store Accumulator In Memory
	void STA(Operand&);
	//Store X in Memory
	void STX(Operand&);
	//Store Y in Memory
	void STY(Operand&);
	//Transfer A to X
	void TAX(Operand&);
	//Transfer A to Y
	void TAY(Operand&);
	//Transfer Stack Pointer to X
	void TSX(Operand&);
	//Transfer X to A
	void TXA(Operand&);
	//Transfer X to Stack Pointer
	void TXS(Operand&);
	//Transfer Y to A
	void TYA(Operand&);
};


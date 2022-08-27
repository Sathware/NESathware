#include "CPU_6052.h"
#include <string>
#include "BUS.h"
#include <iostream>
#include <format>


/* IMPORTANT NOTE: INC, DEC, LSR, ASL, ROL, ROR simulate data reads even though they modify the data, which may or may not cause issues with PPU addressing */

ubyte CPU_6052::Execute()
{
	ubyte2 currAddress = ProgramCounter;

	ubyte opcode = Read(ProgramCounter);
	const Instruction& instruction = Instructions[opcode];
	
	if (instruction.Operation == nullptr)
		throw std::runtime_error("Invalid Opcode!");

	Operand operand = (this->*instruction.GetOperand)();
	(this->*instruction.Operation)(operand);
	
	//cycleCount += instruction.baseCycles + operand.deltaCycles;
	std::cout << std::format("Memory: {:#06x}    Opcode: {:#04x}    Name: {}    DataAddress: {:#06x}\n", currAddress, opcode, instruction.Name, operand.address);

	return instruction.baseCycles + operand.deltaCycles;
}

void CPU_6052::Reset()
{
	SetFlag(InterruptDisable);
	ubyte2 programCounterLow = Read(0xfffc);
	ubyte2 programCounterHigh = Read(0xfffd);
	ProgramCounter = (programCounterHigh << 8) | programCounterLow;//combine
}

ubyte CPU_6052::NMI()
{
	//unmaskable interrupt handlers are stored in another address
	//and are not affected by Interrupt disable flag
	PushOntoStack(HighByte(ProgramCounter));
	PushOntoStack(LowByte(ProgramCounter));
	PushOntoStack(Status | Break | InterruptDisable);
	//address of NMI interrupt handler
	ubyte2 pInterruptHandlerLow = Read(0xfffa);
	ubyte2 pInterruptHandlerHigh = Read(0xfffb);
	ubyte2 pInterruptHandler = (pInterruptHandlerHigh << 8) | pInterruptHandlerLow;
	ProgramCounter = pInterruptHandler;
	return 7;//Number of cycles for an interrupt to be processed
}

ubyte CPU_6052::IRQ()
{
	if (!IsSet(InterruptDisable))
	{
		PushOntoStack(HighByte(ProgramCounter));
		PushOntoStack(LowByte(ProgramCounter));
		PushOntoStack(Status | Break | InterruptDisable);
		//address of IRQ interrupt handler
		ubyte2 pInterruptHandlerLow = Read(0xfffe);
		ubyte2 pInterruptHandlerHigh = Read(0xffff);
		ubyte2 pInterruptHandler = (pInterruptHandlerHigh << 8) | pInterruptHandlerLow;
		ProgramCounter = pInterruptHandler;
		return 7;//Number of cycles for an interrupt to be processed
	}
	return 0;
}

ubyte CPU_6052::Read(ubyte2 address)
{
	return Bus.ReadCPU(address);
}

void CPU_6052::Write(ubyte val, ubyte2 address)
{
	Bus.WriteCPU(val, address);
}

/* Implementation of Addressing Modes */

CPU_6052::Operand CPU_6052::IMM()
{
	++ProgramCounter;
	ubyte2 address = ProgramCounter;
	++ProgramCounter;//Point to next instruction
	return { address, 0 };
}

CPU_6052::Operand CPU_6052::ABS()
{
	++ProgramCounter;
	ubyte2 low = Read(ProgramCounter);//get 1st operand byte immediately proceeding instruction byte
	++ProgramCounter;
	ubyte2 high = Read(ProgramCounter);//get 2nd operand byte
	ubyte2 address = CombineBytes(high, low);//combine low and high order bits to form full 16-bit address
	++ProgramCounter;

	return { address, 0 };
}

CPU_6052::Operand CPU_6052::ZPA()
{
	++ProgramCounter;
	ubyte2 address = Read(ProgramCounter);//higher order bits are assumed to be zero in zero page addressing
	++ProgramCounter;

	return { address, 0 };
}

CPU_6052::Operand CPU_6052::ZPX()
{
	++ProgramCounter;
	ubyte2 address = Read(ProgramCounter);
	++ProgramCounter;
	address = (address + X_Register) & 0x00ff;//Ensure no carry is added to high order bits as per specification

	return { address, 0 };
}

CPU_6052::Operand CPU_6052::ZPY()
{
	++ProgramCounter;
	ubyte2 address = Read(ProgramCounter);
	++ProgramCounter;
	address = (address + Y_Register) & 0x00ff;//Ensure no carry is added to high order bits as per specification

	return { address, 0 };
}

CPU_6052::Operand CPU_6052::IAX()
{
	++ProgramCounter;
	ubyte2 low = Read(ProgramCounter);//get 1st operand byte immediately proceeding instruction byte
	++ProgramCounter;
	ubyte2 high = Read(ProgramCounter);//get 2nd operand byte
	++ProgramCounter;
	ubyte2 address = CombineBytes(high, low);//combine low and high order bits to form full 16-bit address
	
	ubyte deltaCycles = 0;
	if (HighByte(address) != HighByte(address + X_Register))
		deltaCycles = 1;//Increase cycle count if page boundary is crossed

	return { ubyte2(address + X_Register), deltaCycles };//Like absolute but X_Register is added as offset
}

CPU_6052::Operand CPU_6052::IAY()
{
	++ProgramCounter;
	ubyte2 low = Read(ProgramCounter);//get 1st operand byte immediately proceeding instruction byte
	++ProgramCounter;
	ubyte2 high = Read(ProgramCounter);//get 2nd operand byte
	++ProgramCounter;
	ubyte2 address = CombineBytes(high, low);//combine low and high order bits to form full 16-bit 

	ubyte deltaCycles = 0;
	if (HighByte(address) != HighByte(address + Y_Register))
		deltaCycles = 1;//Increase cycle count if page boundary is crossed

	return { ubyte2(address + Y_Register), deltaCycles };//Like absolute but Y_Register is added as offset
}

CPU_6052::Operand CPU_6052::IMP()
{
	++ProgramCounter;
	return { 0,0 };//Dummy output, SHOULD NEVER BE USED with implied
}

CPU_6052::Operand CPU_6052::REL()
{
	++ProgramCounter;
	ubyte2 offset = Read(ProgramCounter);//offset is a signed 2's complement byte (-128 to 127)
	++ProgramCounter;//Point to next instruction

	if (isBitOn<7>(offset))//if offset is negative the most significant digit will be 1
		offset |= 0xff00;//preserve 2's complement representation

	ubyte deltaCycles = 0;
	if (HighByte(ProgramCounter) != HighByte(ProgramCounter + offset))
			deltaCycles = 1;

	return { ubyte2(ProgramCounter + offset), deltaCycles };
}

CPU_6052::Operand CPU_6052::IIX()
{
	++ProgramCounter;
	ubyte pAddress = Read(ProgramCounter);
	++ProgramCounter;
	pAddress += X_Register;//Carry/Overflow is disregarded as per specification, value must be within zero page, which is automatically handled by unsigned arithmetic

	ubyte2 addressLow = Read(pAddress);//Some address in page zero
	ubyte2 addressHigh = Read(ubyte(pAddress + 1u));//The next address in page zero, or wraps around to beginning of page zero which is automatically handled by unsigned arithmetic
	ubyte2 address = CombineBytes(addressHigh, addressLow);

	return { address, 0 };
}

CPU_6052::Operand CPU_6052::IIY()
{
	++ProgramCounter;
	ubyte pAddress = Read(ProgramCounter);
	++ProgramCounter;

	ubyte2 addressLow = Read(pAddress);
	ubyte2 addressHigh = Read(ubyte(pAddress + 1u));//Wraps pAddress as per specification which is handled automatically by unsigned arithmetic
	ubyte2 address = CombineBytes(addressHigh, addressLow);

	ubyte deltaCycles = 0;
	if (HighByte(address) != HighByte(address + Y_Register))
		deltaCycles = 1;//Increase cycle count if page boundary is crossed

	return { ubyte2(address + Y_Register), deltaCycles };//Add Y_Register as offset
}

CPU_6052::Operand CPU_6052::ABI()
{
	++ProgramCounter;
	ubyte pAddressLow = Read(ProgramCounter);
	++ProgramCounter;
	ubyte2 pAddressHigh = Read(ProgramCounter);

	ubyte2 pAddress = CombineBytes(pAddressHigh, pAddressLow);

	ubyte2 pAddressNext = CombineBytes(pAddressHigh, ubyte(pAddressLow + 1));//Simulate bug where address Read(xxff + 1) actually gives Read(xx00)

	ubyte2 addressLow = Read(pAddress);
	ubyte2 addressHigh = Read(pAddressNext);
	ubyte2 address = CombineBytes(addressHigh, addressLow);

	return { address, 0 };
}

CPU_6052::Operand CPU_6052::ABJ()
{
	++ProgramCounter;
	ubyte2 low = Read(ProgramCounter);//get 1st operand byte immediately proceeding instruction byte
	++ProgramCounter;
	ubyte2 high = Read(ProgramCounter);//get 2nd operand byte
	++ProgramCounter;
	ubyte2 address = CombineBytes(high, low);//combine low and high order bits to form full 16-bit address
	return { address, 0 };
}


/* Implementation of 6052 Instructions */

void CPU_6052::SEC(Operand&)
{
	SetFlag(Carry);
}

void CPU_6052::CLC(Operand&)
{
	RemoveFlag(Carry);
}

void CPU_6052::CLV(Operand&)
{
	RemoveFlag(Overflow);
}

void CPU_6052::SEI(Operand&)
{
	SetFlag(InterruptDisable);
}

void CPU_6052::CLI(Operand&)
{
	RemoveFlag(InterruptDisable);
}

void CPU_6052::SED(Operand&)
{
	SetFlag(Decimal);
}

void CPU_6052::CLD(Operand&)
{
	RemoveFlag(Decimal);
}

void CPU_6052::LDA(Operand& operand)
{
	ubyte data = Read(operand.address);
	Accumulator = data;
	SetFlagTo(Zero, data == 0);
	SetFlagTo(Negative, GetMSB(data));
}

void CPU_6052::LDX(Operand& operand)
{
	ubyte data = Read(operand.address);
	X_Register = data;
	SetFlagTo(Zero, data == 0);
	SetFlagTo(Negative, GetMSB(data));
}

void CPU_6052::LDY(Operand& operand)
{
	ubyte data = Read(operand.address);
	Y_Register = data;
	SetFlagTo(Zero, data == 0);
	SetFlagTo(Negative, GetMSB(data));
}

void CPU_6052::STA(Operand& operand)
{
	Write(Accumulator, operand.address);
	operand.deltaCycles = 0;
}

void CPU_6052::STX(Operand& operand)
{
	Write(X_Register, operand.address);
}

void CPU_6052::STY(Operand& operand)
{
	Write(Y_Register, operand.address);
}

void CPU_6052::TAX(Operand&)
{
	X_Register = Accumulator;
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator));
}

void CPU_6052::TAY(Operand&)
{
	Y_Register = Accumulator;
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator));
}

void CPU_6052::TXA(Operand&)
{
	Accumulator = X_Register;
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator));
}

void CPU_6052::TYA(Operand&)
{
	Accumulator = Y_Register;
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator));
}

void CPU_6052::TXS(Operand&)
{
	StackPointer = X_Register;
}

void CPU_6052::TSX(Operand&)
{
	X_Register = StackPointer;
	SetFlagTo(Zero, StackPointer == 0);
	SetFlagTo(Negative, GetMSB(StackPointer));
}

void CPU_6052::ADC(Operand& operand)
{
	ubyte data = Read(operand.address);
	//Conversion to byte then byte2 is done, so that when the values get widened, the 2's complement representation is preserved i.e. padding is f not 0;
	//Casting right to byte2 will pad with 0, not preserving 2's complement, e.g. 0xf0 will become 0x00f0 not 0xfff0
	sbyte2 temp = (sbyte2)(sbyte)Accumulator + (sbyte2)(sbyte)data + (sbyte2)(sbyte)IsSet(Carry);
	Accumulator = ubyte(temp & 0x00ff);

	SetFlagTo(Carry, isBitOn<8>((ubyte2)temp));//Value exceeds 255, i.e 8th bit is set
	SetFlagTo(Overflow, temp > 127 || temp < -128);//result cannot be respresented in one byte
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator) != 0);
}

void CPU_6052::SBC(Operand& operand)
{
	ubyte data = Read(operand.address);

	sbyte2 temp = (sbyte2)(sbyte)Accumulator - (sbyte2)(sbyte)data - (sbyte2)(sbyte)(!IsSet(Carry));
	Accumulator = ubyte(temp & 0x00ff);

	//Set if borrowing did not occur, else clear, borrowing did not occur if 8th bit of temp == 1
	//borrow is complement of carry
	SetFlagTo(Carry, !isBitOn<8>(temp));
	SetFlagTo(Overflow, temp > 127 || temp < -128);
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator));
}

void CPU_6052::AND(Operand& operand)
{
	ubyte data = Read(operand.address);
	Accumulator &= data;

	SetFlagTo(Negative, GetMSB(data));
	SetFlagTo(Zero, Accumulator == 0);
}

void CPU_6052::ORA(Operand& operand)
{
	ubyte data = Read(operand.address);
	Accumulator |= data;
	
	SetFlagTo(Negative, GetMSB(data));
	SetFlagTo(Zero, Accumulator == 0);
}

void CPU_6052::EOR(Operand& operand)
{
	ubyte data = Read(operand.address);
	Accumulator ^= data;

	SetFlagTo(Negative, GetMSB(data));
	SetFlagTo(Zero, Accumulator == 0);
}

void CPU_6052::INX(Operand&)
{
	++X_Register;//Increment should roll over as per specification, and unsigned already has that functionality built in

	SetFlagTo(Negative, GetMSB(X_Register));
	SetFlagTo(Zero, X_Register == 0);
}

void CPU_6052::INY(Operand&)
{
	++Y_Register;//Increment should roll over as per specification, and unsigned already has that functionality built in

	SetFlagTo(Negative, GetMSB(Y_Register));
	SetFlagTo(Zero, Y_Register == 0);
}

void CPU_6052::DEX(Operand&)
{
	--X_Register;//Decrement should roll over as per specification, and unsigned already has that functionality built in

	SetFlagTo(Negative, GetMSB(X_Register));
	SetFlagTo(Zero, X_Register == 0);
}

void CPU_6052::DEY(Operand&)
{
	--Y_Register;//Decrement should roll over as per specification, and unsigned already has that functionality built in

	SetFlagTo(Negative, GetMSB(Y_Register));
	SetFlagTo(Zero, Y_Register == 0);
}

void CPU_6052::INC(Operand& operand)
{
	ubyte data = Read(operand.address);
	++data;
	Write(data, operand.address);
	operand.deltaCycles = 0;

	SetFlagTo(Negative, GetMSB(data));
	SetFlagTo(Zero, data == 0);
}

void CPU_6052::DEC(Operand& operand)
{
	ubyte data = Read(operand.address);
	--data;
	Write(data, operand.address);
	operand.deltaCycles = 0;

	SetFlagTo(Negative, GetMSB(data));
	SetFlagTo(Zero, data == 0);
}

void CPU_6052::CMP(Operand& operand)
{
	ubyte data = Read(operand.address);
	ubyte temp = Accumulator - data;
	SetFlagTo(Carry, Accumulator >= data);
	SetFlagTo(Zero, Accumulator == data);
	SetFlagTo(Negative, GetMSB(temp));//this works because unsigned numbers roll over values if out of range 
}

void CPU_6052::CPX(Operand& operand)
{
	ubyte data = Read(operand.address);
	ubyte temp = X_Register - data;
	SetFlagTo(Carry, X_Register >= data);
	SetFlagTo(Zero, X_Register == data);
	SetFlagTo(Negative, GetMSB(temp));
}

void CPU_6052::CPY(Operand& operand)
{
	ubyte data = Read(operand.address);
	ubyte temp = Y_Register - data;
	SetFlagTo(Carry, Y_Register >= data);
	SetFlagTo(Zero, Y_Register == data);
	SetFlagTo(Negative, GetMSB(temp));
}

void CPU_6052::BIT(Operand& operand)
{
	ubyte data = Read(operand.address);
	ubyte temp = Accumulator & data;
	SetFlagTo(Negative, GetMSB(temp));
	SetFlagTo(Overflow, isBitOn<6>(temp));//Check the 6th bit of temp
	SetFlagTo(Zero, temp == 0);
}

void CPU_6052::LSR(Operand& operand)
{
	ubyte data = Read(operand.address);
	SetFlagTo(Carry, (data & 1) != 0);
	data = data >> 1;
	Write(data, operand.address);
	SetFlagTo(Zero, data == 0);
	RemoveFlag(Negative);

	operand.deltaCycles = 0;
}

void CPU_6052::LSRA(Operand&)
{
	SetFlagTo(Carry, (Accumulator & 1) != 0);
	Accumulator = Accumulator >> 1;
	SetFlagTo(Zero, Accumulator == 0);
	RemoveFlag(Negative);
}

void CPU_6052::ASL(Operand& operand)
{
	ubyte data = Read(operand.address);
	SetFlagTo(Carry, GetMSB(data));
	data = data << 1;
	Write(data, operand.address);
	SetFlagTo(Negative, GetMSB(data));
	SetFlagTo(Zero, data == 0);

	operand.deltaCycles = 0;//ASL does not change cycle count if page boundary is crossed
}

void CPU_6052::ASLA(Operand&)
{
	SetFlagTo(Carry, GetMSB(Accumulator));
	Accumulator = Accumulator << 1;
	SetFlagTo(Negative, GetMSB(Accumulator));
	SetFlagTo(Zero, Accumulator == 0);
}

void CPU_6052::ROL(Operand& operand)
{
	ubyte data = Read(operand.address);
	ubyte new0bit = IsSet(Carry);//new bit 0 comes from the carry flag for ROL
	SetFlagTo(Carry, GetMSB(data));//old bit 7 is used to update carry
	data = data << 1;
	data |= new0bit;
	Write(data, operand.address);
	SetFlagTo(Zero, data == 0);
	SetFlagTo(Negative, GetMSB(data));

	operand.deltaCycles = 0;
}

void CPU_6052::ROLA(Operand&)
{
	ubyte new0bit = IsSet(Carry);//new bit 0 comes from the carry flag for ROL
	SetFlagTo(Carry, GetMSB(Accumulator));//old bit 7 is used to update carry
	Accumulator = Accumulator << 1;
	Accumulator |= new0bit;
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator));
}

void CPU_6052::ROR(Operand& operand)
{
	ubyte data = Read(operand.address);
	ubyte new7bit = (IsSet(Carry) << 7);//new bit 7 comes from the carry flag for ROR
	SetFlagTo(Carry, (data & 1) != 0);//old 0 bit  is used to update carry
	data = data >> 1;
	data |= new7bit;
	Write(data, operand.address);
	SetFlagTo(Zero, data == 0);
	SetFlagTo(Negative, GetMSB(data));

	operand.deltaCycles = 0;
}

void CPU_6052::RORA(Operand&)
{
	ubyte new7bit = (IsSet(Carry) << 7);//new bit 7 comes from the carry flag for ROR
	SetFlagTo(Carry, (Accumulator & 1) != 0);//old 0 bit  is used to update carry
	Accumulator = Accumulator >> 1;
	Accumulator |= new7bit;
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator));
}

void CPU_6052::JMP(Operand& operand)
{
	ProgramCounter = operand.address;
}

void CPU_6052::BMI(Operand& operand)
{
	if (IsSet(Negative))
	{
		++operand.deltaCycles;
		ProgramCounter = operand.address;
	}
	operand.deltaCycles = 0;
}

void CPU_6052::BPL(Operand& operand)
{
	if (!IsSet(Negative))
	{
		++operand.deltaCycles;
		ProgramCounter = operand.address;
	}
	operand.deltaCycles = 0;
}

void CPU_6052::BVS(Operand& operand)
{
	if (IsSet(Overflow))
	{
		++operand.deltaCycles;
		ProgramCounter = operand.address;
	}
	operand.deltaCycles = 0;
}

void CPU_6052::BVC(Operand& operand)
{
	if (!IsSet(Overflow))
	{
		++operand.deltaCycles;
		ProgramCounter = operand.address;
	}
	operand.deltaCycles = 0;
}

void CPU_6052::BCS(Operand& operand)
{
	if (IsSet(Carry))
	{
		++operand.deltaCycles;
		ProgramCounter = operand.address;
	}
	operand.deltaCycles = 0;
}

void CPU_6052::BCC(Operand& operand)
{
	if (!IsSet(Carry))
	{
		++operand.deltaCycles;
		ProgramCounter = operand.address;
	}
	operand.deltaCycles = 0;
}

void CPU_6052::BEQ(Operand& operand)
{
	if (IsSet(Zero))
	{
		++operand.deltaCycles;
		ProgramCounter = operand.address;
	}
	operand.deltaCycles = 0;
}

void CPU_6052::BNE(Operand& operand)
{
	if (!IsSet(Zero))
	{
		++operand.deltaCycles;
		ProgramCounter = operand.address;
	}
	operand.deltaCycles = 0;
}

void CPU_6052::JSR(Operand& operand)
{
	//JSR stores the last byte of the instruction on the stack
	//In other words ProgramCounter - 1
	PushOntoStack(HighByte(ProgramCounter - 1));
	PushOntoStack(LowByte(ProgramCounter - 1));
	ProgramCounter = operand.address;
}

void CPU_6052::RTS(Operand& operand)
{
	ubyte2 returnAddressLow = PopOffStack();
	ubyte2 returnAddressHigh = PopOffStack();

	ubyte2 returnAddress = CombineBytes(returnAddressHigh, returnAddressLow);
	ProgramCounter = returnAddress + 1;//As per specification the RTS corrects the JSR by returning the returnAddress + 1 to point to the next instruction
}

void CPU_6052::PHA(Operand&)
{
	PushOntoStack(Accumulator);
}

void CPU_6052::PHP(Operand&)
{
	PushOntoStack(Status);
}

void CPU_6052::PLA(Operand&)
{
	Accumulator = PopOffStack();
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator));
}

void CPU_6052::PLP(Operand&)
{
	Status = PopOffStack();
}

void CPU_6052::BRK(Operand&)
{
	//As per specification BRK pushes onto the stack the Program Counter of the second byte after it
	//Since, ProgramCounter is currently pointing to the byte immediately after BRK,
	//ProgramCounter + 1 is pushed onto the stack

	++ProgramCounter;//Point to 2nd byte after BRK opcode
	PushOntoStack(HighByte(ProgramCounter));
	PushOntoStack(LowByte(ProgramCounter));
	PushOntoStack(Status | Break);
	ubyte2 pInterruptHandlerLow = Read(0xfffe);
	ubyte2 pInterruptHandlerHigh = Read(0xffff);
	ubyte2 pInterruptHandler = CombineBytes(pInterruptHandlerHigh, pInterruptHandlerLow);
	ProgramCounter = pInterruptHandler;
}

void CPU_6052::RTI(Operand&)
{
	Status = PopOffStack();
	RemoveFlag(Break);

	ubyte2 returnLow = PopOffStack();
	ubyte2 returnHigh = PopOffStack();
	ubyte2 returnAddress = CombineBytes(returnHigh, returnLow);
	ProgramCounter = returnAddress;
}

void CPU_6052::NOP(Operand&)
{
}


#include "CPU_6052.h"
#include <string>
#include "BUS.h"

ubyte CPU_6052::Execute()
{
	{
#if defined(DEBUG) || defined(_DEBUG)
		static int cycleCount = 0;
#endif // DEBUG

		ubyte opcode = GetData(ProgramCounter);
		const Instruction& instruction = Instructions[opcode];
		ubyte deltaCycles = 0;

#if defined(DEBUG) || defined(_DEBUG)
		//if (ProgramCounter ==/* 0xdbb5u*/0xc66eu)
		//	int x = 5;//Something to put a break point on

		std::cout << "Memory: " << "0x" << std::hex << std::setfill('0') << std::setw(4) << (unsigned int)ProgramCounter;
		std::cout << "   Opcode: " << " 0x" << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)opcode;
		std::cout << "   Name: " << instruction.Name;
#endif

		if (std::holds_alternative<JumpOperation>(instruction.Operation))
		{
			auto jumpoperation = std::get<JumpOperation>(instruction.Operation);//get operation
			auto dataFunc = std::get<OperandAddress>(instruction.Data);//get addressing mode

			ubyte2 address = (this->*dataFunc)(deltaCycles);//get change in cycles depending on whether page boundary was crossed or not

#if defined(DEBUG) || defined(_DEBUG)
			std::cout << "   Data: " << "0x" << std::hex << std::setfill('0') << std::setw(4) << (unsigned int)address;
#endif

			(this->*jumpoperation)(address, deltaCycles);//update based on function
		}
		else if (std::get<Operation>(instruction.Operation) == &CPU_6052::NUL)
		{
			throw std::runtime_error("Invalid Opcode!");
		}
		else
		{
			auto operation = std::get<Operation>(instruction.Operation);
			auto dataFunc = std::get<OperandByte>(instruction.Data);//get addressing mode

			ubyte& data = (this->*dataFunc)(deltaCycles);//get change in cycles depending on whether page boundary was crossed or not

#if defined(DEBUG) || defined(_DEBUG)
			if (&data != &Status)
				std::cout << "   Data: " << "0x" << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)data;
			else
				std::cout << "   Data: " << "-";
#endif

			(this->*operation)(data, deltaCycles);//update based on function
		}
		cycleCount += instruction.baseCycles + deltaCycles;

#if defined(DEBUG) || defined(_DEBUG)
		std::cout << "   Cyc: " << std::dec << cycleCount << std::endl;
#endif

		return instruction.baseCycles + deltaCycles;
	}
}

ubyte& CPU_6052::GetData(ubyte2 address)
{
	return Bus.Read(address);
}

void CPU_6052::SetData(ubyte val, ubyte2 address)
{
	Bus.Write(val, address);
}

/* Implementation of Addressing Modes */

ubyte& CPU_6052::ACA(ubyte& deltaCycles)
{
	++ProgramCounter;//Point to next instruction in program
	return Accumulator;
}

ubyte& CPU_6052::IMM(ubyte& deltaCycles)
{
	++ProgramCounter;
	ubyte& temp = GetData(ProgramCounter);//Get operand byte immediately proceeding instruction
	++ProgramCounter;//Point to next instruction
	return temp;
}

ubyte& CPU_6052::ABS(ubyte& deltaCycles)
{
	++ProgramCounter;
	ubyte2 low = GetData(ProgramCounter);//get 1st operand byte immediately proceeding instruction byte
	++ProgramCounter;
	ubyte2 high = GetData(ProgramCounter);//get 2nd operand byte
	ubyte2 address = (high << 8) | low;//combine low and high order bits to form full 16-bit address
	++ProgramCounter;

	return GetData(address);
}

ubyte& CPU_6052::ZPA(ubyte& deltaCycles)
{
	++ProgramCounter;
	ubyte2 address = GetData(ProgramCounter);//higher order bits are assumed to be zero in zero page addressing
	++ProgramCounter;

	return GetData(address);
}

ubyte& CPU_6052::ZPX(ubyte& deltaCycles)
{
	++ProgramCounter;
	ubyte2 address = GetData(ProgramCounter);
	++ProgramCounter;
	address = (address + X_Register) & 0x00ff;//Ensure no carry is added to high order bits as per specification

	return GetData(address);
}

ubyte& CPU_6052::ZPY(ubyte& deltaCycles)
{
	++ProgramCounter;
	ubyte2 address = GetData(ProgramCounter);
	++ProgramCounter;
	address = (address + Y_Register) & 0x00ff;//Ensure no carry is added to high order bits as per specification

	return GetData(address);
}

ubyte& CPU_6052::IAX(ubyte& deltaCycles)
{
	++ProgramCounter;
	ubyte2 low = GetData(ProgramCounter);//get 1st operand byte immediately proceeding instruction byte
	++ProgramCounter;
	ubyte2 high = GetData(ProgramCounter);//get 2nd operand byte
	++ProgramCounter;
	ubyte2 address = (high << 8) | low;//combine low and high order bits to form full 16-bit address
	
	if (High(address) != High(address + X_Register))
		++deltaCycles;//Increase cycle count if page boundary is crossed

	return GetData(address + X_Register);//Like absolute but X_Register is added as offset
}

ubyte& CPU_6052::IAY(ubyte& deltaCycles)
{
	++ProgramCounter;
	ubyte2 low = GetData(ProgramCounter);//get 1st operand byte immediately proceeding instruction byte
	++ProgramCounter;
	ubyte2 high = GetData(ProgramCounter);//get 2nd operand byte
	++ProgramCounter;
	ubyte2 address = (high << 8) | low;//combine low and high order bits to form full 16-bit 

	if (High(address) != High(address + Y_Register))
		++deltaCycles;//Increase cycle count if page boundary is crossed

	return GetData(address + Y_Register);//Like absolute but Y_Register is added as offset
}

ubyte& CPU_6052::IMP(ubyte& deltaCycles)
{
	++ProgramCounter;
	return Status;//Dummy output, SHOULD NEVER BE USED for implied
}

ubyte2 CPU_6052::REL(ubyte& deltaCycles)
{
	++ProgramCounter;
	ubyte2 offset = GetData(ProgramCounter);//offset is a signed 2's complement byte (-128 to 127)
	++ProgramCounter;//Point to next instruction

	if (isBitOn<7>(offset))//if offset is negative the most significant digit will be 1
		offset |= 0xff00;//preserve 2's complement representation


	if (High(ProgramCounter) != High(ProgramCounter + offset))
			++deltaCycles;

	return ProgramCounter + offset;
}

ubyte& CPU_6052::IIX(ubyte& deltaCycles)
{
	++ProgramCounter;
	ubyte pAddress = GetData(ProgramCounter);
	++ProgramCounter;
	pAddress += X_Register;//Carry/Overflow is disregarded as per specification, value must be within zero page, which is automatically handled by unsigned arithmetic

	ubyte2 addressLow = GetData(pAddress);//Some address in page zero
	ubyte2 addressHigh = GetData(ubyte(pAddress + 1u));//The next address in page zero, or wraps around to beginning of page zero which is automatically handled by unsigned arithmetic
	ubyte2 address = (addressHigh << 8u) | addressLow;

	return GetData(address);
}

ubyte& CPU_6052::IIY(ubyte& deltaCycles)
{
	++ProgramCounter;
	ubyte pAddress = GetData(ProgramCounter);
	++ProgramCounter;

	ubyte2 addressLow = GetData(pAddress);
	ubyte2 addressHigh = GetData(ubyte(pAddress + 1u));//Wraps pAddress as per specification which is handled automatically by unsigned arithmetic
	ubyte2 address = (addressHigh << 8) | addressLow;

	if (High(address) != High(address + Y_Register))
		++deltaCycles;//Increase cycle count if page boundary is crossed

	return GetData(address + Y_Register);//Add Y_Register as offset
}

ubyte2 CPU_6052::ABI(ubyte& deltaCycles)
{
	++ProgramCounter;
	ubyte pAddressLow = GetData(ProgramCounter);
	++ProgramCounter;
	ubyte2 pAddressHigh = GetData(ProgramCounter);

	ubyte2 pAddress = (pAddressHigh << 8) | pAddressLow;

	ubyte2 pAddressNext = (pAddressHigh << 8) | ubyte(pAddressLow + 1);//Simulate bug where address Read(xxff + 1) actually gives Read(xx00)

	ubyte2 addressLow = GetData(pAddress);
	ubyte2 addressHigh = GetData(pAddressNext);
	ubyte2 address = (addressHigh << 8) | addressLow;

	return address;
}

ubyte2 CPU_6052::ABJ(ubyte& deltaCycles)
{
	++ProgramCounter;
	ubyte2 low = GetData(ProgramCounter);//get 1st operand byte immediately proceeding instruction byte
	++ProgramCounter;
	ubyte2 high = GetData(ProgramCounter);//get 2nd operand byte
	++ProgramCounter;
	ubyte2 address = (high << 8) | low;//combine low and high order bits to form full 16-bit address
	return address;
}

ubyte2 CPU_6052::ERR(ubyte&)
{
	return 0;
}



/* Implementation of 6052 Instructions */

void CPU_6052::SEC(ubyte& dummy, ubyte& deltaCycles)
{
	SetFlag(Carry);
}

void CPU_6052::CLC(ubyte& dummy, ubyte& deltaCycles)
{
	RemoveFlag(Carry);
}

void CPU_6052::CLV(ubyte& dummy, ubyte& deltaCycles)
{
	RemoveFlag(Overflow);
}

void CPU_6052::SEI(ubyte& dummy, ubyte& deltaCycles)
{
	SetFlag(InterruptDisable);
}

void CPU_6052::CLI(ubyte& dummy, ubyte& deltaCycles)
{
	RemoveFlag(InterruptDisable);
}

void CPU_6052::SED(ubyte& dummy, ubyte& deltaCycles)
{
	SetFlag(Decimal);
}

void CPU_6052::CLD(ubyte& dummy, ubyte& deltaCycles)
{
	RemoveFlag(Decimal);
}

void CPU_6052::LDA(ubyte& data, ubyte& deltaCycles)
{
	Accumulator = data;
	SetFlagTo(Zero, data == 0);
	SetFlagTo(Negative, GetMSB(data));
}

void CPU_6052::LDX(ubyte& data, ubyte& deltaCycles)
{
	X_Register = data;
	SetFlagTo(Zero, data == 0);
	SetFlagTo(Negative, GetMSB(data));
}

void CPU_6052::LDY(ubyte& data, ubyte& deltaCycles)
{
	Y_Register = data;
	SetFlagTo(Zero, data == 0);
	SetFlagTo(Negative, GetMSB(data));
}

void CPU_6052::STA(ubyte& data, ubyte& deltaCycles)
{
	data = Accumulator;
	deltaCycles = 0;
}

void CPU_6052::STX(ubyte& data, ubyte& deltaCycles)
{
	data = X_Register;
}

void CPU_6052::STY(ubyte& data, ubyte& deltaCycles)
{
	data = Y_Register;
}

void CPU_6052::TAX(ubyte& dummy, ubyte& deltaCycles)
{
	X_Register = Accumulator;
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator));
}

void CPU_6052::TAY(ubyte& dummy, ubyte& deltaCycles)
{
	Y_Register = Accumulator;
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator));
}

void CPU_6052::TXA(ubyte& dummy, ubyte& deltaCycles)
{
	Accumulator = X_Register;
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator));
}

void CPU_6052::TYA(ubyte& dummy, ubyte& deltaCycles)
{
	Accumulator = Y_Register;
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator));
}

void CPU_6052::NUL(ubyte& data, ubyte& deltaCycles)
{
}

void CPU_6052::TXS(ubyte& dummy, ubyte& deltaCycles)
{
	StackPointer = X_Register;
}

void CPU_6052::TSX(ubyte& dummy, ubyte& deltaCycles)
{
	X_Register = StackPointer;
	SetFlagTo(Zero, StackPointer == 0);
	SetFlagTo(Negative, GetMSB(StackPointer));
}

void CPU_6052::ADC(ubyte& data, ubyte& deltaCycles)
{
	//Conversion to byte then byte2 is done, so that when the values get widened, the 2's complement representation is preserved i.e. padding is f not 0;
	//Casting right to byte2 will pad with 0, not preserving 2's complement, e.g. 0xf0 will become 0x00f0 not 0xfff0
	byte2 temp = (byte2)(byte)Accumulator + (byte2)(byte)data + (byte2)(byte)IsSet(Carry);
	Accumulator = ubyte(temp & 0x00ff);

	SetFlagTo(Carry, isBitOn<8>((ubyte2)temp));//Value exceeds 255, i.e 8th bit is set
	SetFlagTo(Overflow, temp > 127 || temp < -128);//result cannot be respresented in one byte
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator) != 0);
}

void CPU_6052::SBC(ubyte& data, ubyte& deltaCycles)
{
	byte2 temp = (byte2)(byte)Accumulator - (byte2)(byte)data - (byte2)(byte)(!IsSet(Carry));
	Accumulator = ubyte(temp & 0x00ff);

	//Set if borrowing did not occur, else clear, borrowing did not occur if 8th bit of temp == 1
	//borrow is complement of carry
	SetFlagTo(Carry, !isBitOn<8>(temp));
	SetFlagTo(Overflow, temp > 127 || temp < -128);
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator));
}

void CPU_6052::AND(ubyte& data, ubyte& deltaCycles)
{
	Accumulator &= data;

	SetFlagTo(Negative, GetMSB(data));
	SetFlagTo(Zero, Accumulator == 0);
}

void CPU_6052::ORA(ubyte& data, ubyte& deltaCycles)
{
	Accumulator |= data;
	
	SetFlagTo(Negative, GetMSB(data));
	SetFlagTo(Zero, Accumulator == 0);
}

void CPU_6052::EOR(ubyte& data, ubyte& deltaCycles)
{
	Accumulator ^= data;

	SetFlagTo(Negative, GetMSB(data));
	SetFlagTo(Zero, Accumulator == 0);
}

void CPU_6052::INX(ubyte& dummy, ubyte& deltaCycles)
{
	++X_Register;//Increment should roll over as per specification, and unsigned already has that functionality built in

	SetFlagTo(Negative, GetMSB(X_Register));
	SetFlagTo(Zero, X_Register == 0);
}

void CPU_6052::INY(ubyte& dummy, ubyte& deltaCycles)
{
	++Y_Register;//Increment should roll over as per specification, and unsigned already has that functionality built in

	SetFlagTo(Negative, GetMSB(Y_Register));
	SetFlagTo(Zero, Y_Register == 0);
}

void CPU_6052::DEX(ubyte& dummy, ubyte& deltaCycles)
{
	--X_Register;//Decrement should roll over as per specification, and unsigned already has that functionality built in

	SetFlagTo(Negative, GetMSB(X_Register));
	SetFlagTo(Zero, X_Register == 0);
}

void CPU_6052::DEY(ubyte& dummy, ubyte& deltaCycles)
{
	--Y_Register;//Decrement should roll over as per specification, and unsigned already has that functionality built in

	SetFlagTo(Negative, GetMSB(Y_Register));
	SetFlagTo(Zero, Y_Register == 0);
}

void CPU_6052::INC(ubyte& data, ubyte& deltaCycles)
{
	++data;
	deltaCycles = 0;

	SetFlagTo(Negative, GetMSB(data));
	SetFlagTo(Zero, data == 0);
}

void CPU_6052::DEC(ubyte& data, ubyte& deltaCycles)
{
	--data;
	deltaCycles = 0;

	SetFlagTo(Negative, GetMSB(data));
	SetFlagTo(Zero, data == 0);
}

void CPU_6052::CMP(ubyte& data, ubyte& deltaCycles)
{
	ubyte temp = Accumulator - data;
	SetFlagTo(Carry, Accumulator >= data);
	SetFlagTo(Zero, Accumulator == data);
	SetFlagTo(Negative, GetMSB(temp));//this works because unsigned numbers roll over values if out of range 
}

void CPU_6052::CPX(ubyte& data, ubyte& deltaCycles)
{
	ubyte temp = X_Register - data;
	SetFlagTo(Carry, X_Register >= data);
	SetFlagTo(Zero, X_Register == data);
	SetFlagTo(Negative, GetMSB(temp));
}

void CPU_6052::CPY(ubyte& data, ubyte& deltaCycles)
{
	ubyte temp = Y_Register - data;
	SetFlagTo(Carry, Y_Register >= data);
	SetFlagTo(Zero, Y_Register == data);
	SetFlagTo(Negative, GetMSB(temp));
}

void CPU_6052::BIT(ubyte& data, ubyte& deltaCycles)
{
	ubyte temp = Accumulator & data;
	SetFlagTo(Negative, GetMSB(temp));
	SetFlagTo(Overflow, isBitOn<6>(temp));//Check the 6th bit of temp
	SetFlagTo(Zero, temp == 0);
}

void CPU_6052::LSR(ubyte& data, ubyte& deltaCycles)
{
	SetFlagTo(Carry, (data & 1) != 0);
	data = data >> 1;
	SetFlagTo(Zero, data == 0);
	RemoveFlag(Negative);

	deltaCycles = 0;
}

void CPU_6052::ASL(ubyte& data, ubyte& deltaCycles)
{
	deltaCycles = 0;//ASL does not change cycle count if page boundary is crossed

	SetFlagTo(Carry, GetMSB(data));
	data = data << 1;
	SetFlagTo(Negative, GetMSB(data));
	SetFlagTo(Zero, data == 0);
}

void CPU_6052::ROL(ubyte& data, ubyte& deltaCycles)
{
	ubyte new0bit = IsSet(Carry);//new bit 0 comes from the carry flag for ROL
	SetFlagTo(Carry, GetMSB(data));//old bit 7 is used to update carry
	data = data << 1;
	data |= new0bit;
	SetFlagTo(Zero, data == 0);
	SetFlagTo(Negative, GetMSB(data));

	deltaCycles = 0;
}

void CPU_6052::ROR(ubyte& data, ubyte& deltaCycles)
{
	ubyte new7bit = (IsSet(Carry) << 7);//new bit 7 comes from the carry flag for ROR
	SetFlagTo(Carry, (data & 1) != 0);//old 0 bit  is used to update carry
	data = data >> 1;
	data |= new7bit;
	SetFlagTo(Zero, data == 0);
	SetFlagTo(Negative, GetMSB(data));

	deltaCycles = 0;
}

void CPU_6052::JMP(ubyte2 address, ubyte& deltaCycles)
{
	ProgramCounter = address;
}

void CPU_6052::BMI(ubyte2 address, ubyte& deltaCycles)
{
	if (IsSet(Negative))
	{
		++deltaCycles;
		ProgramCounter = address;
	}
	deltaCycles = 0;
}

void CPU_6052::BPL(ubyte2 address, ubyte& deltaCycles)
{
	if (!IsSet(Negative))
	{
		++deltaCycles;
		ProgramCounter = address;
	}
	deltaCycles = 0;
}

void CPU_6052::BVS(ubyte2 address, ubyte& deltaCycles)
{
	if (IsSet(Overflow))
	{
		++deltaCycles;
		ProgramCounter = address;
	}
	deltaCycles = 0;
}

void CPU_6052::BVC(ubyte2 address, ubyte& deltaCycles)
{
	if (!IsSet(Overflow))
	{
		++deltaCycles;
		ProgramCounter = address;
	}
	deltaCycles = 0;
}

void CPU_6052::BCS(ubyte2 address, ubyte& deltaCycles)
{
	if (IsSet(Carry))
	{
		++deltaCycles;
		ProgramCounter = address;
	}
	deltaCycles = 0;
}

void CPU_6052::BCC(ubyte2 address, ubyte& deltaCycles)
{
	if (!IsSet(Carry))
	{
		++deltaCycles;
		ProgramCounter = address;
	}
	deltaCycles = 0;
}

void CPU_6052::BEQ(ubyte2 address, ubyte& deltaCycles)
{
	if (IsSet(Zero))
	{
		++deltaCycles;
		ProgramCounter = address;
	}
	deltaCycles = 0;
}

void CPU_6052::BNE(ubyte2 address, ubyte& deltaCycles)
{
	if (!IsSet(Zero))
	{
		++deltaCycles;
		ProgramCounter = address;
	}
	deltaCycles = 0;
}

void CPU_6052::JSR(ubyte2 address, ubyte& deltaCycles)
{
	//JSR stores the last byte of the instruction on the stack
	//In other words ProgramCounter - 1
	PushOntoStack(High(ProgramCounter - 1));
	PushOntoStack(Low(ProgramCounter - 1));
	ProgramCounter = address;
}

void CPU_6052::RTS(ubyte& dummy, ubyte& deltaCycles)
{
	ubyte2 returnAddressLow = PopOffStack();
	ubyte2 returnAddressHigh = PopOffStack();

	ubyte2 returnAddress = (returnAddressHigh << 8) | returnAddressLow;
	ProgramCounter = returnAddress + 1;//As per specification the RTS corrects the JSR by returning the returnAddress + 1 to point to the next instruction
}

void CPU_6052::PHA(ubyte& dummy, ubyte& deltaCycles)
{
	PushOntoStack(Accumulator);
}

void CPU_6052::PHP(ubyte& dummy, ubyte& deltaCycles)
{
	PushOntoStack(Status);
}

void CPU_6052::PLA(ubyte& dummy, ubyte& deltaCycles)
{
	Accumulator = PopOffStack();
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator));
}

void CPU_6052::PLP(ubyte& dummy, ubyte& deltaCycles)
{
	Status = PopOffStack();
}

void CPU_6052::BRK(ubyte& dummy, ubyte& deltaCycles)
{
	//As per specification BRK pushes onto the stack the Program Counter of the second byte after it
	//Since, ProgramCounter is currently pointing to the byte immediately after BRK,
	//ProgramCounter + 1 is pushed onto the stack

	++ProgramCounter;//Point to 2nd byte after BRK opcode
	PushOntoStack(High(ProgramCounter));
	PushOntoStack(Low(ProgramCounter));
	PushOntoStack(Status | Break);
	ubyte2 pInterruptHandlerLow = GetData(0xfffe);
	ubyte2 pInterruptHandlerHigh = GetData(0xffff);
	ubyte2 pInterruptHandler = (pInterruptHandlerHigh << 8) | pInterruptHandlerLow;
	ProgramCounter = pInterruptHandler;
}

void CPU_6052::RTI(ubyte& dummy, ubyte& deltaCycles)
{
	Status = PopOffStack();
	RemoveFlag(Break);

	ubyte2 returnLow = PopOffStack();
	ubyte2 returnHigh = PopOffStack();
	ubyte2 returnAddress = (returnHigh << 8) | returnLow;
	ProgramCounter = returnAddress;
}

void CPU_6052::NOP(ubyte& dummy, ubyte& deltaCycles)
{
}


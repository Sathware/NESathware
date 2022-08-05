#include "CPU_6052.h"

ubyte& CPU_6052::Read(ubyte2 index)
{
	return Bus.RAM[index];
}

void CPU_6052::Write(ubyte val, ubyte2 index)
{
	Bus.RAM[index] = val;
}

/* Implementation of Addressing Modes */

ubyte& CPU_6052::ACA(ubyte& cycles)
{
	++ProgramCounter;//Point to next instruction in program
	return Accumulator;
}

ubyte& CPU_6052::IMM(ubyte& cycles)
{
	++ProgramCounter;
	ubyte& temp = Read(ProgramCounter);//Get operand byte immediately proceeding instruction
	++ProgramCounter;//Point to next instruction
	return temp;
}

ubyte& CPU_6052::ABS(ubyte& cycles)
{
	++ProgramCounter;
	ubyte2 low = Read(ProgramCounter);//get 1st operand byte immediately proceeding instruction byte
	++ProgramCounter;
	ubyte2 high = Read(ProgramCounter);//get 2nd operand byte
	ubyte2 address = (high << 8) | low;//combine low and high order bits to form full 16-bit address
	++ProgramCounter;

	return Read(address);
}

ubyte& CPU_6052::ZPA(ubyte& cycles)
{
	++ProgramCounter;
	ubyte2 address = Read(ProgramCounter);//higher order bits are assumed to be zero in zero page addressing
	++ProgramCounter;

	return Read(address);
}

ubyte& CPU_6052::ZPX(ubyte& cycles)
{
	++ProgramCounter;
	ubyte2 address = Read(ProgramCounter);
	++ProgramCounter;
	address = (address + X_Register) & 0x00ff;//Ensure no carry is added to high order bits as per specification

	return Read(address);
}

ubyte& CPU_6052::ZPY(ubyte& cycles)
{
	++ProgramCounter;
	ubyte2 address = Read(ProgramCounter);
	++ProgramCounter;
	address = (address + Y_Register) & 0x00ff;//Ensure no carry is added to high order bits as per specification

	return Read(address);
}

ubyte& CPU_6052::IAX(ubyte& cycles)
{
	++ProgramCounter;
	ubyte2 low = Read(ProgramCounter);//get 1st operand byte immediately proceeding instruction byte
	++ProgramCounter;
	ubyte2 high = Read(ProgramCounter);//get 2nd operand byte
	++ProgramCounter;
	ubyte2 address = (high << 8) | low;//combine low and high order bits to form full 16-bit address
	address += X_Register;//Like absolute but X_Register is added as offset

	return Read(address);
}

ubyte& CPU_6052::IAY(ubyte& cycles)
{
	++ProgramCounter;
	ubyte2 low = Read(ProgramCounter);//get 1st operand byte immediately proceeding instruction byte
	++ProgramCounter;
	ubyte2 high = Read(ProgramCounter);//get 2nd operand byte
	++ProgramCounter;
	ubyte2 address = (high << 8) | low;//combine low and high order bits to form full 16-bit 
	address += Y_Register;//Like absolute but Y_Register is added as offset

	return Read(address);
}

ubyte& CPU_6052::IMP(ubyte& cycles)
{
	++ProgramCounter;
	return Status;//Dummy output, SHOULD NEVER BE USED for implied
}

ubyte2 CPU_6052::REL(ubyte& cycles)
{
	++ProgramCounter;
	ubyte2 offset = Read(ProgramCounter);//offset is a signed 2's complement byte (-128 to 127), storing it in a 16-bit value just makes life easier
	++ProgramCounter;//Point to next instruction

	if (offset & 0b10000000)//if offset is negative the most significant digit will be 1
	{
		offset = ~offset + 1;//Convert from 2's complement to unsigned representation
		return ProgramCounter - offset;
	}
	else
		return ProgramCounter + offset;
}

ubyte& CPU_6052::IIX(ubyte& cycles)
{
	++ProgramCounter;
	ubyte pAddress = Read(ProgramCounter);
	++ProgramCounter;
	pAddress += X_Register;//Carry/Overflow is disregarded as per specification, value must be within zero page

	ubyte2 addressLow = Read(pAddress);//Some address in page zero
	ubyte2 addressHigh = Read(pAddress + 1);//The next address in page zero
	ubyte2 address = (addressHigh << 8) | addressLow;

	return Read(address);
}

ubyte& CPU_6052::IIY(ubyte& cycles)
{
	++ProgramCounter;
	ubyte2 pAddress = Read(ProgramCounter);
	++ProgramCounter;

	ubyte2 addressLow = Read(pAddress);//Don't take into account carry of the addition between pAddress and Y_Register
	ubyte2 addressHigh = Read(pAddress + 1);
	ubyte2 address = (addressHigh << 8) | addressLow;
	address += Y_Register;//Add Y_Register as offset

	return Read(address);
}

ubyte2 CPU_6052::ABI(ubyte& cycles)
{
	++ProgramCounter;
	ubyte2 pAddressLow = Read(ProgramCounter);
	++ProgramCounter;
	ubyte2 pAddressHigh = Read(ProgramCounter);

	ubyte2 pAddress = (pAddressHigh << 8) | pAddressLow;

	ubyte2 addressLow = Read(pAddress);
	ubyte2 addressHigh = Read(pAddress + 1);
	ubyte2 address = (addressHigh << 8) | addressLow;

	//ProgramCounter = address;

	return address;//Dummy return
}

ubyte2 CPU_6052::ABJ(ubyte& cycles)
{
	++ProgramCounter;
	ubyte2 low = Read(ProgramCounter);//get 1st operand byte immediately proceeding instruction byte
	++ProgramCounter;
	ubyte2 high = Read(ProgramCounter);//get 2nd operand byte
	++ProgramCounter;
	ubyte2 address = (high << 8) | low;//combine low and high order bits to form full 16-bit address
	return address;
}



/* Implementation of 6052 Instructions */

void CPU_6052::SEC(ubyte& dummy)
{
	SetFlag(Carry);
}

void CPU_6052::CLC(ubyte& dummy)
{
	RemoveFlag(Carry);
}

void CPU_6052::CLV(ubyte& dummy)
{
	RemoveFlag(Overflow);
}

void CPU_6052::SEI(ubyte& dummy)
{
	SetFlag(InterruptDisable);
}

void CPU_6052::CLI(ubyte& dummy)
{
	RemoveFlag(InterruptDisable);
}

void CPU_6052::CLD(ubyte& dummy)
{
	RemoveFlag(Decimal);
}

void CPU_6052::LDA(ubyte& data)
{
	Accumulator = data;
	SetFlagTo(Zero, data == 0);
	SetFlagTo(Negative, GetMSB(data));
}

void CPU_6052::LDX(ubyte& data)
{
	X_Register = data;
	SetFlagTo(Zero, data == 0);
	SetFlagTo(Negative, GetMSB(data));
}

void CPU_6052::LDY(ubyte& data)
{
	Y_Register = data;
	SetFlagTo(Zero, data == 0);
	SetFlagTo(Negative, GetMSB(data));
}

void CPU_6052::STA(ubyte& data)
{
	data = Accumulator;
}

void CPU_6052::STX(ubyte& data)
{
	data = X_Register;
}

void CPU_6052::STY(ubyte& data)
{
	data = Y_Register;
}

void CPU_6052::TAX(ubyte& dummy)
{
	X_Register = Accumulator;
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator));
}

void CPU_6052::TAY(ubyte& dummy)
{
	Y_Register = Accumulator;
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator));
}

void CPU_6052::TXA(ubyte& dummy)
{
	Accumulator = X_Register;
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator));
}

void CPU_6052::TYA(ubyte& dummy)
{
	Accumulator = Y_Register;
	SetFlagTo(Zero, Accumulator == 0);
	SetFlagTo(Negative, GetMSB(Accumulator));
}

void CPU_6052::TXS(ubyte& dummy)
{
	StackPointer = X_Register;
}

void CPU_6052::TSX(ubyte& dummy)
{
	X_Register = StackPointer;
	SetFlagTo(Zero, StackPointer == 0);
	SetFlagTo(Negative, GetMSB(StackPointer));
}

void CPU_6052::ADC(ubyte& data)
{
	//Conversion to byte then byte2 is done, so that when the values get widened, the 2's complement representation is preserved i.e. padding is f not 0;
	//Casting right to byte2 will pad with 0, not preserving 2's complement, e.g. 0xf0 will become 0x00f0 not 0xfff0
	byte2 temp = (byte2)(byte)Accumulator + (byte2)(byte)data + (byte2)(byte)IsSet(Carry);
	
	SetFlagTo(Carry, (temp & (1 << 8)) != 0);
	SetFlagTo(Overflow, temp > 127 || temp < -128);
	SetFlagTo(Zero, (temp & 0x00ff) == 0);
	SetFlagTo(Negative, (temp & (1 << 7)) != 0);
	
	Accumulator = ubyte(temp & 0x00ff);
}

void CPU_6052::SBC(ubyte& data)
{
	byte2 temp = (byte2)(byte)Accumulator - (byte2)(byte)data - (byte2)(byte)(!IsSet(Carry));

	//Set if borrowing did not occur, else clear, borrowing did not occur if 9th bit of temp == 1
	SetFlagTo(Carry, (temp & (1 << 8)) != 0);
	SetFlagTo(Overflow, temp > 127 || temp < -128);
	SetFlagTo(Zero, (temp & 0x00ff) == 0);
	SetFlagTo(Negative, (temp & (1 << 7)) != 0);

	Accumulator = ubyte(temp & 0x00ff);
}

void CPU_6052::AND(ubyte& data)
{
	Accumulator &= data;

	SetFlagTo(Negative, GetMSB(data));
	SetFlagTo(Zero, Accumulator == 0);
}

void CPU_6052::ORA(ubyte& data)
{
	Accumulator |= data;
	
	SetFlagTo(Negative, GetMSB(data));
	SetFlagTo(Zero, Accumulator == 0);
}

void CPU_6052::EOR(ubyte& data)
{
	Accumulator ^= data;

	SetFlagTo(Negative, GetMSB(data));
	SetFlagTo(Zero, Accumulator == 0);
}

void CPU_6052::INX(ubyte& dummy)
{
	++X_Register;

	SetFlagTo(Negative, GetMSB(X_Register));
	SetFlagTo(Zero, X_Register == 0);
}

void CPU_6052::INY(ubyte& dummy)
{
	++Y_Register;

	SetFlagTo(Negative, GetMSB(Y_Register));
	SetFlagTo(Zero, Y_Register == 0);
}

void CPU_6052::INX(ubyte& dummy)
{
	--X_Register;

	SetFlagTo(Negative, GetMSB(X_Register));
	SetFlagTo(Zero, X_Register == 0);
}

void CPU_6052::INY(ubyte& dummy)
{
	--Y_Register;

	SetFlagTo(Negative, GetMSB(Y_Register));
	SetFlagTo(Zero, Y_Register == 0);
}

void CPU_6052::INC(ubyte& data)
{
	++data;

	SetFlagTo(Negative, GetMSB(data));
	SetFlagTo(Zero, data == 0);
}

void CPU_6052::DEC(ubyte& data)
{
	--data;

	SetFlagTo(Negative, GetMSB(data));
	SetFlagTo(Zero, data == 0);
}

void CPU_6052::CMP(ubyte& data)
{
	SetFlagTo(Carry, (byte)Accumulator >= (byte)data);
	SetFlagTo(Zero, Accumulator == data);
	SetFlagTo(Negative, (byte)Accumulator < (byte)data);
}

void CPU_6052::CPX(ubyte& data)
{
	SetFlagTo(Carry, (byte)X_Register >= (byte)data);
	SetFlagTo(Zero, X_Register == data);
	SetFlagTo(Negative, (byte)X_Register < (byte)data);
}

void CPU_6052::CPY(ubyte& data)
{
	SetFlagTo(Carry, (byte)Y_Register >= (byte)data);
	SetFlagTo(Zero, Y_Register == data);
	SetFlagTo(Negative, (byte)Y_Register < (byte)data);
}

void CPU_6052::BIT(ubyte& data)
{
	ubyte temp = Accumulator & data;
	SetFlagTo(Negative, GetMSB(temp));
	SetFlagTo(Overflow, (temp & (1 << 6)) != 0);//Check the 6th bit of temp
	SetFlagTo(Zero, temp == 0);
}

void CPU_6052::LSR(ubyte& data)
{
	SetFlagTo(Carry, (data & 1) != 0);
	data = data >> 1;
	SetFlagTo(Zero, data == 0);
	RemoveFlag(Negative);
}

void CPU_6052::ASL(ubyte& data)
{
	SetFlagTo(Carry, GetMSB(data));
	data = data << 1;
	SetFlagTo(Negative, GetMSB(data));
	SetFlagTo(Zero, data == 0);
}

void CPU_6052::ROL(ubyte& data)
{
	ubyte new0bit = IsSet(Carry);//new bit 0 comes from the carry flag for ROL
	SetFlagTo(Carry, GetMSB(data));//old bit 7 is used to update carry
	data = data << 1;
	data |= new0bit;
	SetFlagTo(Zero, data == 0);
	SetFlagTo(Negative, GetMSB(data));
}

void CPU_6052::ROR(ubyte& data)
{
	ubyte new7bit = (IsSet(Carry) << 7);//new bit 7 comes from the carry flag for ROR
	SetFlagTo(Carry, (data & 1) != 0);//old 0 bit  is used to update carry
	data = data >> 1;
	data |= new7bit;
	SetFlagTo(Zero, data == 0);
	SetFlagTo(Negative, GetMSB(data));
}

void CPU_6052::JMP(ubyte2 address)
{
	ProgramCounter = address;
}

void CPU_6052::BMI(ubyte2 address)
{
	if (IsSet(Negative))
		ProgramCounter = address;
}

void CPU_6052::BPL(ubyte2 address)
{
	if (!IsSet(Negative))
		ProgramCounter = address;
}

void CPU_6052::BVS(ubyte2 address)
{
	if (IsSet(Overflow))
		ProgramCounter = address;
}

void CPU_6052::BVC(ubyte2 address)
{
	if (!IsSet(Overflow))
		ProgramCounter = address;
}

void CPU_6052::BCS(ubyte2 address)
{
	if (IsSet(Carry))
		ProgramCounter = address;
}

void CPU_6052::BCC(ubyte2 address)
{
	if (!IsSet(Carry))
		ProgramCounter = address;
}

void CPU_6052::BEQ(ubyte2 address)
{
	if (IsSet(Zero))
		ProgramCounter = address;
}

void CPU_6052::BNE(ubyte2 address)
{
	if (!IsSet(Zero))
		ProgramCounter = address;
}

void CPU_6052::JSR(ubyte2 address)
{

}
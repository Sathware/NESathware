#include "APU_2A03.h"
#include "BUS.h"

void APU_2A03::Execute()
{
	//Play sound for 0.00416589088f seconds
	if (clockCount == (2.0f * 3729.0f))
	{
		// Step 1
		mPulse1Envelope.Execute();

		PlayIfValid();
	}
	if (clockCount == (2.0f * 7457.0f))
	{
		// Step 2
		mPulse1Envelope.Execute();

		mPulse1Sweep.Execute();

		if (mPulse1LengthCounterFlag)
			mPulse1LengthCounter = 0;
		else
			mPulse1LengthCounter = (mPulse1LengthCounter == 0) ? 0 : (mPulse1LengthCounter - 1);

		PlayIfValid();
	}
	if (clockCount == (2.0f * 11186.0f))
	{
		// Step 3
		mPulse1Envelope.Execute();

		PlayIfValid();
	}
	if (clockCount == (2.0f * 14915.0f))
	{
		// Step 4
		mPulse1Envelope.Execute();

		mPulse1Sweep.Execute();

		if (mPulse1LengthCounterFlag)
			mPulse1LengthCounter = 0;
		else
			mPulse1LengthCounter = (mPulse1LengthCounter == 0) ? 0 : (mPulse1LengthCounter - 1);

		if (mInterruptFlag)
			Bus.InvokeIRQ();

		PlayIfValid();
	}

	clockCount = (clockCount + 1) % (2 * 14915);
}

void APU_2A03::WriteRegister(ubyte val, ubyte2 address)
{
	//APU and I/O registers
	if (address < 0x4004u)
	{
		// Pulse 1
		if (address == 0x4000u)
		{
			mPulse1Duty = val >> 6;
			mPulse1LengthCounterFlag = IsBitOn(5, val);
			mPulse1ConstantVolumeFlag = IsBitOn(4, val);
			mPulse1VolumeDividerPeriod = val & 0x0F;
		}
		if (address == 0x4001u)
		{
			mPulse1SweepFlag = IsBitOn(7, val);
			mPulse1SweepDividerPeriod = (val >> 4) & 0x0E;
			mPulse1SweepNegateFlag = IsBitOn(3, val);
			mPulse1SweepShiftCounter = val & 0x0E;

			// TODO set sweep reload flag

		}
		if (address == 0x4002u)
		{
			// Set low 8 bits of timer (timer is 11 bit)
			mPulse1Timer = (mPulse1Timer & 0x0700) | ubyte2(val);
		}
		if (address == 0x4003u)
		{
			// Set high 3 bits of timer (timer is 11 bit)
			mPulse1Timer = (mPulse1Timer & 0x00FF) | (ubyte2(val & 0x7) << 8);
			mPulse1LengthCounter = (val >> 3);

			// TODO set envelope start flag
		}
	}
	if (address < 0x4008u)
	{
		// Square 2
	}
	if (address < 0x400Cu)
	{
		// Triangle
	}
	if (address < 0x4010u)
	{
		// Noise
	}
	if (address < 0x4014u)
	{
		// DMC
	}

	if (address == 0x4017)
	{
		mInterruptFlag = IsBitOn(6, val);
	}
}

#include "APU_2A03.h"
#include "BUS.h"

void APU_2A03::Execute()
{
	/* APU Frame Counter */

	if (mClockCount == (2u * 3729u))
	{
		mPulse1.QueueWaveform();
		mPulse2.QueueWaveform();
	}
	if (mClockCount == (2u * 7457u))
	{
		mPulse1.mLengthCounter.Execute();
		mPulse2.mLengthCounter.Execute();

		mPulse1.QueueWaveform();
		mPulse2.QueueWaveform();
	}
	if (mClockCount == (2u * 11186u))
	{
		mPulse1.QueueWaveform();
		mPulse2.QueueWaveform();
	}
	if ( (mClockCount == (2u * 14915u) && !mModeFlag) || (mClockCount == (2u * 18641u) && mModeFlag) )
	{
		mPulse1.mLengthCounter.Execute();
		mPulse2.mLengthCounter.Execute();

		mPulse1.QueueWaveform();
		mPulse2.QueueWaveform();

		if (mInterruptFlag && !mModeFlag)
			Bus.InvokeIRQ();
	}

	mClockCount = (mClockCount + 1u) % (mModeFlag ? (2u * 18641u) : (2u * 14915u));
}

void APU_2A03::WriteRegister(ubyte val, ubyte2 address)
{
	//APU and I/O registers
	if (address < 0x4004u)
	{
		// Pulse 1
		if (address == 0x4000u)
		{
			/*mPulse1Duty = val >> 6;
			mPulse1LengthCounterFlag = IsBitOn(5, val);
			mPulse1ConstantVolumeFlag = IsBitOn(4, val);
			mPulse1VolumeDividerPeriod = val & 0x0F;*/

			mPulse1.mDutyCycle = val >> 6;
			mPulse1.mLengthCounter.mHaltFlag = IsBitOn(5, val);
		}
		if (address == 0x4001u)
		{
			/*mPulse1SweepFlag = IsBitOn(7, val);
			mPulse1SweepDividerPeriod = (val >> 4) & 0x0E;
			mPulse1SweepNegateFlag = IsBitOn(3, val);
			mPulse1SweepShiftCounter = val & 0x0E;*/

			// TODO set sweep reload flag

		}
		if (address == 0x4002u)
		{
			// Set low 8 bits of timer (timer is 11 bit)
			/*mPulse1Timer = (mPulse1Timer & 0x0700) | ubyte2(val);*/

			mPulse1.mTimer = (mPulse1.mTimer & 0x0700) | ubyte2(val);

			mPulse1.QueueWaveform();
		}
		if (address == 0x4003u)
		{
			// Set high 3 bits of timer (timer is 11 bit)
			/*mPulse1Timer = (mPulse1Timer & 0x00FF) | (ubyte2(val & 0x7) << 8);
			mPulse1LengthCounter = (val >> 3);*/

			mPulse1.mTimer = (mPulse1.mTimer & 0x00FF) | (ubyte2(val & 0x7) << 8);
			mPulse1.mLengthCounter.SetCounter(val >> 3);

			mPulse1.QueueWaveform();

			// TODO set envelope start flag
		}
	}
	if (address < 0x4008u)
	{
		// Pulse 2
		if (address == 0x4004u)
		{
			mPulse2.mDutyCycle = val >> 6;
			mPulse2.mLengthCounter.mHaltFlag = IsBitOn(5, val);
		}
		if (address == 0x4005u)
		{
			/*mPulse1SweepFlag = IsBitOn(7, val);
			mPulse1SweepDividerPeriod = (val >> 4) & 0x0E;
			mPulse1SweepNegateFlag = IsBitOn(3, val);
			mPulse1SweepShiftCounter = val & 0x0E;*/

			// TODO set sweep reload flag

		}
		if (address == 0x4006u)
		{
			// Set low 8 bits of timer (timer is 11 bit)
			/*mPulse1Timer = (mPulse1Timer & 0x0700) | ubyte2(val);*/

			mPulse2.mTimer = (mPulse2.mTimer & 0x0700) | ubyte2(val);

			mPulse2.QueueWaveform();
		}
		if (address == 0x4007u)
		{
			// Set high 3 bits of timer (timer is 11 bit)
			/*mPulse1Timer = (mPulse1Timer & 0x00FF) | (ubyte2(val & 0x7) << 8);
			mPulse1LengthCounter = (val >> 3);*/

			mPulse2.mTimer = (mPulse2.mTimer & 0x00FF) | (ubyte2(val & 0x7) << 8);
			mPulse2.mLengthCounter.SetCounter(val >> 3);

			mPulse2.QueueWaveform();

			// TODO set envelope start flag
		}
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

	if (address == 0x4015)
	{
		mPulse1.mLengthCounter.mEnabledFlag = IsBitOn(0, val);
		mPulse2.mLengthCounter.mEnabledFlag = IsBitOn(1, val);

		mPulse1.QueueWaveform();
		mPulse2.QueueWaveform();
	}

	if (address == 0x4017)
	{
		mModeFlag = IsBitOn(7, val);
		mInterruptFlag = IsBitOn(6, val);
	}
}

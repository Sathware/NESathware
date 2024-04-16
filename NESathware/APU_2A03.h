#pragma once
#include "CommonTypes.h"
#include <numbers>
#include <vector>
#include "../SathwareEngine/Audio.h"

class APU_2A03
{
public:
	APU_2A03(class BUS& bus, Audio& AudioEngine)
		: Bus(bus), AudioEngine(AudioEngine)
	{}

	//Run half an APU Cycle
	void Execute();
	//Write to APU Register
	void WriteRegister(ubyte val, ubyte2 address);
private:
	BUS& Bus;
	Audio& AudioEngine;

	int clockCount = 0;
	
	//Envelope (aka Volume) control unit
	struct EnvelopeUnit
	{
		EnvelopeUnit(const ubyte& volumePeriod, const bool& loopFlag, const bool& constantFlag)
			: volumePeriod(volumePeriod), loopFlag(loopFlag), constantFlag(constantFlag)
		{}
		const ubyte& volumePeriod;
		const bool& loopFlag;
		const bool& constantFlag;

		bool startFlag = true;
		ubyte dividerCounter = 0;
		ubyte decayCounter = 0;

		// Update volume parameters by one step
		void Execute()
		{
			// Case 1: start flag is set
			if (startFlag)
			{
				startFlag = false;
				decayCounter = 15;
				dividerCounter = volumePeriod;
				return;
			}

			// Case 2: start flag is not set
			if (dividerCounter == 0)
			{
				dividerCounter = volumePeriod;
				if (loopFlag)
					decayCounter = (decayCounter == 0) ? 15 : (decayCounter - 1);
				else
					decayCounter = (decayCounter == 0) ? 0 : (decayCounter - 1);
			}
			else
				dividerCounter = dividerCounter - 1;
		}

		float GetAmplitudeScalar()
		{
			if (constantFlag)
				return float(volumePeriod) / 15.0f;
			else
				return float(decayCounter) / 15.0f;
		}
	};

	struct SweepUnit
	{
		SweepUnit(ubyte& sweepPeriod, ubyte2& timerPeriod, ubyte& shiftCounter, bool& negateFlag)
			: sweepPeriod(sweepPeriod), timerPeriod(timerPeriod), shiftCounter(shiftCounter), negateFlag(negateFlag)
		{}
		const ubyte& sweepPeriod;
		ubyte2& timerPeriod;
		const ubyte& shiftCounter;
		const bool& negateFlag;

		bool reloadFlag = false;
		ubyte dividerCounter = 0;

		void Execute()
		{
			if (reloadFlag)
				dividerCounter = sweepPeriod;

			if (dividerCounter == 0)
			{
				reloadFlag = false;
				dividerCounter = sweepPeriod;
				ubyte2 change = timerPeriod >> shiftCounter;
				sbyte2 temp = (negateFlag ? (-sbyte2(change) - 1) : sbyte2(change)) + sbyte2(timerPeriod);
				timerPeriod = (temp < 0) ? 0 : ubyte2(temp);
			}
			dividerCounter = dividerCounter - 1;
		}

		float GetFrequencyScalar()
		{
			return 1789773.0f / (16.0f * (float(timerPeriod) + 1));
		}
	};

	/* Global audio parameters */
	bool mInterruptFlag = false;

	/* Parameters for each sound channel */
	// Pulse 1 //
	ubyte mPulse1Duty = 0;
	bool mPulse1LengthCounterFlag = false;
	bool mPulse1ConstantVolumeFlag = false;
	ubyte mPulse1VolumeDividerPeriod = 0;
	bool mPulse1SweepFlag = false;
	ubyte mPulse1SweepDividerPeriod = 0;
	bool mPulse1SweepNegateFlag = false;
	ubyte mPulse1SweepShiftCounter = 0;
	ubyte2 mPulse1Timer = 0;
	ubyte mPulse1LengthCounter = 0;
	EnvelopeUnit mPulse1Envelope = EnvelopeUnit(mPulse1VolumeDividerPeriod, mPulse1LengthCounterFlag, mPulse1ConstantVolumeFlag);
	SweepUnit mPulse1Sweep = SweepUnit(mPulse1SweepDividerPeriod, mPulse1Timer, mPulse1SweepShiftCounter, mPulse1SweepNegateFlag);

	std::vector<float> mPulse1SoundData = std::vector<float>(184); //44100 sampling rate for .00416589088 seconds give 184 samples


	float IntegerDutyCycleToFloat(ubyte duty)
	{
		if (duty == 0)
			return .125f;
		if (duty == 1)
			return .25f;
		if (duty == 2)
			return .50f;
		if (duty == 3)
			return .75f;
	}

	void GeneratePulseWaveForm(const float duty, const float amplitude, const float frequency, std::vector<float>& data)
	{
		constexpr float pi = float(std::numbers::pi);
		// Fixed sound length is 0.00416589088f
		for (unsigned int i = 0; i < 184; ++i)
		{
			float t = 0.00416589088f * float(i) / float(184);
			float sum = 0.0f;
			for (unsigned int k = 1; k <= 10; ++k)
				sum = sum + 4.0f / (k * pi) * sinf(pi * k * duty) * cos(2.0f * pi * k * frequency * t - pi * k * duty);

			data[i] = (2.0f * duty - 1 + sum);
		}

	}

	void PlayIfValid()
	{
		if (mPulse1LengthCounter != 0)
		{
			// Play Sound
			float duty = IntegerDutyCycleToFloat(mPulse1Duty);
			float amplitude = mPulse1Envelope.GetAmplitudeScalar();
			float frequency = mPulse1Sweep.GetFrequencyScalar();
			GeneratePulseWaveForm(duty, amplitude, frequency, mPulse1SoundData);
			AudioEngine.PlaySoundData(0.00416589088f, mPulse1SoundData);
		}
	}
};


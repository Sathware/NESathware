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

	/* Helper Functions */
	// Return decremented value clamped to 0
	static unsigned int ClampedDecrement(unsigned int val)
	{
		return (val == 0) ? 0 : (val - 1);
	}

	/* Helper Structs */
	struct AudioSwapBuffer
	{
		AudioSwapBuffer(uint32_t numElements)
			: mSoundData1(numElements), mSoundData2(numElements)
		{}

		std::vector<float> mSoundData1;
		std::vector<float> mSoundData2;

		bool mUseFirst = false;

		std::vector<float>& GetBuffer()
		{
			// Toggle flag to swap buffer
			mUseFirst = !mUseFirst;

			// Return buffer depending on flag
			if (mUseFirst)
				return mSoundData1;
			else
				return mSoundData2;
		}
	};

	// Controls the duration of a sound - refernce: "https://www.nesdev.org/wiki/APU_Length_Counter"
	struct LengthCounter
	{
		constexpr static unsigned int mDurationLookup[] =
		{
			10u, 254u, 20u, 2u, 40u, 4u, 80u, 6u, 160u, 8u, 60u, 10u, 14u, 12u, 26u, 14u,
			12u, 16u, 24u, 18u, 48u, 20u, 96u, 22u, 192u, 24u, 72u, 26u, 16u, 28u, 32u, 30u
		};

		unsigned int mCounter = 0;
		bool mHaltFlag;
		bool mEnabledFlag = false;

		void SetCounter(unsigned int val)
		{
			if (mEnabledFlag)
				mCounter = mDurationLookup[val];
			else
				mCounter = 0;
		}

		void Execute()
		{
			// If not enabled then counter is forced to 0
			if (!mEnabledFlag)
				mCounter = 0;
			// If halt flag is not set then try to decrement
			else if (!mHaltFlag)
				mCounter = ClampedDecrement(mCounter);
		}
	};
	
	struct PulseChannel
	{
		PulseChannel(Audio& AudioEngine)
			: mAudioInterface(AudioEngine.CreateSoundChannel())
		{
			mAudioInterface.Start();
		}

		float GetFrequency() const
		{
			return 1789773.0f / (16.0f * (float(mTimer) + 1.0f));
		}

		// Sample the waveform waveform to fill buffer with audio data, then add to play queue
		void QueueWaveform()
		{
			float duty = mDutyCycleLookup[mDutyCycle];
			float frequency = GetFrequency();

			// Fill sound data buffer with one cycle of waveform
			std::vector<float>& audioData = mSwapBuffer.GetBuffer();
			// Number of samples needed to capture one cycle of the waveform i.e. (1 / frequency) = time taken to complete 1 cycle and time * sampling_rate = number of samples 
			uint32_t numSamples = static_cast<uint32_t>(static_cast<float>(SoundChannel::sampleFrequency) / GetFrequency()) + 1;
			for (uint32_t i = 0; i < numSamples; ++i)
			{
				if (mLengthCounter.mCounter == 0)
				{
					audioData[i] = 0.0f;
				}
				else
				{
					float t = float(i) / float(SoundChannel::sampleFrequency);
					float sample = 0.05f * 2.0f * (floorf(0.5f + duty + frequency * t) - floorf(0.5f + frequency * t) - 0.5f);
					audioData[i] = sample;
				}
			}

			mAudioInterface.ExitLoop();
			mAudioInterface.ClearQueue();
			mAudioInterface.QueueSoundData(numSamples, audioData, true);
		}

		constexpr static float mDutyCycleLookup[] = { 0.125f, 0.25f, 0.50f, 0.75f };

		// Determines the duty of the pulse wave see: "https://thewolfsound.com/sine-saw-square-triangle-pulse-basic-waveforms-in-synthesis/"
		uint32_t mDutyCycle = 0;
		// Determines the frequency of the pulse wave see: "https://www.nesdev.org/wiki/APU_Pulse"
		uint32_t mTimer = 0;

		// Determines duration of notes
		LengthCounter mLengthCounter;

		/* Audio Generation */
		// Audio Engine Interface
		SoundChannel mAudioInterface;
		// Swap Buffer (2 audio buffers that swap) that can hold 1 second worth of sound samples at a sample rate of 44100
		AudioSwapBuffer mSwapBuffer = AudioSwapBuffer(SoundChannel::sampleFrequency);
	};

	/* APU frame counter clock */
	// Tracks elapsed CPU cycles **Note: elapsed APU cycles = elapsed CPU cycles / 2
	unsigned int mClockCount = 0;
	// Which frame sequencer mode to use
	bool mModeFlag = false;
	// Flag to invoke CPU IRQ for last step of frame counter sequence
	bool mInterruptFlag = false;

	///* Audio Generation Parameters */
	//const unsigned int mSamplingRate = 44100;

	//AudioSwapBuffer mSwapBuffer = AudioSwapBuffer(mSamplingRate); //44100 sampling rate for 1 second, or 44100 samples

	//std::vector<float> mSoundData = std::vector<float>(mSamplingRate); //44100 sampling rate for 1 second, or 44100 samples
	//std::vector<float> mPulse1SoundData = std::vector<float>(184); //44100 sampling rate for .00416589088 seconds give 184 samples

	/* Parameters for each sound channel */
	PulseChannel mPulse1 = PulseChannel(AudioEngine);
	PulseChannel mPulse2 = PulseChannel(AudioEngine);



	//Envelope (aka Volume) control unit
	//struct EnvelopeUnit
	//{
	//	EnvelopeUnit(const ubyte& volumePeriod, const bool& loopFlag, const bool& constantFlag)
	//		: volumePeriod(volumePeriod), loopFlag(loopFlag), constantFlag(constantFlag)
	//	{}
	//	const ubyte& volumePeriod;
	//	const bool& loopFlag;
	//	const bool& constantFlag;

	//	bool startFlag = true;
	//	ubyte dividerCounter = 0;
	//	ubyte decayCounter = 0;

	//	// Update volume parameters by one step
	//	void Execute()
	//	{
	//		// Case 1: start flag is set
	//		if (startFlag)
	//		{
	//			startFlag = false;
	//			decayCounter = 15;
	//			dividerCounter = volumePeriod;
	//			return;
	//		}

	//		// Case 2: start flag is not set
	//		if (dividerCounter == 0)
	//		{
	//			dividerCounter = volumePeriod;
	//			if (loopFlag)
	//				decayCounter = (decayCounter == 0) ? 15 : (decayCounter - 1);
	//			else
	//				decayCounter = (decayCounter == 0) ? 0 : (decayCounter - 1);
	//		}
	//		else
	//			dividerCounter = dividerCounter - 1;
	//	}

	//	float GetAmplitudeScalar()
	//	{
	//		if (constantFlag)
	//			return float(volumePeriod) / 15.0f;
	//		else
	//			return float(decayCounter) / 15.0f;
	//	}
	//};

	//struct SweepUnit
	//{
	//	SweepUnit(ubyte& sweepPeriod, ubyte2& timerPeriod, ubyte& shiftCounter, bool& negateFlag)
	//		: sweepPeriod(sweepPeriod), timerPeriod(timerPeriod), shiftCounter(shiftCounter), negateFlag(negateFlag)
	//	{}
	//	const ubyte& sweepPeriod;
	//	ubyte2& timerPeriod;
	//	const ubyte& shiftCounter;
	//	const bool& negateFlag;

	//	bool reloadFlag = false;
	//	ubyte dividerCounter = 0;

	//	void Execute()
	//	{
	//		if (reloadFlag)
	//			dividerCounter = sweepPeriod;

	//		if (dividerCounter == 0)
	//		{
	//			reloadFlag = false;
	//			dividerCounter = sweepPeriod;
	//			ubyte2 change = timerPeriod >> shiftCounter;
	//			sbyte2 temp = (negateFlag ? (-sbyte2(change) - 1) : sbyte2(change)) + sbyte2(timerPeriod);
	//			timerPeriod = (temp < 0) ? 0 : ubyte2(temp);
	//		}
	//		dividerCounter = dividerCounter - 1;
	//	}

	//	float GetFrequencyScalar()
	//	{
	//		return 1789773.0f / (16.0f * (float(timerPeriod) + 1));
	//	}
	//};

	// Pulse 1 //
	//ubyte mPulse1Duty = 0;
	//bool mPulse1LengthCounterFlag = false;
	//bool mPulse1ConstantVolumeFlag = false;
	//ubyte mPulse1VolumeDividerPeriod = 0;
	//bool mPulse1SweepFlag = false;
	//ubyte mPulse1SweepDividerPeriod = 0;
	//bool mPulse1SweepNegateFlag = false;
	//ubyte mPulse1SweepShiftCounter = 0;
	//ubyte2 mPulse1Timer = 0;
	//ubyte mPulse1LengthCounter = 0;
	//EnvelopeUnit mPulse1Envelope = EnvelopeUnit(mPulse1VolumeDividerPeriod, mPulse1LengthCounterFlag, mPulse1ConstantVolumeFlag);
	//SweepUnit mPulse1Sweep = SweepUnit(mPulse1SweepDividerPeriod, mPulse1Timer, mPulse1SweepShiftCounter, mPulse1SweepNegateFlag);


	//float IntegerDutyCycleToFloat(ubyte duty)
	//{
	//	if (duty == 0)
	//		return .125f;
	//	if (duty == 1)
	//		return .25f;
	//	if (duty == 2)
	//		return .50f;
	//	if (duty == 3)
	//		return .75f;
	//}

	//void GeneratePulseWaveForm(const float duty, const float amplitude, const float frequency, std::vector<float>& data)
	//{
	//	constexpr float pi = float(std::numbers::pi);
	//	// Fixed sound length is 0.00416589088f
	//	for (unsigned int i = 0; i < 184; ++i)
	//	{
	//		float t = 0.00416589088f * float(i) / float(184);
	//		float sum = 0.0f;
	//		for (unsigned int k = 1; k <= 10; ++k)
	//			sum = sum + 4.0f / (k * pi) * sinf(pi * k * duty) * cos(2.0f * pi * k * frequency * t - pi * k * duty);

	//		data[i] = (2.0f * duty - 1 + sum);
	//	}

	//}

	//void PlayIfValid()
	//{
	//	if (mPulse1LengthCounter != 0)
	//	{
	//		// Play Sound
	//		float duty = IntegerDutyCycleToFloat(mPulse1Duty);
	//		float amplitude = mPulse1Envelope.GetAmplitudeScalar();
	//		float frequency = mPulse1Sweep.GetFrequencyScalar();
	//		GeneratePulseWaveForm(duty, amplitude, frequency, mPulse1SoundData);
	//		AudioEngine.PlaySoundData(0.00416589088f, mPulse1SoundData);
	//	}
	//}
};


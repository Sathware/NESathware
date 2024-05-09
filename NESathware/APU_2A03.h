#pragma once
#include "CommonTypes.h"
#include <numbers>
#include <vector>
#include <algorithm>
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
		LengthCounter(bool& haltFlag)
			: mHaltFlag(haltFlag)
		{}

		constexpr static unsigned int mDurationLookup[] =
		{
			10u, 254u, 20u, 2u, 40u, 4u, 80u, 6u, 160u, 8u, 60u, 10u, 14u, 12u, 26u, 14u,
			12u, 16u, 24u, 18u, 48u, 20u, 96u, 22u, 192u, 24u, 72u, 26u, 16u, 28u, 32u, 30u
		};

		unsigned int mCounter = 0;
		const bool& mHaltFlag;
		bool mEnabledFlag = false;

		void SetCounter(unsigned int val)
		{
			// If disabled then counter value should be stuck at 0
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

	// Controls the volume of a sound - reference: "https://www.nesdev.org/wiki/APU_Envelope"
	struct EnvelopeUnit
	{
		EnvelopeUnit(bool& loopFlag)
			: mLoopFlag(loopFlag)
		{}

		// NES registers
		unsigned int mVolume = 0;
		const bool& mLoopFlag;
		bool mConstantVolumeFlag = false;

		// Internal registers
		unsigned int mDivider = 0;
		unsigned int mCounter = 0;
		bool mStartFlag = false;

		void Execute()
		{
			// If start flag is set
			if (mStartFlag)
			{
				mStartFlag = false;
				mCounter = 15;
				mDivider = mVolume;
				return;
			}
			// If start flag is not set
			if (mDivider == 0)
			{
				// Divider is being clocked
				mDivider = mVolume;

				if (mCounter == 0 && mLoopFlag)
					mCounter = 15;
				else
					mCounter = ClampedDecrement(mCounter);
			}
			else
				// Divider is implemented as down counter
				mDivider = mDivider - 1;
		}

		float GetAmplitude()
		{
			if (mConstantVolumeFlag)
				return float(mVolume) / 16.0f;
			else
				return float(mCounter) / 16.0f;
		}
	};

	struct SweepUnit
	{
		SweepUnit(uint32_t& timer, bool twosComplement)
			: mTimer(timer), twosComplement(twosComplement)
		{}

		// NES Registers
		bool mEnabledFlag = false;
		uint32_t mDividerPeriod = 0;
		bool mNegateFlag = false;
		uint32_t mShiftCount = 0;

		uint32_t& mTimer;

		// Internal Registers
		uint32_t mDividerCounter = 0;
		bool mReloadFlag = false;
		const bool twosComplement;

		uint32_t GetTargetPeriod()
		{
			uint32_t delta = mTimer >> mShiftCount;
			
			if (mNegateFlag)
				delta = ~delta + (twosComplement ? 1 : 0); // Add one after inverting if two's complement otherwise don't add anything

			uint32_t targetPeriod = mTimer + delta;
			if (IsBitOn<10>(static_cast<ubyte2>(targetPeriod)))
				targetPeriod = 0;

			return targetPeriod;
		}

		bool IsMuting()
		{
			return (mTimer < 8) || (GetTargetPeriod() > 0x7FF);
		}

		void Execute()
		{
			// If reload flag is set (i.e. by writing to sweep registers) then reload counter with period
			if (mReloadFlag || (mDividerCounter == 0))
			{
				mReloadFlag = false;
				mDividerCounter = mDividerPeriod;
			}
			else
				mDividerCounter = mDividerCounter - 1;

			if ((mDividerCounter == 0) && mEnabledFlag && (mShiftCount > 0) && !IsMuting())
				mTimer = GetTargetPeriod();
		}
	};
	
	struct PulseChannel
	{
		PulseChannel(Audio& AudioEngine, bool twoComplementShift)
			: mAudioInterface(AudioEngine.CreateSoundChannel()), mSweepUnit(mTimer, twoComplementShift)
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
			float amplitude = mEnvelopeUnit.GetAmplitude();

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
					audioData[i] = amplitude * sample;
				}
			}

			mAudioInterface.ExitLoop();
			mAudioInterface.ClearQueue();
			mAudioInterface.QueueSoundData(numSamples, audioData, true);
		}

		constexpr static float mDutyCycleLookup[] = { 0.125f, 0.25f, 0.50f, 0.75f };

		/* Registers */
		// Determines the duty of the pulse wave see: "https://thewolfsound.com/sine-saw-square-triangle-pulse-basic-waveforms-in-synthesis/"
		uint32_t mDutyCycle = 0;
		// Determines the frequency of the pulse wave see: "https://www.nesdev.org/wiki/APU_Pulse"
		uint32_t mTimer = 0;
		// Determines whether LengthCounter is halted and if envelope loop is enabled
		bool mLengthCounterHalt = false;

		/* Sub Components */
		// Determines duration of notes
		LengthCounter mLengthCounter = LengthCounter(mLengthCounterHalt);
		// Determines volume of notes
		EnvelopeUnit mEnvelopeUnit = EnvelopeUnit(mLengthCounterHalt);
		// Determine the pitch of notes
		SweepUnit mSweepUnit;

		/* Audio Generation */
		// Audio Engine Interface
		SoundChannel mAudioInterface;
		// Swap Buffer (2 audio buffers that swap) that can hold 1 second worth of sound samples at a sample rate of 44100
		AudioSwapBuffer mSwapBuffer = AudioSwapBuffer(SoundChannel::sampleFrequency);
	};

	struct LinearCounter
	{
		LinearCounter(bool& controlFlag)
			: mControlFlag(controlFlag)
		{}

		unsigned int mCounter = 0;
		unsigned int mCounterReload = 0;
		bool mCounterReloadFlag = false;
		const bool& mControlFlag;

		void Execute()
		{
			if (mCounterReloadFlag)
				mCounter = mCounterReload;
			else
				mCounter = ClampedDecrement(mCounter);

			if (!mControlFlag)
				mCounterReloadFlag = false;
		}
	};

	struct TriangleChannel
	{
		TriangleChannel(Audio& AudioEngine)
			: mAudioInterface(AudioEngine.CreateSoundChannel()), mCrossAudio(AudioEngine.CreateSoundChannel())
		{
			mAudioInterface.Start();
			mCrossAudio.Start();
		}

		float GetFrequency() const
		{
			return 1789773.0f / (32.0f * (float(mTimer) + 1.0f));
		}

		uint32_t mTimer = 0;
		bool mControlFlag = false;

		LinearCounter mLinearCounter = LinearCounter(mControlFlag);
		LengthCounter mLengthCounter = LengthCounter(mControlFlag);

		// Sample the waveform waveform to fill buffer with audio data, then add to play queue
		void QueueWaveform()
		{
			float frequency = GetFrequency();
			float pi = static_cast<float>(std::numbers::pi);
			// Fill sound data buffer with one cycle of waveform
			std::vector<float>& audioData = mSwapBuffer.GetBuffer();
			// Number of samples needed to capture one cycle of the waveform i.e. (1 / frequency) = time taken to complete 1 cycle and time * sampling_rate = number of samples 
			uint32_t numSamples = static_cast<uint32_t>(static_cast<float>(SoundChannel::sampleFrequency) / GetFrequency());

			// Apply cross fade for ~10 milliseconds
			std::vector<float>& crossFadeData = mCrossFadeBuffer.GetBuffer();
			uint32_t numSamplesForTenMilliSeconds = uint32_t(0.01f * SoundChannel::sampleFrequency);
			uint32_t numSamplesForCrossFade = 0;
			if (numSamples > 0)
				numSamplesForCrossFade = numSamples - (numSamplesForTenMilliSeconds % numSamples) + numSamplesForTenMilliSeconds; //Adjust number of samples to end on zero crossing of new frequency

			if (mTimer == 0 || mLengthCounter.mCounter == 0 || mLinearCounter.mCounter == 0)
			{
				for (uint32_t i = 0; i < numSamples; ++i)
					audioData[i] = 0.0f;

				for (uint32_t i = 0; i < numSamplesForCrossFade; ++i)
				{
					/*float t = float(i) / float(SoundChannel::sampleFrequency);
					float sample = 4 * fabsf(frequency * t + 0.25f - floorf(frequency * t + 0.75f)) - 1;
					float fade = static_cast<float>(numSamplesForCrossFade - i) / (SoundChannel::sampleFrequency);
					crossFadeData[i] = 0.25f * sample * fade;*/
					float t = float(i) / float(SoundChannel::sampleFrequency);
					float sample = 4 * fabsf(mPrevFrequency * t + 0.25f - floorf(mPrevFrequency * t + 0.75f)) - 1;
					//float vlmcntr = sinf(0.5 * pi / float(numSamples) * (float(i) + 0.5f));

					float sampleRate = SoundChannel::sampleFrequency;
					float envelope = static_cast<float>(numSamplesForCrossFade - i) / (sampleRate * 0.01f);

					crossFadeData[i] = 0.25f * sample * envelope;
				}
			}
			else
			{
				for (uint32_t i = 0; i < numSamples; ++i)
				{
					float t = float(i) / float(SoundChannel::sampleFrequency);
					float sample = 4 * fabsf(frequency * t + 0.25f - floorf(frequency * t + 0.75f)) - 1;
					//float vlmcntr = sinf(0.5 * pi / float(numSamples) * (float(i) + 0.5f));

					//float sampleRate = SoundChannel::sampleFrequency;
					/*float envelope = (i < sampleRate * 0.01f) ? (static_cast<float>(i) / (sampleRate * 0.01f)) :
						(i > numSamples * 3 - sampleRate * 0.01f) ? (static_cast<float>(numSamples * 3 - i) / (sampleRate * 0.01f)) : 1.0f;*/
					audioData[i] = 0.25f * sample;
				}

				for (uint32_t i = 0; i < numSamplesForCrossFade; ++i)
				{
					float t = float(i) / float(SoundChannel::sampleFrequency);
					float sample = 4 * fabsf(mPrevFrequency * t + 0.25f - floorf(mPrevFrequency * t + 0.75f)) - 1;
					//float vlmcntr = sinf(0.5 * pi / float(numSamples) * (float(i) + 0.5f));

					float sampleRate = SoundChannel::sampleFrequency;
					float envelope = static_cast<float>(numSamplesForCrossFade - i) / (sampleRate * 0.01f);

					crossFadeData[i] = 0.25f * sample * envelope;
				}

				mPrevFrequency = frequency;

				/*for (uint32_t i = 0; i < numSamplesForCrossFade; ++i)
				{
					float t = float(i) / float(SoundChannel::sampleFrequency);
					float samplePrevFreq = 4.0f * fabsf(mPrevFrequency * t + 0.25f - floorf(mPrevFrequency * t + 0.75f)) - 1;
					float sampleCurrFreq = 4.0f * fabsf(frequency * t + 0.25f - floorf(frequency * t + 0.75f)) - 1;

					float a = static_cast<float>(i) / static_cast<float>(numSamplesForCrossFade);

					float out = samplePrevFreq * (1 - a) + sampleCurrFreq * a;
					crossFadeData[i] = 0.25f * out;
				}
				mPrevFrequency = frequency;*/
			}
			//else if (std::fabsf(mPrevFrequency - frequency) > .00001)
			//{
			//	for (uint32_t i = 0; i < numSamples; ++i)
			//	{
			//		float t = float(i) / float(SoundChannel::sampleFrequency);
			//		float sample = 4 * fabsf(frequency * t + 0.25f - floorf(frequency * t + 0.75f)) - 1;
			//		//float vlmcntr = sinf(0.5 * pi / float(numSamples) * (float(i) + 0.5f));
			//		audioData[i] = 0.25f * sample;
			//	}

			//	for (int i = 0; i < numSamplesForCrossFade; ++i)
			//	{
			//		float t = float(i) / float(SoundChannel::sampleFrequency);
			//		float samplePrevFreq = 4 * fabsf(mPrevFrequency * t + 0.25f - floorf(mPrevFrequency * t + 0.75f)) - 1;
			//		float sampleCurrFreq = 4 * fabsf(frequency * t + 0.25f - floorf(frequency * t + 0.75f)) - 1;

			//		float a = (i < SoundChannel::sampleFrequency * 0.1f) ? (static_cast<float>(i) / (SoundChannel::sampleFrequency * 0.1f)) :
			//			(i > numSamplesForCrossFade - SoundChannel::sampleFrequency * 0.1f) ? (static_cast<float>(numSamplesForCrossFade - i) / (SoundChannel::sampleFrequency * 0.1f)) : 1.0f;

			//		crossFadeData[i] = 0.25f * (samplePrevFreq * (1 - a) + sampleCurrFreq * a);
			//	}
			//	mPrevFrequency = frequency;
			//}

			mAudioInterface.ClearQueue();
			mCrossAudio.ClearQueue();
			mCrossAudio.QueueSoundData(numSamplesForCrossFade, crossFadeData, false);
			mAudioInterface.QueueSoundData(numSamples, audioData, true);
			mAudioInterface.ExitLoop();
		}

		/* Audio Generation */
		float mPrevFrequency = 0.0f;
		SoundChannel mCrossAudio;
		AudioSwapBuffer mCrossFadeBuffer = AudioSwapBuffer(SoundChannel::sampleFrequency); //Holds 1 second worth of audio data at a sample rate of 44100
		// Audio Engine Interface
		SoundChannel mAudioInterface;
		// Swap Buffer (2 audio buffers that swap) that can hold 1 second worth of sound samples at a sample rate of 44100
		AudioSwapBuffer mSwapBuffer = AudioSwapBuffer(SoundChannel::sampleFrequency);
	};

	struct NoiseChannel
	{
		bool mLengthCounterHaltFlag = false;
		bool mEnvelopeFlag = false;
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
	PulseChannel mPulse1 = PulseChannel(AudioEngine, false);
	PulseChannel mPulse2 = PulseChannel(AudioEngine, true);
	TriangleChannel mTriangle = TriangleChannel(AudioEngine);


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


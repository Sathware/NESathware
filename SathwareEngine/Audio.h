#pragma once
#include <xaudio2.h>
#include <wrl.h>
#include "SathwareException.h"
#include "SathwareEngine.h"
#include <vector>

class SathwareAPI SoundChannel
{
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
	SoundChannel(ComPtr<IXAudio2>& XAudio2Interface);

	// Start the source voice
	void Start()
	{
		HRESULT result = mSourceVoice->Start();
		ThrowIfFailed(result, L"Failed call to Start on source voice!");
	}

	// Stop the source voice
	void Stop()
	{
		HRESULT result = mSourceVoice->Stop();
		ThrowIfFailed(result, L"Failed call to Stop on source voice!");
	}

	// Stop looping currently playing buffer, but after finishing the current loop
	void ExitLoop()
	{
		HRESULT result = mSourceVoice->ExitLoop();
		ThrowIfFailed(result, L"Failed call to ExitLoop on source voice!");
	}

	// Queue audio sample buffer to be played
	void QueueSoundData(const uint32_t numSamples, const std::vector<float>& data, bool loop);

	// Clear queued sample buffers except the buffer currently being played 
	void ClearQueue()
	{
		// Clear all pending buffers in queue (does not affect currently playing buffer)
		HRESULT result = mSourceVoice->FlushSourceBuffers();
		ThrowIfFailed(result, L"Failed call to FlushSourceBuffers to clear sound queue!");
	}

	static constexpr unsigned int bitsPerSample = 32;
	static constexpr unsigned int sampleFrequency = 44100;
private:
	IXAudio2SourceVoice* mSourceVoice;
};

class SathwareAPI Audio
{
public:
	Audio();

	SoundChannel CreateSoundChannel()
	{
		return SoundChannel(mXAudio2);
	}

	/*void PlaySine();
	void PlaySoundData(const uint32_t numSamples, const std::vector<float>& data);*/

	Audio(const Audio& other) = delete;
	Audio(const Audio&& other) = delete;
	Audio& operator=(const Audio& other) = delete;
private:
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	ComPtr<IXAudio2> mXAudio2;
	IXAudio2MasteringVoice* mMasteringVoice;

	/*IXAudio2SourceVoice* mSourceVoice;
	static constexpr unsigned int bitsPerSample = 32;
	static constexpr unsigned int sampleFrequency = 44100;
	static constexpr unsigned int numBytes = bitsPerSample * 5 * sampleFrequency;
	float soundData[5 * sampleFrequency];*/
};
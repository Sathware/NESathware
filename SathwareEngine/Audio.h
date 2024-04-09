#pragma once
#include <xaudio2.h>
#include <wrl.h>
#include "SathwareException.h"
#include "SathwareEngine.h"

class SathwareAPI Audio
{
public:
	Audio();

	void PlaySine();

	Audio(const Audio& other) = delete;
	Audio(const Audio&& other) = delete;
	Audio& operator=(const Audio& other) = delete;
private:
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	ComPtr<IXAudio2> mXAudio2;
	IXAudio2MasteringVoice* mMasteringVoice;
	IXAudio2SourceVoice* mSourceVoice;
	static constexpr unsigned int bitsPerSample = 32;
	static constexpr unsigned int sampleFrequency = 44100;
	static constexpr unsigned int numBytes = bitsPerSample * 5 * sampleFrequency;
	float soundData[5 * sampleFrequency];
};
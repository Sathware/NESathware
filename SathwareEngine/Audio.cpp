#include "Audio.h"
#include <numbers>
#include <random>

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

Audio::Audio()
{
	HRESULT result;
	result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	ThrowIfFailed(result, L"Failed call to CoInitializeEx to initialize COM!");

	result = XAudio2Create(&mXAudio2);
	ThrowIfFailed(result, L"Failed call to XAudio2Create to instantiate the xaudio2 engine!");

	result = mXAudio2->CreateMasteringVoice(&mMasteringVoice);

	// Create a source voice
	WAVEFORMATEX waveformat;
	waveformat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
	waveformat.nChannels = 1;
	waveformat.nSamplesPerSec = sampleFrequency;
	waveformat.nAvgBytesPerSec = sampleFrequency * bitsPerSample / 8;
	waveformat.nBlockAlign = bitsPerSample / 8;
	waveformat.wBitsPerSample = bitsPerSample;
	waveformat.cbSize = 0;

	result = mXAudio2->CreateSourceVoice(&mSourceVoice, &waveformat);
	ThrowIfFailed(result, L"Failed call to CreateSourceVoice to instantiate a source voice!");

	// Start the source voice
	result = mSourceVoice->Start();

	// Fill audio data
	for (unsigned int i = 0; i < sampleFrequency * 5; ++i)
	{
		float t = float(i) / 44100.0f;
		float freq = 220.0f;
		constexpr float pi2 = 2.0f * float(std::numbers::pi);
		// Reference: "https://thewolfsound.com/sine-saw-square-triangle-pulse-basic-waveforms-in-synthesis/#sine"
		soundData[i] = sinf(2.0f * float(std::numbers::pi) * freq * t);
		//soundData[i] = 0.1f * sgn(sinf(2.0f * float(std::numbers::pi) * freq * t));
		//soundData[i] = 4.0f * fabs(freq*t - floorf(freq*t + 0.5f)) - 1.0f;
	}
}

void Audio::PlaySine()
{
	XAUDIO2_BUFFER buffer = { 0 };
	buffer.AudioBytes = 5 * sampleFrequency * bitsPerSample / 8;
	buffer.pAudioData = reinterpret_cast<byte*>(&soundData);
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.PlayBegin = 0;
	buffer.PlayLength = 5 * 44100;

	HRESULT result = mSourceVoice->SubmitSourceBuffer(&buffer);
	ThrowIfFailed(result, L"Failed call to SubmitSourceBuffer to play sound!");
}

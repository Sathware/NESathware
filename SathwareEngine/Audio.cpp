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

#if defined(DEBUG) || defined(_DEBUG)
	XAUDIO2_DEBUG_CONFIGURATION config = { 0 };
	config.TraceMask = XAUDIO2_LOG_ERRORS;
	config.BreakMask = XAUDIO2_LOG_ERRORS;

	mXAudio2->SetDebugConfiguration(&config);
#endif

	result = mXAudio2->CreateMasteringVoice(&mMasteringVoice);

	//// Create a source voice
	//WAVEFORMATEX waveformat;
	//waveformat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
	//waveformat.nChannels = 1;
	//waveformat.nSamplesPerSec = sampleFrequency;
	//waveformat.nAvgBytesPerSec = sampleFrequency * bitsPerSample / 8;
	//waveformat.nBlockAlign = bitsPerSample / 8;
	//waveformat.wBitsPerSample = bitsPerSample;
	//waveformat.cbSize = 0;

	//result = mXAudio2->CreateSourceVoice(&mSourceVoice, &waveformat);
	//ThrowIfFailed(result, L"Failed call to CreateSourceVoice to instantiate a source voice!");

	//// Start the source voice
	//result = mSourceVoice->Start();
	//ThrowIfFailed(result, L"Failed call to Start on source voice!");

	//// Fill audio data
	//for (unsigned int i = 0; i < sampleFrequency * 5; ++i)
	//{
	//	float t = float(i) / 44100.0f; // The time in seconds
	//	float freq = 220.0f;
	//	constexpr float pi2 = 2.0f * float(std::numbers::pi);
	//	// Reference: "https://thewolfsound.com/sine-saw-square-triangle-pulse-basic-waveforms-in-synthesis/#sine"
	//	soundData[i] = sinf(2.0f * float(std::numbers::pi) * freq * t);
	//	//soundData[i] = 0.1f * sgn(sinf(2.0f * float(std::numbers::pi) * freq * t));
	//	//soundData[i] = 4.0f * fabs(freq*t - floorf(freq*t + 0.5f)) - 1.0f;
	//}
}

//void Audio::PlaySine()
//{
//	XAUDIO2_BUFFER buffer = { 0 };
//	buffer.AudioBytes = 5 * sampleFrequency * bitsPerSample / 8;
//	buffer.pAudioData = reinterpret_cast<byte*>(&soundData);
//	buffer.Flags = XAUDIO2_END_OF_STREAM;
//	buffer.PlayBegin = 0;
//	buffer.PlayLength = 5 * 44100;
//
//	HRESULT result = mSourceVoice->SubmitSourceBuffer(&buffer);
//	ThrowIfFailed(result, L"Failed call to SubmitSourceBuffer to play sound!");
//}
//
//void Audio::PlaySoundData(const uint32_t numSamples, const std::vector<float>& data)
//{
//	////Clear previous audio data
//	//result = mSourceVoice->FlushSourceBuffers();
//	//ThrowIfFailed(result, L"Failed call to FlushSourceBuffers on source voice to clear audio data!");
//	////Start voice again
//	//result = mSourceVoice->Start();
//	//ThrowIfFailed(result, L"Failed call to Start on source voice!");
//
//	// Stop playing the previous sound, but let it finish its current cycle
//	/*HRESULT result = mSourceVoice->Stop();
//	ThrowIfFailed(result, L"Failed call to Stop on source voice to stop sound!");*/
//
//	// Clear all pending buffers in queue (does not affect currently playing buffer)
//	/*HRESULT result = mSourceVoice->FlushSourceBuffers();
//	ThrowIfFailed(result, L"Failed call to Stop on source voice to stop sound!");*/
//
//	// Push new data into voice queue
//	HRESULT result = mSourceVoice->ExitLoop();
//	ThrowIfFailed(result, L"Failed call to exit loop on current playing sound!");
//	// Clear all pending buffers in queue (does not affect currently playing buffer)
//	result = mSourceVoice->FlushSourceBuffers();
//	ThrowIfFailed(result, L"Failed call to FlushSourceBuffers to clear sound queue!");
//
//	// Specify new data to be queued for voice
//	XAUDIO2_BUFFER buffer = { 0 };
//	buffer.AudioBytes = data.size() * bitsPerSample / 8;
//	buffer.pAudioData = reinterpret_cast<const byte*>(data.data());
//	buffer.PlayBegin = 0;
//	buffer.PlayLength = numSamples;
//	buffer.LoopBegin = 0;
//	buffer.LoopLength = numSamples;
//	buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
//
//	// Push new data into voice queue
//	result = mSourceVoice->SubmitSourceBuffer(&buffer);
//	ThrowIfFailed(result, L"Failed call to SubmitSourceBuffer to play sound!");
//}

//void Audio::PlayWaveForm(float seconds, std::vector<float> buffer);

SoundChannel::SoundChannel(ComPtr<IXAudio2>& XAudio2Interface)
{
	// Create a source voice
	WAVEFORMATEX waveformat;
	waveformat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
	waveformat.nChannels = 1;
	waveformat.nSamplesPerSec = sampleFrequency;
	waveformat.nAvgBytesPerSec = sampleFrequency * bitsPerSample / 8;
	waveformat.nBlockAlign = bitsPerSample / 8;
	waveformat.wBitsPerSample = bitsPerSample;
	waveformat.cbSize = 0;

	HRESULT result = XAudio2Interface->CreateSourceVoice(&mSourceVoice, &waveformat);
	ThrowIfFailed(result, L"Failed call to CreateSourceVoice to instantiate a source voice!");
}

void SoundChannel::QueueSoundData(const uint32_t numSamples, const std::vector<float>& data, bool loop)
{
	// Specify new data to be queued for voice
	XAUDIO2_BUFFER buffer = { 0 };
	buffer.AudioBytes = data.size() * bitsPerSample / 8;
	buffer.pAudioData = reinterpret_cast<const byte*>(data.data());
	buffer.PlayBegin = 0;
	buffer.PlayLength = numSamples;
	if (loop)
	{
		buffer.LoopBegin = 0;
		buffer.LoopLength = numSamples;
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	// Push new data into voice queue
	HRESULT result = mSourceVoice->SubmitSourceBuffer(&buffer);
	ThrowIfFailed(result, L"Failed call to SubmitSourceBuffer to play sound!");
}

#include "NES.h"
#include "../SathwareEngine/SathwareEngine.h"
#include "../SathwareEngine/DesktopWindow.h"
#include "../SathwareEngine/Graphics.h"
#include "../SathwareEngine/Audio.h"
#include "../SathwareEngine/Timer.h"
#include <iostream>
#include <iomanip>
#include <numbers>

int main()
{
    try
    {
        HINSTANCE SathwareAPI dllInstance;

        if (dllInstance == nullptr)
            throw std::runtime_error("Could not get DLL Handle");

        DesktopWindow desktopWindow(256u, 240u, dllInstance, SW_NORMAL);
        Graphics directXGFX(desktopWindow);
        Audio xAudio2Engine;
        NES nes("DuckHunt.nes", directXGFX, xAudio2Engine, desktopWindow);
        Timer timer;




        /*nes.mPPU.DisplayCHRROM();
        directXGFX.Render();*/

        //directXGFX.ClearBuffer();
        //// Display waveform
        //for (unsigned int x = 0; x < 256u; ++x)
        //{
        //    float t = 2 * float(x) / float(256u) / 220.0f;
        //    float sample = 4 * fabsf(frequency * t + 0.25f - floorf(frequency * t + 0.75f)) - 1;
        //    // Taper beginning and end of sound
        //    // float vlmcntr = sinf(0.5 * pi / float(numSamples) * (float(i) + 0.5f));
        //    int y = int(0.25f * sample * 240u + 120u);
        //    directXGFX.PutPixel(x, y, Color::White);
        //    directXGFX.PutPixel(x, 120u, Color::Red);
        //}
        //directXGFX.Render();

        //auto channel = xAudio2Engine.CreateSoundChannel();
        //float frequency = 10*220.0f;
        //uint32_t numSamples = static_cast<uint32_t>(static_cast<float>(SoundChannel::sampleFrequency) / frequency) + 1;
        //auto audioData = std::vector<float>(SoundChannel::sampleFrequency);
        //float pi = static_cast<float>(std::numbers::pi);

        //unsigned int a = 20;
        //unsigned int b = 120;
        //// Maintain Volume (Loop Region)
        //for (uint32_t i = 0; i < numSamples * b; ++i)
        //{
        //    float t = float(i) / float(SoundChannel::sampleFrequency);
        //    t = t + 1 / (4 * frequency); // Shift Triangle wave to start and end on a zero
        //    float sample = 4 * fabsf(frequency * t - floorf(frequency * t + 0.5f)) - 1;
        //    // Taper beginning and end of sound
        //    // float vlmcntr = sinf(0.5 * pi / float(numSamples) * (float(i) + 0.5f));
        //    // Apply smooth envelope (fade in/out)
            /*float sampleRate = SoundChannel::sampleFrequency;
            float envelope = (i < sampleRate * 0.01f) ? (static_cast<float>(i) / (sampleRate * 0.01f)) :
                (i > numSamples * b - sampleRate * 0.01f) ? (static_cast<float>(numSamples * b - i) / (sampleRate * 0.01f)) : 1.0f;*/

        //    audioData[i] = 0.25f * sample * envelope;
        //}

        //// Ramp Up volume
        //for (uint32_t i = 0; i < numSamples; ++i)
        //{
        //  float t = float(i) / float(SoundChannel::sampleFrequency);
        //  t = t + 1/(4 * frequency); // Shift Triangle wave to start and end on a zero
        //  float sample = 4 * fabsf(frequency * t - floorf(frequency * t + 0.5f)) - 1;
        //  // Taper beginning and end of sound
        //  float vlmcntr = float(i) / float(numSamples); // sinf(0.5 * pi / float(numSamples) * (float(i) + 0.5f));

        //  audioData[i] = vlmcntr * 0.25f * sample;
        //}
        // 
        //// Ramp down volume
        //for (uint32_t i = 0; i < numSamples; ++i)
        //{
        //    float t = float(i) / float(SoundChannel::sampleFrequency);
        //    t = t + 1 / (4 * frequency); // Shift Triangle wave to start and end on a zero
        //    float sample = 4 * fabsf(frequency * t - floorf(frequency * t + 0.5f)) - 1;
        //    // Taper beginning and end of sound
        //    float vlmcntr = 1.0f - float(i) / float(numSamples); //sinf(0.5 * pi / float(numSamples) * (float(i) + 0.5f));

        //    audioData[i + 2 * numSamples] = vlmcntr * 0.25f * sample;
        //}
        // audioData[numSamples - 1] = 0.0f;
        /*channel.QueueSoundData(SoundChannel::sampleFrequency, audioData, true);
        channel.Start();*/

        while (desktopWindow.IsRunning())
        {
            nes.Run(timer.GetElapsedSeconds());
        }
    }
    catch (Exception& e)
    {
        MessageBoxExW(nullptr, e.what(), nullptr, MB_OK, 0);
        throw e;
    }
    catch (std::runtime_error& e)
    {
        MessageBoxA(nullptr, e.what(), nullptr, MB_OK);
        throw e;
    }

    return 0;
}
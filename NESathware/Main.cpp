#include "NES.h"
#include "../SathwareEngine/SathwareEngine.h"
#include "../SathwareEngine/DesktopWindow.h"
#include "../SathwareEngine/Graphics.h"
#include "../SathwareEngine/Timer.h"

static bool IsBitOn(unsigned int bit, ubyte val)
{
    return (val & (1u << bit)) != 0u;
}

//void DisplayCHRROM(Mapper* cartridge, Graphics& gfx)
//{
//    for (unsigned int patternTableIndex = 0; patternTableIndex <= 255; ++patternTableIndex)
//    {
//        //Display tile
//        for (unsigned int tileRow = 0; tileRow < 8; ++tileRow)
//        {
//            unsigned int patternLow = cartridge->ReadPPU(0x1000 + patternTableIndex * 16 + tileRow);
//            unsigned int patternHigh = cartridge->ReadPPU(0x1000 + patternTableIndex * 16 + 8 + tileRow);
//            unsigned int pattern = patternLow | patternHigh;
//            for (unsigned int bit = 0; bit < 8; ++bit)
//            {
//                unsigned int x = (patternTableIndex % 32) * 8 + (7 - bit);
//                unsigned int y = (patternTableIndex / 32) * 8 + tileRow;
//                if (IsBitOn(bit, pattern))
//                {
//                    gfx.PutPixel(x, y, Color::White);
//                }
//            }
//        }
//    }
//}

int main()
{
    try
    {
        HINSTANCE SathwareAPI dllInstance;

        if (dllInstance == nullptr)
            throw std::runtime_error("Could not get DLL Handle");

        DesktopWindow desktopWindow(256u, 240u, dllInstance, SW_NORMAL);
        Graphics directXGFX(desktopWindow);
        Timer timer;

        NES nes("backgrounds.nes", directXGFX);

        /*DisplayCHRROM(nes.mpCartridge.get(), directXGFX);
        directXGFX.Render();*/

        while (desktopWindow.IsRunning())
        {
            //Display CHR ROM
            

            //Timer FrameTimer;
            //directXGFX.Clear();

            nes.Run(timer.GetElapsedSeconds());

            //for (int x = 0; x < 100; ++x)
            //    directXGFX.PutPixel(x, 100, Color(255, 255, 255, 255));

            //directXGFX.Render();
            //OutputDebugStringW(std::to_wstring(FrameTimer.GetElapsedSeconds()).c_str());
        }
    }
    catch (Exception& e)
    {
        MessageBoxExW(nullptr, e.what(), nullptr, MB_OK, 0);
    }
    catch (std::runtime_error& e)
    {
        MessageBoxA(nullptr, e.what(), nullptr, MB_OK);
    }

    return 0;
}
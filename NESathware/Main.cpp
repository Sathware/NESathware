#include "NES.h"
#include "../SathwareEngine/SathwareEngine.h"
#include "../SathwareEngine/DesktopWindow.h"
#include "../SathwareEngine/Graphics.h"
#include "../SathwareEngine/Timer.h"
#include <iostream>
#include <iomanip>

int main()
{
    try
    {
        HINSTANCE SathwareAPI dllInstance;

        if (dllInstance == nullptr)
            throw std::runtime_error("Could not get DLL Handle");

        DesktopWindow desktopWindow(256u, 240u, dllInstance, SW_NORMAL);
        Graphics directXGFX(desktopWindow);

        NES nes("DonkeyKong.nes", directXGFX);
        Timer timer;

        /*nes.mPPU.DisplayCHRROM();
        directXGFX.Render();*/

        while (desktopWindow.IsRunning())
        {
            nes.Run(timer.GetElapsedSeconds());
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
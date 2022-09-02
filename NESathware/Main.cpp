#include "NES.h"
#include "../SathwareEngine/SathwareEngine.h"
#include "../SathwareEngine/DesktopWindow.h"
#include "../SathwareEngine/Graphics.h"
#include "../SathwareEngine/Timer.h"

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

        NES nes("helloworld.nes", directXGFX);

        while (desktopWindow.IsRunning())
        {
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
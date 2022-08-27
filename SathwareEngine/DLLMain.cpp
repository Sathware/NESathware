#include "DesktopWindow.h"
#include "SathwareException.h"
#include "SathwareApp.h"
#include <stdexcept>
#include "SathwareEngine.h"
#include "Timer.h"
//#include "Graphics.h"
//#include "Timer.h"

HINSTANCE SathwareAPI dllInstance = nullptr;

//std::unique_ptr<DesktopWindow> SathwareAPI CreateSathwareWindow(unsigned int clientWidth, unsigned int clientHeight, const std::wstring& windowTitle = L"Sathware Engine")
//{
//    return std::make_unique<DesktopWindow>(clientWidth, clientHeight, dllInstance, SW_NORMAL, windowTitle);
//}

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpvReserved)  // reserved
{
    // Perform actions based on the reason for calling.
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        dllInstance = hinstDLL;
        break;

    case DLL_THREAD_ATTACH:
        // Do thread-specific initialization.
        break;

    case DLL_THREAD_DETACH:
        // Do thread-specific cleanup.
        break;

    case DLL_PROCESS_DETACH:

        if (lpvReserved != nullptr)
        {
            break; // do not do cleanup if process termination scenario
        }

        // Perform any necessary cleanup.
        break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
//int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
//{
//	//TODO: REMOVE RESOLUTION FROM BEING TIED TO WINDOW CLASS, GRAPHICS SHOULD QUERY AND GET MONITOR INFO FIRST
//	try
//	{
//		DesktopWindow desktopWindow(1920u, 1080u, hInstance, nCmdShow);
//
//		Graphics directXGFX(desktopWindow);
//
//		while (desktopWindow.IsRunning())
//		{
//			directXGFX.Clear();
//
//			
//			for (int x = 0; x < 500; ++x) 
//				directXGFX.PutPixel(x, 100, Color(255, 255, 255, 255));
//
//			//Timer FrameTimer;
//			directXGFX.Render();
//			//OutputDebugStringW(std::to_wstring(FrameTimer.GetElapsedSeconds()).c_str());
//		}
//	}
//	catch (Exception& e)
//	{
//		MessageBoxExW(nullptr, e.what(), nullptr, MB_OK, 0);
//	}
//	catch (std::runtime_error& e)
//	{
//		MessageBoxA(nullptr, e.what(), nullptr, MB_OK);
//	}
//
//	return 0;
//}
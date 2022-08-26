#include "DesktopWindow.h"
#include "SathwareException.h"
#include "../NES.h"
#include <stdexcept>
//#include "Graphics.h"
//#include "Timer.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	//TODO: REMOVE RESOLUTION FROM BEING TIED TO WINDOW CLASS, GRAPHICS SHOULD QUERY AND GET MONITOR INFO FIRST
	try
	{
		DesktopWindow desktopWindow(hInstance, nCmdShow);

		Graphics directXGFX(desktopWindow);

		NES nes("DonkeyKong.nes", directXGFX);

		while (desktopWindow.Running())
		{
			//directXGFX.Clear();

			//Ready to present frame
			if (nes.Execute())
			{
				directXGFX.Render();
				directXGFX.Clear();
			}

			/*for (int x = 0; x < 500; ++x) 
				directXGFX.PutPixel(x, 100, Color(255, 255, 255, 255));*/

			//Timer FrameTimer;
			//directXGFX.Render();
			//OutputDebugStringW(std::to_wstring(FrameTimer.GetElapsedSeconds()).c_str());
		}
	}
	catch (Exception& e)
	{
		MessageBoxExW(nullptr, e.what(), nullptr, MB_OK, 0);
	}
	catch (std::exception& e)
	{
		MessageBoxExA(nullptr, e.what(), nullptr, MB_OK, 0);
	}

	return 0;
}
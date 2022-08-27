#pragma once
#include <string>
#include "SathwareException.h"
#include "Graphics.h"
#include "SathwareEngine.h"

class SathwareAPI DesktopWindow
{
public:
	friend Graphics::Graphics(const DesktopWindow& window);
	//----User Functions 

	/*
	* params:
	* hInstance - instance/module handle given by wWinMain
	* nCmdShow - given by wWinMain
	* windowTitle - wide string text to be displayed on the titlebar of the window
	*/
	DesktopWindow(unsigned int clientWidth, unsigned int clientHeight, HINSTANCE hInstance, int nCmdShow, const std::wstring& windowTitle = L"Sathware Engine");

	/*
	* return:
	* true - if the window is running (i.e. the user hasn't closed the window yet)
	* false - the user has chosen to close the window
	*/
	bool IsRunning();

	//----Deleted Functions 
	DesktopWindow() = delete;
	DesktopWindow(const DesktopWindow& other) = delete;
	DesktopWindow(const DesktopWindow&& other) = delete;
	DesktopWindow& operator=(const DesktopWindow& other) = delete;
private:
	static LRESULT CALLBACK WindowProc_Setup(HWND windowHandle, UINT message, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK WindowProc_Thunk(HWND windowHandle, UINT message, WPARAM wparam, LPARAM lparam);
	LRESULT EventHandler(UINT message, WPARAM wparam, LPARAM lparam);
private:
	int mClientWidth;
	int mClientHeight;
	HWND m_windowHandle;
	MSG m_windowsMessage;
	std::wstring m_windowTitle;
	static constexpr wchar_t m_className[] = L"Desktop App";//Win32 internal use, see "https://docs.microsoft.com/en-us/windows/win32/learnwin32/creating-a-window"
};

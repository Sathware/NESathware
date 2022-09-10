#include "DesktopWindow.h"

//We set the userdata for the window instance so that DesktopWindow instantiations are responsible for processing their own messages
//"https://docs.microsoft.com/en-us/windows/win32/learnwin32/managing-application-state-"
LRESULT CALLBACK DesktopWindow::WindowProc_Setup(HWND windowHandle, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (message == WM_CREATE)
	{
		CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lparam);
		SetWindowLongPtrW(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreate->lpCreateParams));
		SetWindowLongPtrW(windowHandle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&DesktopWindow::WindowProc_Thunk));
	}
	return DefWindowProcW(windowHandle, message, wparam, lparam);
}

//Dummy function that we can give to windows as the WndProc function pointer
LRESULT CALLBACK DesktopWindow::WindowProc_Thunk(HWND windowHandle, UINT message, WPARAM wparam, LPARAM lparam)
{
	DesktopWindow* windowInstance = reinterpret_cast<DesktopWindow*>(GetWindowLongPtrW(windowHandle, GWLP_USERDATA));
	return windowInstance->EventHandler(message, wparam, lparam);
}

//Returns true while running, and false after user closes window
bool DesktopWindow::IsRunning()
{
	while (PeekMessageW(&m_windowsMessage, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&m_windowsMessage);
		DispatchMessageW(&m_windowsMessage);

		if (m_windowsMessage.message == WM_QUIT)
			return false;
	}
	return true;
}

//Standard WndProc
//"https://docs.microsoft.com/en-us/windows/win32/learnwin32/writing-the-window-procedure"
LRESULT DesktopWindow::EventHandler(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:
		DestroyWindow(m_windowHandle);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProcW(m_windowHandle, message, wParam, lParam);
	}
}

//Standard construction of window
//"https://docs.microsoft.com/en-us/windows/win32/learnwin32/creating-a-window"
//"https://docs.microsoft.com/en-us/windows/win32/learnwin32/managing-application-state-" - 'Object-Oriented' section
DesktopWindow::DesktopWindow(unsigned int clientWidth, unsigned int clientHeight, HINSTANCE hInstance, int nCmdShow, const std::wstring& windowTitle)
	: mClientWidth(clientWidth), mClientHeight(clientHeight), m_windowTitle(windowTitle), m_windowsMessage()
{
	WNDCLASSEX window{
		sizeof(WNDCLASSEX),
		0,
		&DesktopWindow::WindowProc_Setup,
		0,
		0,
		hInstance,
		LoadIconW(hInstance, IDI_APPLICATION),
		LoadCursorW(nullptr, IDC_ARROW),
		nullptr/*reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1)*/,
		nullptr,
		m_className,
		LoadIcon(hInstance, IDI_APPLICATION)
	};

	if (!RegisterClassExW(&window))
	{
		throw Exception(L"Call to RegisterClassExW failed!");
	}

	RECT clientArea{0, 0, mClientWidth, mClientHeight};
	//Calculate the window width and height, given the client area width and height
	AdjustWindowRect(&clientArea, WS_OVERLAPPEDWINDOW & ~WS_OVERLAPPED, false);
		
	m_windowHandle = CreateWindowExW(
		0,
		m_className,
		windowTitle.c_str(),
		WS_OVERLAPPEDWINDOW & ~WS_OVERLAPPED,
		30,
		30,
		clientArea.right - clientArea.left,
		clientArea.bottom - clientArea.top,
		nullptr,
		nullptr,
		hInstance,
		reinterpret_cast<LPVOID>(this));

	if (m_windowHandle == nullptr)
	{
		throw Exception(L"Call to CreateWindowExW failed!");
	}

	ShowWindow(m_windowHandle, SW_SHOWDEFAULT);
	//UpdateWindow(m_windowHandle);
}
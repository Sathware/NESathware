#pragma once
#include "DesktopWindow.h"
#include "Graphics.h"
#include "SathwareEngine.h"

class SathwareAPI SathwareApp
{
public:
	SathwareApp(unsigned int clientWidth, unsigned int clientHeight, HINSTANCE hInstance, const std::wstring& windowTitle = L"Sathware Engine")
		: mWindow(clientWidth, clientHeight, hInstance, SW_NORMAL, windowTitle), mGFX(mWindow)
	{}

	DesktopWindow mWindow;
	Graphics mGFX;
};


#include "Controller.h"
#include "BUS.h"
#include "..\SathwareEngine\DesktopWindow.h"
#include <iostream>

void Controller::Execute()
{
	if (mPollFlag)
	{
		mInputState = 0;
		mCurrButtonIndex = 0;
		for (unsigned int mButtonIndex = 0; mButtonIndex < 8; ++mButtonIndex)
		{
			//if button is pressed
			if (Window.KeyIsPressed(mButtons[mButtonIndex]))
				mInputState |= (1 << mButtonIndex);
		}
	}
}

void Controller::WriteCPU(bool setStrobe)
{
	mPollFlag = setStrobe;
}

ubyte Controller::ReadCPU()
{
	if (mPollFlag)
	{
		return mInputState & 0x01u;
	}
	else if (mCurrButtonIndex < 8)
	{
		return (mInputState >> (mCurrButtonIndex++)) & 0x01u;
	}
	else
		return 1u;
}

void Zapper::Execute()
{
	if (mPollFlag)
	{
		mInputState = 0b00001000;

		// Calculate if light is sensed
		vec2f mousepos = Window.GetMousePosition();

		unsigned int y = static_cast<unsigned int>(static_cast<float>(Gfx.m_height) * mousepos.y);
		unsigned int x = static_cast<unsigned int>(static_cast<float>(Gfx.m_width) * mousepos.x);

		y = std::clamp(y, 0u, static_cast<unsigned int>(Gfx.m_height - 1));
		x = std::clamp(x, 0u, static_cast<unsigned int>(Gfx.m_width - 1));

		Color pixelValue = Gfx.GetPixel(x, y);

		if ((pixelValue.rgba - mPrevPixelValue) > 100u)
		{
			mInputState &= (0u << 3u);
		}

		// Calculate if trigger pull state
		bool currLDown = static_cast<int>(GetAsyncKeyState(MK_LBUTTON)) < 0;
		bool currRDown = static_cast<int>(GetAsyncKeyState(MK_RBUTTON)) < 0;

		// On Mouse Down
		/*if (currLDown && !mPrevLDown)
			int x = 5;*/
		// On Mouse Held Press
		if (currLDown && mPrevLDown)
			mInputState |= (1u << 4u);
		// On Mouse Held Press
		if (currRDown && mPrevRDown)
			mInputState |= (1u << 4u);
		// On Mouse Release
		//if (!currLDown && mPrevLDown)
		//	int x = 5;
		//	//mInputState |= (1u << 4u);

		mPrevLDown = currLDown;
		mPrevRDown = currRDown;
		mPrevPixelValue = pixelValue.rgba;

		//std::cout << "Val: " << pixelValue.rgba << " L: " << currLDown << std::endl;
	}
}

void Zapper::WriteCPU(bool setStrobe)
{
	mPollFlag = setStrobe;
}

ubyte Zapper::ReadCPU()
{
	/*if (mPollFlag)
	{
		return mInputState & 0x01u;
	}
	else if (mCurrButtonIndex < 8)
	{
		return (mInputState >> (mCurrButtonIndex++)) & 0x01u;
	}
	else
		return 1u;*/
	return mInputState;
}
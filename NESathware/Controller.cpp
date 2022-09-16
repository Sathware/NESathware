#include "Controller.h"
#include "BUS.h"

void Controller::Execute()
{
	if (mPollFlag)
	{
		mInputState = 0;
		mCurrButtonIndex = 0;
		for (unsigned int mButtonIndex = 0; mButtonIndex < 8; ++mButtonIndex)
		{
			//if button is pressed
			if (GetKeyState(mButtons[mButtonIndex]) & 0x8000)
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
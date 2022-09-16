#pragma once
#include "CommonTypes.h"

class Controller
{
public:
	Controller(class BUS& bus, class DesktopWindow& window)
		: Bus(bus), Window(window)
	{}
	void Execute();
	//CPU tells controller to start or stop polling
	void WriteCPU(bool setStrobe);
	//CPU reads input state from controller
	ubyte ReadCPU();
private:
	BUS& Bus;
	DesktopWindow& Window;

	//Dictates whether the controller is currently polling input or not
	bool mPollFlag = false;
	//Current button to read
	unsigned int mCurrButtonIndex = 0;
	//bits: 0 = A, 1 = B, 2 = Select, 3 = Start, 4 = Up, 5 = Down, 6 = Left, 7 = Right
	unsigned int mInputState = 0;
	//Keyboard keys: Space B E T W S A D
	int mButtons[8] = {0x20, 'B', 'E', 'T', 'W', 'S', 'A', 'D'};
};


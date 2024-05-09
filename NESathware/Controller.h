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

class Zapper
{
public:
	Zapper(BUS& bus, DesktopWindow& window, class Graphics& gfx)
		: Bus(bus), Window(window), Gfx(gfx)
	{}
	void Execute();
	//CPU reads input state from controller
	ubyte ReadCPU();
	//CPU tells controller to start or stop polling
	void WriteCPU(bool setStrobe);
private:
	BUS& Bus;
	DesktopWindow& Window;
	Graphics& Gfx;

	//Dictates whether the controller is currently polling input or not
	bool mPollFlag = false;
	//bits: 0 = Serial for (vs.), 3 = Light Sensed (0 if light detected; 1 if not), 4 = Trigger pull (0 if fully released or pulled; 1 if half pulled)
	unsigned int mInputState = 0;
	unsigned int mPrevPixelValue = 0;
	bool mPrevLDown = false;
	bool mPrevRDown = false;
};

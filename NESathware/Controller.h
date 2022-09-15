#pragma once
#include "CommonTypes.h"

class Controller
{
public:
	Controller(class BUS& Bus)
		: Bus(Bus)
	{}
private:
	BUS& Bus;
};


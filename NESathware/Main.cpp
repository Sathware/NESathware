#include "NES.h"

int main()
{
	NES nes("nestest.nes", 0xc000);

	while (true)
	{
		nes.Execute();
	}
}
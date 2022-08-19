#include "NES.h"

int main()
{
	NES nes("DonkeyKong.nes", 0xc000);

	while (true)
	{
		nes.Execute();
	}
}
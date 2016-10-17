#include "Console.h"
#include <iostream>
#include <string>

namespace Emu8
{
	void Console::Print(std::string text)
	{
		std::cout << text << std::endl;
	}
}

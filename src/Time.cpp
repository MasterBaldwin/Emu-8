#include "Time.h"
#include <SDL.h>

namespace Emu8
{
	Time::Time()
			: nextUpdateTicks(SDL_GetTicks()), fpsLimit(1000), fpsCount(), fpsTicks(SDL_GetTicks()), fpsPerSecond()
	{
	}

	bool Time::canUpdate()
	{
		int ticks = SDL_GetTicks();

		if(ticks >= nextUpdateTicks)
		{
			nextUpdateTicks += fpsLimit == 0 ? ticks - nextUpdateTicks : (1000 / fpsLimit) == 0 ? 1 : 1000 / fpsLimit;

			fpsCount++;

			if(ticks - fpsTicks >= 1000)
			{
				fpsTicks = ticks;
				fpsPerSecond = fpsCount;
				fpsCount = 0;
			}

			return true;
		}

		return false;
	}

	int Time::ticksTillUpdate()
	{
		return nextUpdateTicks - SDL_GetTicks();
	}

	int Time::getFPS()
	{
		return fpsPerSecond;
	}
}

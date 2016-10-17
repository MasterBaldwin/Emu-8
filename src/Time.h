#ifndef EMU_8_TIME_H
#define EMU_8_TIME_H

namespace Emu8
{
	class Time
	{
	private:
		int nextUpdateTicks;
		int fpsLimit;
		int fpsCount;
		int fpsTicks;
		int fpsPerSecond;

	public:
		Time();
		bool canUpdate();
		int ticksTillUpdate();
		int getFPS();
	};
}

#endif //EMU_8_TIME_H

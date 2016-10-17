#ifndef EMU_8_DISPLAY_H
#define EMU_8_DISPLAY_H

#include <SDL.h>

namespace Emu8
{
	class Display
	{
	private:
		static SDL_Window* window;
		static SDL_Surface* windowSurface;
		static SDL_Renderer* windowRenderer;
		static int screenWidth;
		static int screenHeight;

	public:
		static int GetScreenWidth();
		static int GetScreenHeight();
		static SDL_Window* GetWindow();
		static SDL_Surface* GetWindowSurface();
		static SDL_Renderer* GetWindowRenderer();
		static bool Create(const int screenWidth, const int screenHeight);
		static void Destroy();
		static void Flip();
	};
}

#endif //EMU_8_DISPLAY_H

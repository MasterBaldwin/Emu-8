#include "Console.h"
#include "Display.h"
#include <SDL.h>

namespace Emu8
{
	SDL_Window* Display::window;
	SDL_Surface* Display::windowSurface;
	SDL_Renderer* Display::windowRenderer;
	int Display::screenWidth;
	int Display::screenHeight;

	int Display::GetScreenWidth()
	{
		return screenWidth;
	}

	int Display::GetScreenHeight()
	{
		return screenHeight;
	}

	SDL_Window* Display::GetWindow()
	{
		return window;
	}

	SDL_Surface* Display::GetWindowSurface()
	{
		return windowSurface;
	}

	SDL_Renderer* Display::GetWindowRenderer()
	{
		return windowRenderer;
	}

	bool Display::Create(const int screenWidth, const int screenHeight)
	{
		Destroy();

		window = SDL_CreateWindow("Emu-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);

		if(window == nullptr)
		{
			Console::Print("Failed to create an SDL2 window!");
			return false;
		}

		windowSurface = SDL_GetWindowSurface(window);
		windowRenderer = SDL_GetRenderer(window);

		return true;
	}

	void Display::Destroy()
	{
		if(window != nullptr)
		{
			SDL_DestroyWindow(window);
			window = nullptr;
		}
	}

	void Display::Flip()
	{
		SDL_UpdateWindowSurface(window);
	}
}

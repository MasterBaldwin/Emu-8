#ifndef EMU_8_CHIP8_H
#define EMU_8_CHIP8_H

#include <array>
#include <SDL.h>
#include <SDL_ttf.h>
#include <stack>
#include <string>
#include "Time.h"

namespace Emu8
{
	class Chip8
	{
	private:
		bool isRunning;
		std::array<unsigned char, 4096> mainMem;
		std::stack<unsigned short> stackMem;
		std::array<unsigned char, 16> vReg;
		std::array<unsigned char, 128 * 64> displayArray;
		std::array<bool, 16> keyInputs;
		unsigned short iRegister;
		unsigned char delayRegister;
		unsigned char soundRegister;
		unsigned short programCounter;
		SDL_Event inputEvent;
		SDL_Surface* screenSurface;
		bool stopProcessing;
		unsigned char regX;
		Time time;
		TTF_Font* fpsFont;

		void loadFontData();
		void runInstruction(unsigned char upper, unsigned char lower);
		void setPixel(SDL_Surface* surface, unsigned int x, unsigned int y, Uint32 color);
		Uint32 getPixel(SDL_Surface* surface, unsigned int x, unsigned int y);
		void WriteDisplayArrayToSurface();
		void ProcessKeyInput();
		bool GetKeyInput(unsigned char value);
		unsigned char ConvertToHexKeyboard(SDL_Keycode code);

	public:
		Chip8();
		~Chip8();
		bool init();
		void release();
		void loadGame(std::string filePath);
		void start();
	};
}

#endif //EMU_8_CHIP8_H

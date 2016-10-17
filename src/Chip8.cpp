#include "Chip8.h"
#include <array>
#include <fstream>
#include <iostream>
#include <random>
#include <SDL_ttf.h>
#include <stack>
#include <string>
#include "Console.h"
#include "Display.h"
#include "Time.h"
#include "File.h"

const unsigned int SCREEN_WIDTH = 64;
const unsigned int SCREEN_HEIGHT = 32;

namespace Emu8
{
	Chip8::Chip8()
			: isRunning(true), mainMem(), stackMem(), vReg(), displayArray(), keyInputs(), iRegister(), delayRegister(), soundRegister(), programCounter(512), inputEvent(), screenSurface(nullptr), stopProcessing(false), regX(), time(), fpsFont(nullptr)
	{
	}

	Chip8::~Chip8()
	{
		Console::Print("Shutting down emulator.");

		release();

		Console::Print("Goodbye...");
	}

	bool Chip8::init()
	{
		Console::Print("Initializing SDL2...");
		if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
		{
			Console::Print("SDL2 failed to init!");
			return false;
		}
		if(TTF_Init() < 0)
		{
			Console::Print("SDL2_ttf failed to init!");
			return false;
		}

		Console::Print("Creating Display...");
		if(!Display::Create(1280, 720))
		{
			Console::Print("Failed to create display!");
			return false;
		}

		if((fpsFont = TTF_OpenFont("Data/Fonts/arial.ttf", 16)) == nullptr)
		{
			Console::Print("Failed to load font!");
			return false;
		}

		SDL_Surface* tempSurface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
		SDL_Surface* optimizedSurface = SDL_ConvertSurface(tempSurface, Display::GetWindowSurface()->format, 0);
		screenSurface = optimizedSurface;
		SDL_FreeSurface(tempSurface);

		loadFontData();

		return true;
	}

	void Chip8::release()
	{
		Display::Destroy();
		TTF_CloseFont(fpsFont);
		TTF_Quit();
		SDL_FreeSurface(screenSurface);
		SDL_Quit();
	}

	void Chip8::loadGame(std::string filePath)
	{
		File file = File(filePath);
		file.open();
		file.readAll((char*)(&mainMem[512]));
		file.close();

		/*std::ifstream file;
		file.open(filePath, std::ios::out | std::ios::binary | std::ios::ate);

		if(file.is_open())
		{
			int fileSize = (int)file.tellg();
			file.seekg(0, std::ios::beg);
			file.read((char*)(&mainMem[512]), fileSize);
			file.close();
		}//*/
	}

	void Chip8::start()
	{
		while(isRunning)
		{
			while(time.canUpdate())
			{
				while(SDL_PollEvent(&inputEvent))
				{
					if(inputEvent.type == SDL_QUIT)
					{
						isRunning = false;
					}

					if(inputEvent.type == SDL_KEYDOWN)
					{
						if(stopProcessing)
						{
							vReg[regX] = ConvertToHexKeyboard(inputEvent.key.keysym.sym);
							stopProcessing = false;
						}
					}

					ProcessKeyInput();
				}

				//Logic
				if(!stopProcessing)
				{
					runInstruction(mainMem[programCounter], mainMem[programCounter + 1]);

					if(delayRegister > 0)
					{
						delayRegister--;
					}
					if(soundRegister > 0)
					{
						soundRegister--;
						Console::Print("Beep!");
					}
				}

				//Render
				WriteDisplayArrayToSurface();
				SDL_BlitScaled(screenSurface, nullptr, Display::GetWindowSurface(), nullptr);
				SDL_Color fpsColor = {255, 0, 255, 255};
				std::string fpsText = "FPS: " + std::to_string(time.getFPS());
				SDL_Surface* fpsSurface = TTF_RenderText_Solid(fpsFont, fpsText.c_str(), fpsColor);
				SDL_BlitSurface(fpsSurface, nullptr, Display::GetWindowSurface(), nullptr);
				SDL_FreeSurface(fpsSurface);
				Display::Flip();
			}
			SDL_Delay(time.ticksTillUpdate());
		}
	}

	void Chip8::loadFontData()
	{
		std::array<unsigned char, 80> fontData =
				{
						0xF0, 0x90, 0x90, 0x90, 0xF0, //0
						0x20, 0x60, 0x20, 0x20, 0x70, //1
						0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
						0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
						0x90, 0x90, 0xF0, 0x10, 0x10, //4
						0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
						0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
						0xF0, 0x10, 0x20, 0x40, 0x40, //7
						0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
						0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
						0xF0, 0x90, 0xF0, 0x90, 0x90, //A
						0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
						0xF0, 0x80, 0x80, 0x80, 0xF0, //C
						0xE0, 0x90, 0x90, 0x90, 0xE0, //D
						0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
						0xF0, 0x80, 0xF0, 0x80, 0x80 //F
				};

		std::copy(std::begin(fontData), std::end(fontData), std::begin(mainMem));

		/*for(unsigned int i = 0; i < fontData.size(); i++)
		{
			mainMem[i] = fontData[i];
		}//*/
	}

	void Chip8::runInstruction(unsigned char upper, unsigned char lower)
	{
		unsigned short fullInstruction = (upper << 8) | lower; //Full two byte instruction pulled from memory.
		unsigned char header = (unsigned char)(upper & 0xF0) >> 4; //The upper 4 bits of the instruction.
		unsigned short address = (unsigned short)(fullInstruction & 0x0FFF); //The lowest 12 bits of the instruction.
		unsigned char nibble = (unsigned char)(lower & 0x0F); //The lowest 4 bits of the instruction.
		unsigned char xReg = (unsigned char)(upper & 0x0F); //The lower 4 bits of the high byte of the instruction.
		unsigned char yReg = (unsigned char)(lower & 0xF0) >> 4; //The upper 4 bits of the low byte of the instruction.
		unsigned char kk = lower; //The lowest 8 bits of the instruction.

		switch(header)
		{
			case 0x00:
			{
				switch(fullInstruction)
				{
					case 0x00E0: //Clear the display
					{
						displayArray.fill(0);
						programCounter += 2;
						break;
					}
					case 0x00EE: //Return from subroutine
					{
						programCounter = stackMem.top();
						stackMem.pop();
						break;
					}
					default: //Jump to system address (ignored)
					{
						//Do nothing, this instruction is ignored.
						programCounter += 2;
						break;
					}
				}
				break;
			}
			case 0x01: //Jump to address
			{
				programCounter = address;
				break;
			}
			case 0x02: //Call subroutine at address
			{
				programCounter += 2;
				stackMem.push(programCounter);
				programCounter = address;
				break;
			}
			case 0x03: //Skip the next instruction if Vx == kk
			{
				if(vReg[xReg] == kk)
				{
					programCounter += 2;
				}
				programCounter += 2;
				break;
			}
			case 0x04: //Skip the next instruction if Vx != kk
			{
				if(vReg[xReg] != kk)
				{
					programCounter += 2;
				}
				programCounter += 2;
				break;
			}
			case 0x05: //Skip the next instruction if Vx != Vy
			{
				if(vReg[xReg] == vReg[yReg])
				{
					programCounter += 2;
				}
				programCounter += 2;
				break;
			}
			case 0x06: //Set Vx = kk
			{
				vReg[xReg] = kk;
				programCounter += 2;
				break;
			}
			case 0x07: // Set Vx = Vx + kk
			{
				vReg[xReg] = kk + vReg[xReg];
				programCounter += 2;
				break;
			}
			case 0x08:
			{
				switch(nibble)
				{
					case 0x00: //Set Vx = Vy
					{
						vReg[xReg] = vReg[yReg];
						break;
					}
					case 0x01: //Set Vx = Vx OR Vy
					{
						vReg[xReg] = vReg[xReg] | vReg[yReg];
						break;
					}
					case 0x02: //Set Vx = Vx AND Vy
					{
						vReg[xReg] = vReg[xReg] & vReg[yReg];
						break;
					}
					case 0x03: //Set Vx = Vx XOR Vy
					{
						vReg[xReg] = vReg[xReg] ^ vReg[yReg];
						break;
					}
					case 0x04: //Set Vx = Vx + Vy, set VF = carry
					{
						vReg[15] = vReg[xReg] > (255 - vReg[yReg]) ? (unsigned char)1 : (unsigned char)0;
						vReg[xReg] = vReg[xReg] + vReg[yReg];
						break;
					}
					case 0x05: //Set Vx = Vx - Vy, set VF = NOT borrow
					{
						vReg[15] = vReg[xReg] > vReg[yReg] ? (unsigned char)1 : (unsigned char)0;
						vReg[xReg] = vReg[xReg] - vReg[yReg];
						break;
					}
					case 0x06: //Set Vx = Vx SHR 1
					{
						vReg[15] = vReg[xReg] & 0x1 ? (unsigned char)1 : (unsigned char)0;
						vReg[xReg] = vReg[xReg] / (unsigned char)2;
						break;
					}
					case 0x07: //Set Vx = Vy - Vx, set VF = NOT borrow
					{
						vReg[15] = vReg[yReg] > vReg[xReg] ? (unsigned char)1 : (unsigned char)0;
						vReg[xReg] = vReg[yReg] - vReg[xReg];
						break;
					}
					case 0x0E: //Set Vx = Vx SHL 1
					{
						vReg[15] = vReg[xReg] & 0x8 ? (unsigned char)1 : (unsigned char)0;
						vReg[xReg] = vReg[xReg] * (unsigned char)2;
						break;
					}
					default:
					{
						Console::Print("Unknown instruction has been read with a header of 8!");
						break;
					}
				}
				programCounter += 2;
				break;
			}
			case 0x09: //Skip next instruction if Vx != Vy
			{
				if(vReg[xReg] != vReg[yReg])
				{
					programCounter += 2;
				}
				programCounter += 2;
				break;
			}
			case 0x0A: //Set I = nnn
			{
				iRegister = address;
				programCounter += 2;
				break;
			}
			case 0x0B: //Jump to location nnn + V0
			{
				programCounter = address + vReg[0];
				break;
			}
			case 0x0C: //Set Vx = random byte AND kk
			{
				std::minstd_rand rand;
				unsigned char randomNumber = (unsigned char)(rand() % (255 - 0 + 1) + 0);
				vReg[xReg] = randomNumber & kk;
				programCounter += 2;
				break;
			}
			case 0x0D: //Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
			{
				//Set the flag register to zero, for if there is a collision it will be set to one.
				vReg[15] = 0;

				for(unsigned int iY = 0, memLocation = iRegister; iY < nibble; iY++, memLocation++)
				{
					unsigned char dataByte = mainMem[memLocation];
					std::array<unsigned char, 8> drawBits = std::array<unsigned char, 8>();

					for(unsigned int iX = 0; iX < 8; iX++)
					{
						int drawX = vReg[xReg] + iX;
						int drawY = vReg[yReg] + iY;

						if(iX < 0)
						{
							drawX += SCREEN_WIDTH;
						}
						else if(iX >= SCREEN_WIDTH)
						{
							drawX -= SCREEN_WIDTH;
						}
						if(iY < 0)
						{
							drawY += SCREEN_HEIGHT;
						}
						else if(iY >= SCREEN_HEIGHT)
						{
							drawY -= SCREEN_HEIGHT;
						}

						unsigned char pixel = (dataByte & ((unsigned char)0x80 >> iX)) >> (7 - iX);
						unsigned char currentDisplayBit = displayArray[(drawY * SCREEN_WIDTH) + drawX];

						displayArray[(drawY * SCREEN_WIDTH) + drawX] = currentDisplayBit ^ pixel;

						if(currentDisplayBit == 1 && displayArray[(drawY * SCREEN_WIDTH) + drawX] == 0)
						{
							vReg[15] = 1;
						}
					}
				}
				programCounter += 2;
				break;
			}
			case 0x0E:
			{
				switch(kk)
				{
					case 0x9E: //Skip next instruction if key with the value of Vx is pressed
					{
						if(GetKeyInput(vReg[xReg]))
						{
							programCounter += 2;
						}
						break;
					}
					case 0xA1: //Skip next instruction if key with the value of Vx is not pressed
					{
						if(!GetKeyInput(vReg[xReg]))
						{
							programCounter += 2;
						}
						break;
					}
					default:
					{
						Console::Print("Unknown instruction has been read with a header of E!");
						break;
					}
				}
				programCounter += 2;
				break;
			}
			case 0x0F:
			{
				switch(kk)
				{
					case 0x07: //Set Vx = delay timer value
					{
						vReg[xReg] = delayRegister;
						break;
					}
					case 0x0A: //Wait for a key press, store the value of the key in Vx
					{
						stopProcessing = true;
						regX = xReg;
						break;
					}
					case 0x15: //Set delay timer = Vx
					{
						delayRegister = vReg[xReg];
						break;
					}
					case 0x18: //Set sound timer = Vx
					{
						soundRegister = vReg[xReg];
						break;
					}
					case 0x1E: //Set I = I + Vx
					{
						iRegister = iRegister + vReg[xReg];
						break;
					}
					case 0x29: //Set I = location of sprite for digit Vx
					{
						iRegister = vReg[xReg] * (unsigned short)5;
						break;
					}
					case 0x33: //Store BCD representation of Vx in memory locations I, I+1, and I+2
					{
						mainMem[iRegister] = (unsigned char)(vReg[xReg] / 100);
						mainMem[iRegister + 1] = (unsigned char)((vReg[xReg] / 10) % 10);
						mainMem[iRegister + 2] = (unsigned char)((vReg[xReg] % 100) % 10);
						break;
					}
					case 0x55: //Store registers V0 through Vx in memory starting at location I
					{
						for(unsigned char i = 0; i <= xReg; i++)
						{
							mainMem[iRegister + i] = vReg[i];
						}
						break;
					}
					case 0x65: //Read registers V0 through Vx from memory starting at location I
					{
						for(unsigned char i = 0; i <= xReg; i++)
						{
							vReg[i] = mainMem[iRegister + i];
						}
						break;
					}
					default:
					{
						Console::Print("Unknown instruction has been read with a header of F!");
						break;
					}
				}
				programCounter += 2;
				break;
			}
			default:
			{
				Console::Print("Unknown instruction has been read, IGNORE ME!!!");
				programCounter += 2;
				break;
			}
		}
	}

	void Chip8::setPixel(SDL_Surface* surface, unsigned int x, unsigned int y, Uint32 color)
	{
		int bpp = surface->format->BytesPerPixel;
		/* Here p is the address to the pixel we want to set */
		Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;

		switch(bpp)
		{
			case 1:
				*p = (Uint8)color;
				break;
			case 2:
				*(Uint16*)p = (Uint16)color;
				break;
			case 3:
				if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
				{
					p[0] = (Uint8)((color >> 16) & 0xff);
					p[1] = (Uint8)((color >> 8) & 0xff);
					p[2] = (Uint8)(color & 0xff);
				}
				else
				{
					p[0] = (Uint8)(color & 0xff);
					p[1] = (Uint8)((color >> 8) & 0xff);
					p[2] = (Uint8)((color >> 16) & 0xff);
				}
				break;
			case 4:
				*(Uint32*)p = color;
				break;
			default:
				*(Uint32*)p = color;
				break;
		}
	}

	Uint32 Chip8::getPixel(SDL_Surface* surface, unsigned int x, unsigned int y)
	{
		int bpp = surface->format->BytesPerPixel;
		/* Here p is the address to the pixel we want to retrieve */
		Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;

		switch(bpp)
		{
			case 1:
				return *p;
			case 2:
				return *(Uint16*)p;
			case 3:
				if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
				{
					return p[0] << 16 | p[1] << 8 | p[2];
				}
				else
				{
					return p[0] | p[1] << 8 | p[2] << 16;
				}
			case 4:
				return *(Uint32*)p;
			default:
				return 0;       /* shouldn't happen, but avoids warnings */
		}
	}

	void Chip8::WriteDisplayArrayToSurface()
	{
		if(SDL_MUSTLOCK(screenSurface))
		{
			SDL_LockSurface(screenSurface);
		}

		//screenSurface->pixels

		for(unsigned int y = 0; y < SCREEN_HEIGHT; y++)
		{
			for(unsigned int x = 0; x < SCREEN_WIDTH; x++)
			{
				setPixel(screenSurface, x, y, displayArray[(y * SCREEN_WIDTH) + x] == 1 ? SDL_MapRGB(screenSurface->format, 255, 255, 255) : SDL_MapRGB(screenSurface->format, 0, 0, 0));
			}
		}

		if(SDL_MUSTLOCK(screenSurface))
		{
			SDL_UnlockSurface(screenSurface);
		}
	}

	void Chip8::ProcessKeyInput()
	{
		const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);

		keyInputs[0] = currentKeyStates[SDL_SCANCODE_1];
		keyInputs[1] = currentKeyStates[SDL_SCANCODE_2];
		keyInputs[2] = currentKeyStates[SDL_SCANCODE_3];
		keyInputs[3] = currentKeyStates[SDL_SCANCODE_4];
		keyInputs[4] = currentKeyStates[SDL_SCANCODE_Q];
		keyInputs[5] = currentKeyStates[SDL_SCANCODE_W];
		keyInputs[6] = currentKeyStates[SDL_SCANCODE_E];
		keyInputs[7] = currentKeyStates[SDL_SCANCODE_R];
		keyInputs[8] = currentKeyStates[SDL_SCANCODE_A];
		keyInputs[9] = currentKeyStates[SDL_SCANCODE_S];
		keyInputs[10] = currentKeyStates[SDL_SCANCODE_D];
		keyInputs[11] = currentKeyStates[SDL_SCANCODE_F];
		keyInputs[12] = currentKeyStates[SDL_SCANCODE_Z];
		keyInputs[13] = currentKeyStates[SDL_SCANCODE_X];
		keyInputs[14] = currentKeyStates[SDL_SCANCODE_C];
		keyInputs[15] = currentKeyStates[SDL_SCANCODE_V];
	}

	bool Chip8::GetKeyInput(unsigned char value)
	{
		switch(value)
		{
			case 0x1:
				return keyInputs[0];
			case 0x2:
				return keyInputs[1];
			case 0x3:
				return keyInputs[2];
			case 0xC:
				return keyInputs[3];
			case 0x4:
				return keyInputs[4];
			case 0x5:
				return keyInputs[5];
			case 0x6:
				return keyInputs[6];
			case 0xD:
				return keyInputs[7];
			case 0x7:
				return keyInputs[8];
			case 0x8:
				return keyInputs[9];
			case 0x9:
				return keyInputs[10];
			case 0xE:
				return keyInputs[11];
			case 0xA:
				return keyInputs[12];
			case 0x0:
				return keyInputs[13];
			case 0xB:
				return keyInputs[14];
			case 0xF:
				return keyInputs[15];
			default:
				return false;
		}
	}

	unsigned char Chip8::ConvertToHexKeyboard(SDL_Keycode code)
	{
		switch(code)
		{
			case SDLK_1:
				return 0x1;
			case SDLK_2:
				return 0x2;
			case SDLK_3:
				return 0x3;
			case SDLK_4:
				return 0xC;
			case SDLK_q:
				return 0x4;
			case SDLK_w:
				return 0x5;
			case SDLK_e:
				return 0x6;
			case SDLK_r:
				return 0xD;
			case SDLK_a:
				return 0x7;
			case SDLK_s:
				return 0x8;
			case SDLK_d:
				return 0x9;
			case SDLK_f:
				return 0xE;
			case SDLK_z:
				return 0xA;
			case SDLK_x:
				return 0x0;
			case SDLK_c:
				return 0xB;
			case SDLK_v:
				return 0xF;
			default:
				return 0;
		}
	}
}

int main(int argc, char* args[])
{
	Emu8::Chip8* chip8 = new Emu8::Chip8();
	if(!chip8->init())
	{
		Emu8::Console::Print("Chip8 failed to initialize!");
	}
	chip8->loadGame("Chip-8 Game pack/Invaders");
	chip8->start();
	delete chip8;
	return 0;
}

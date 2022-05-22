#pragma once
#include "CPU.hpp"

class SDL_Window;
class SDL_Renderer;
class SDL_Texture;

class DebugWindow {
public:
	DebugWindow();
	~DebugWindow();
	void CreateWindow(int width, int height);
	void PrintInfo(Chip8* chip8);
private:
	SDL_Window* dbgWin;
	SDL_Renderer* dbgRend;
	SDL_Texture* dbgTex;
};
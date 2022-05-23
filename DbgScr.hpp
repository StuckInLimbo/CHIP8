#pragma once
#include "CPU.hpp"
#include <SDL.h>

class DebugWindow {
public:
	DebugWindow();
	~DebugWindow();
	void CreateDebugWindow(int width, int height);
	void PrintInfo(Chip8* chip8);
private:
	SDL_Window* dbgWin;
	SDL_Renderer* dbgRend;
	SDL_Texture* dbgTex;
	SDL_GLContext dbgCtx;
};
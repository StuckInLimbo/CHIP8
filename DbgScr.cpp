#include <SDL.h>
//#include <SDL_ttf.h>
#include "DbgScr.hpp"

//TTF_Font* gFont = NULL;

DebugWindow::DebugWindow() {
	dbgWin = NULL;
	dbgRend = NULL;
	dbgTex = NULL;
}

DebugWindow::~DebugWindow() {
	SDL_DestroyTexture(dbgTex);
	SDL_DestroyRenderer(dbgRend);
	SDL_DestroyWindow(dbgWin);
	//TTF_Quit();
	//SDL_Quit();
}

void DebugWindow::CreateWindow(int width, int height) {
	SDL_Init(SDL_INIT_VIDEO);
	//TTF_Init();
	//gFont = TTF_OpenFont("C:\\Windows\\Fonts\\tahoma.ttf", 30);
	dbgWin = SDL_CreateWindow("CHIP8 Debug", 800, 30, width, height, SDL_WINDOW_SHOWN);
	dbgRend = SDL_CreateRenderer(dbgWin, -1, SDL_RENDERER_ACCELERATED);
	dbgTex = SDL_CreateTexture(dbgRend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	SDL_SetRenderDrawColor(dbgRend, 0, 0, 0, 255);
	SDL_RenderClear(dbgRend);
	SDL_RenderPresent(dbgRend);
}

void DebugWindow::PrintInfo(Chip8* chip8) {
	return;
}

//void DebugWindow::PrintInfo(Chip8* c8) {
//	//Print Registers
//	SDL_Surface* surface = new SDL_Surface();
//	SDL_Color textColor = { 255, 255, 255, 0 };
//	for (int i = 0; i < sizeof(c8->V); ++i) {
//		//surface = TTF_RenderText_Solid(gFont, (const char*)c8->V[i], textColor);
//		surface = TTF_RenderText_Solid(gFont, "Test", textColor);
//		dbgTex = SDL_CreateTextureFromSurface(dbgRend, surface);
//		
//		SDL_Rect* rect = new SDL_Rect();
//		rect->x = 0;
//		rect->y = i * 12;
//		rect->w = surface->w;
//		rect->h = surface->h;
//
//		SDL_RenderCopy(dbgRend, dbgTex, NULL, rect);
//
//		SDL_FreeSurface(surface);
//	}
//}
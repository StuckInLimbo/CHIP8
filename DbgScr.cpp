#include <SDL.h>
//#include <SDL_ttf.h>
#include "DbgScr.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_sdl.h"

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

	//ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	//TTF_Quit();
	//SDL_Quit();
}

void DebugWindow::CreateDebugWindow(int width, int height) {
	SDL_Init(SDL_INIT_VIDEO);
	//TTF_Init();
	//gFont = TTF_OpenFont("C:\\Windows\\Fonts\\tahoma.ttf", 30);
	dbgWin = SDL_CreateWindow("CHIP8 Debug", 1200, 30, width, height, SDL_WINDOW_SHOWN);
	dbgRend = SDL_CreateRenderer(dbgWin, -1, SDL_RENDERER_ACCELERATED);
	dbgTex = SDL_CreateTexture(dbgRend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	SDL_SetRenderDrawColor(dbgRend, 0, 0, 0, 255);
	SDL_RenderClear(dbgRend);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	//ImGui_ImplSDL2_InitForOpenGL(dbgWin, dbgCtx);
	//ImGui_ImplOpenGL3_Init();
	ImGui_ImplSDL2_InitForD3D(dbgWin);
	ImGui_ImplDX9_Init(SDL_RenderGetD3D9Device(dbgRend));
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplSDL2_NewFrame(dbgWin);
	ImGui::NewFrame();
	SDL_RenderPresent(dbgRend);
}

void DebugWindow::PrintInfo(Chip8* chip8) {
	SDL_RenderClear(dbgRend);

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplSDL2_NewFrame(dbgWin);
	ImGui::NewFrame();

	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	
	ImGui::Begin("CHIP8 Debug");
	ImGui::End();

	SDL_RenderPresent(dbgRend);
	ImGui::EndFrame();
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
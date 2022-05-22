#include "CPU.hpp"
#include "Platform.hpp"
#include "DbgScr.hpp"
#include <cstdio>
#include <iostream>
#include <string>
#include <chrono>

#ifdef _DEBUG
#include "DbgScr.hpp"
#endif

bool quit = false;

int main(int argc, char** argv) {
#ifdef _DEBUG
	//Create debug window
	DebugWindow dw;
	dw.CreateDebugWindow(300, 600);

	int vScale = 15;
	int cycleDelay = 800;
	char const* romFilename = "roms/test.ch8";
#else	
	if (argc != 4) {
		std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
		std::exit(EXIT_FAILURE);
	}

	int vScale = std::stoi(argv[1]);
	int cycleDelay = std::stoi(argv[2]);
	char const* romFilename = argv[3];
#endif

	//Setup rendering and graphics
	Platform platform("CHIP-8 Interpreter", VIDEO_WIDTH * vScale, VIDEO_HEIGHT * vScale, VIDEO_WIDTH, VIDEO_HEIGHT);

	//Init Chip8 Sys
	Chip8 chip8;
	chip8.LoadRom(romFilename);

	int pitch = sizeof(chip8.video[0]) * VIDEO_WIDTH;

	auto lastCycle = std::chrono::high_resolution_clock::now();

	//Game Loop
	while(!quit) {
		quit = platform.ProcessInput(chip8.keypad);

		auto currTime = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::microseconds::period>(currTime - lastCycle).count();

		if (dt > cycleDelay) {
			lastCycle = currTime;

			chip8.RunCycle();

			#ifdef _DEBUG
			dw.PrintInfo(&chip8);
			#endif
			platform.Update(chip8.video, pitch);
		}
	}

	return 0;
}
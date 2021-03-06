#include "CPU.hpp"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <random>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <iomanip>

// Fixed font address at $50
const unsigned int FONTSET_START_ADDRESS = 0x50;
// Fixed start address at $200
const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_SIZE = 80;

// Used mattmikolay's Mastering Chip8 github wiki
uint8_t fontset[FONTSET_SIZE] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// randGen part is from austinmorlan
Chip8::Chip8() : randGen( std::chrono::system_clock::now().time_since_epoch().count() ) {
	// Initalize
	memset(video, 0, sizeof(video));
	memset(memory, 0, sizeof(memory));
	//memory = malloc(0x100000); //c-style
	memset(V, 0, sizeof(V));
	memset(stack, 0, sizeof(stack));
	memset(keypad, 0, sizeof(keypad));
	pc = START_ADDRESS;
	opcode = 0;
	I = 0;
	sp = 0;
	delayTimer = 0;
	soundTimer = 0;

	// Initialize RNG
	randByte = std::uniform_int_distribution<uint16_t>(0, 255U);

	// Load fonts into memory
	for (int i = 0; i < FONTSET_SIZE; ++i) {
		Chip8::memory[FONTSET_START_ADDRESS + i] = fontset[i];
	}

	// Set up function pointer table, thanks to austinmorlan for the tut
	// Master Table
	table[0x0] = &Chip8::Table0;
	table[0x1] = &Chip8::OP_1nnn;
	table[0x2] = &Chip8::OP_2nnn;
	table[0x3] = &Chip8::OP_3xnn;
	table[0x4] = &Chip8::OP_4xnn;
	table[0x5] = &Chip8::OP_5xy0;
	table[0x6] = &Chip8::OP_6xnn;
	table[0x7] = &Chip8::OP_7xnn;
	table[0x8] = &Chip8::Table8;
	table[0x9] = &Chip8::OP_9xy0;
	table[0xA] = &Chip8::OP_Annn;
	table[0xB] = &Chip8::OP_Bnnn;
	table[0xC] = &Chip8::OP_Cxbb;
	table[0xD] = &Chip8::OP_Dxyn;
	table[0xE] = &Chip8::TableE;
	table[0xF] = &Chip8::TableF;

	// Table 0
	table0[0x0] = &Chip8::OP_00E0;
	table0[0xE] = &Chip8::OP_00EE;

	// Table 8
	table8[0x0] = &Chip8::OP_8xy0;
	table8[0x1] = &Chip8::OP_8xy1;
	table8[0x2] = &Chip8::OP_8xy2;
	table8[0x3] = &Chip8::OP_8xy3;
	table8[0x4] = &Chip8::OP_8xy4;
	table8[0x5] = &Chip8::OP_8xy5;
	table8[0x6] = &Chip8::OP_8xy6;
	table8[0x7] = &Chip8::OP_8xy7;
	table8[0xE] = &Chip8::OP_8xyE;

	tableE[0x1] = &Chip8::OP_ExA1;
	tableE[0xE] = &Chip8::OP_Ex9E;

	tableF[0x07] = &Chip8::OP_Fx07;
	tableF[0x0A] = &Chip8::OP_Fx0A;
	tableF[0x15] = &Chip8::OP_Fx15;
	tableF[0x18] = &Chip8::OP_Fx18;
	tableF[0x1E] = &Chip8::OP_Fx1E;
	tableF[0x29] = &Chip8::OP_Fx29;
	tableF[0x33] = &Chip8::OP_Fx33;
	tableF[0x55] = &Chip8::OP_Fx55;
	tableF[0x65] = &Chip8::OP_Fx65;
}

void Chip8::DumpRegisters() {
	system("cls");
	std::cout << std::hex << "op: " << opcode << '\n';
	std::cout << std::hex << "pc: " << pc << ' ';
	std::cout << std::hex << "I: " << I << '\n';
	for (auto i = 0; i < REGISTER_COUNT; i++) {
		std::cout << std::hex << "v" << i << ": " << std::setw(4) << unsigned(V[i]) << '\n';
	}
	std::cout << std::endl;
}

void Chip8::LoadRom(const char* filename) {
	std::ifstream is(filename, std::ios::in | std::ios::binary);
	std::vector<char> prog(
		(std::istreambuf_iterator<char>(is)),
		std::istreambuf_iterator<char>()
	);
	is.close();
	//copy program into memory
	//copy(prog.begin(), prog.end(), memory.begin() + pc);
	for (int i = 0; i < prog.size(); ++i) {
		memory[START_ADDRESS + i] = prog[i];
	}
}

void Chip8::RunCycle() {
	//DumpRegisters();

	// Fetch
	opcode = memory[pc] << 8 | memory[pc + 1];

	// Increment the PC
	pc += 2;

	// Decode & Execute
	((*this).*(table[(opcode & 0xF000u) >> 12u]))();
	
	// Decrement the delay timer if it's been set
	if (delayTimer > 0) {
		--delayTimer;
	}
	// Decrement the sound timer if it's been set
	if (soundTimer > 0) {
		--soundTimer;
	}
}

void Chip8::Table0() {
	((*this).*(table0[opcode & 0x000Fu]))();
}

void Chip8::Table8() {
	((*this).*(table8[opcode & 0x000Fu]))();
}

void Chip8::TableE() {
	((*this).*(tableE[opcode & 0x000Fu]))();
}

void Chip8::TableF() {
	((*this).*(tableF[opcode & 0x00FFu]))();
}

void Chip8::OP_NULL() {
	//NO OPERATION
}

void Chip8::OP_00E0() {
	memset(video, 0, sizeof(video));
}

void Chip8::OP_00EE() {
	--sp;
	pc = stack[sp];
}

void Chip8::OP_1nnn() {
	uint16_t addr = opcode & 0x0FFFu;
	pc = addr;
}

void Chip8::OP_2nnn() {
	uint16_t addr = opcode & 0x0FFFu;
	stack[sp] = pc;
	++sp;
	pc = addr;
}

void Chip8::OP_3xnn() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (V[x] == byte) {
		pc += 2;
	}
}

void Chip8::OP_4xnn() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (V[x] != byte) {
		pc += 2;
	}
}

void Chip8::OP_5xy0() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;

	if (V[x] == V[y]) {
		pc += 2;
	}
}

void Chip8::OP_6xnn() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	V[x] = byte;
}

void Chip8::OP_7xnn() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	V[x] += byte;
}

void Chip8::OP_8xy0() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;

	V[x] = V[y];
}

void Chip8::OP_8xy1() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;

	V[x] |= V[y];
}

void Chip8::OP_8xy2() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;

	V[x] &= V[y];
}

void Chip8::OP_8xy3() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;

	V[x] ^= V[y];
}

void Chip8::OP_8xy4() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;

	uint16_t sum = V[x] + V[y];

	if (sum > 255u) {
		V[0xF] = 1;
	}
	else {
		V[0xF] = 0;
	}

	V[x] = sum & 0xFFu;
}

void Chip8::OP_8xy5() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;

	//uint16_t diff = V[x] - V[y];

	if (V[x] > V[y]) {
		V[0xF] = 1;
	}
	else {
		V[0xF] = 0;
	}

	V[x] -= V[y];
}

void Chip8::OP_8xy6() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	V[0xF] = (V[x] & 0x1u);

	V[x] >>= 1;
}

void Chip8::OP_8xy7() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;

	if (V[y] > V[x]) {
		V[0xF] = 1;
	}
	else {
		V[0xF] = 0;
	}

	V[x] = V[y] - V[x];
}

void Chip8::OP_8xyE() {
	uint8_t x = (opcode & 0xF00u) >> 8u;
	V[0xF] = (V[x] & 0x80u) >> 7u;

	V[x] <<= 1;
}

void Chip8::OP_9xy0() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;

	if (V[x] != V[y]) {
		pc += 2;
	}
}

void Chip8::OP_Annn() {
	uint16_t addr = opcode & 0x0FFFu;
	I = addr;
}

void Chip8::OP_Bnnn() {
	uint16_t addr = opcode & 0x0FFFu;
	pc = V[0] + addr;
}

void Chip8::OP_Cxbb() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	V[x] = randByte(randGen) & byte;
}

// Copied straight from austinmorlan's Chip8 tutorial
void Chip8::OP_Dxyn() { 
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	// Wrap if going beyond screen boundaries
	uint8_t xPos = V[x] % VIDEO_WIDTH;
	uint8_t yPos = V[y] % VIDEO_HEIGHT;

	V[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row)
	{
		uint8_t spriteByte = memory[I + row];

		for (unsigned int col = 0; col < 8; ++col)
		{
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

			// Sprite pixel is on
			if (spritePixel)
			{
				// Screen pixel also on - collision
				if (*screenPixel == 0xFFFFFFFF)
				{
					V[0xF] = 1;
				}

				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}

}

void Chip8::OP_Ex9E() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t key = V[x];

	if (keypad[key]) {
		pc += 2;
	}
}

void Chip8::OP_ExA1() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t key = V[x];

	if (!keypad[key]) {
		pc += 2;
	}
}

void Chip8::OP_Fx07() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	
	V[x] = delayTimer;
}

// Copied straight from austinmorlan's Chip8 tutorial
void Chip8::OP_Fx0A() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;

	if (keypad[0])
	{
		V[x] = 0;
	}
	else if (keypad[1])
	{
		V[x] = 1;
	}
	else if (keypad[2])
	{
		V[x] = 2;
	}
	else if (keypad[3])
	{
		V[x] = 3;
	}
	else if (keypad[4])
	{
		V[x] = 4;
	}
	else if (keypad[5])
	{
		V[x] = 5;
	}
	else if (keypad[6])
	{
		V[x] = 6;
	}
	else if (keypad[7])
	{
		V[x] = 7;
	}
	else if (keypad[8])
	{
		V[x] = 8;
	}
	else if (keypad[9])
	{
		V[x] = 9;
	}
	else if (keypad[10])
	{
		V[x] = 10;
	}
	else if (keypad[11])
	{
		V[x] = 11;
	}
	else if (keypad[12])
	{
		V[x] = 12;
	}
	else if (keypad[13])
	{
		V[x] = 13;
	}
	else if (keypad[14])
	{
		V[x] = 14;
	}
	else if (keypad[15])
	{
		V[x] = 15;
	}
	else
	{
		pc -= 2;
	}

}

void Chip8::OP_Fx15() {
	uint8_t x = (opcode & 0x0F00) >> 8u;

	delayTimer = V[x];
}

void Chip8::OP_Fx18() {
	uint8_t x = (opcode & 0x0F00) >> 8u;

	soundTimer = V[x];
}

void Chip8::OP_Fx1E() {
	uint8_t x = (opcode & 0x0F00) >> 8u;

	I += V[x];
}

void Chip8::OP_Fx29() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = V[x];

	I = FONTSET_START_ADDRESS + (5 * digit);
}

// Copied straight from austinmorlan's Chip8 tutorial
void Chip8::OP_Fx33() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t value = V[x];

	// Ones-place
	memory[I + 2] = value % 10;
	value /= 10;

	// Tens-place
	memory[I + 1] = value % 10;
	value /= 10;

	// Hundreds-place
	memory[I] = value % 10;

}

void Chip8::OP_Fx55() {
	uint8_t x = (opcode & 0x0F00) >> 8u;

	for (int i = 0; i <= x; ++i) {
		memory[I + i] = V[i];
	}
}

void Chip8::OP_Fx65() {
	uint8_t x = (opcode & 0x0F00) >> 8u;

	for (int i = 0; i <= x; ++i) {
		V[i]= memory[I + i];
	}
}
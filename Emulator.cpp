#include "Emulator.h"
#include "GPU.h"
#include <iostream>

Emulator::Emulator() : cartridge(), memory(&cartridge), cpu(&memory){
	// Tile 0: colour 0 throughout
	for (int i = 0; i < 16; i++)
		memory.write(0x8000 + i, 0x00);

	// Tile 1: colour 3 throughout
	for (int i = 0; i < 16; i += 2)
	{
		memory.write(0x8010 + i, 0xFF);
		memory.write(0x8010 + i + 1, 0xFF);
	}

	for (int y = 0; y < 18; y++)
	{
		for (int x = 0; x < 20; x++)
		{
			if ((x + y) % 2 == 0)
			{
				memory.write(0x9800 + y * 32 + x, 0);
			}
			else
			{
				memory.write(0x9800 + y * 32 + x, 1);
			}
		}
	}
};

void Emulator::run() {

	cartridge.load("graphicsDemo.gb");
	PPU ppu(&memory);
	memory.attachPPU(&ppu);

	// Reset divider
	memory.write(0xFF04, 0);

	// Set reload value
	memory.write(0xFF06, 0x42);

	// Set TIMA just before overflow
	memory.write(0xFF05, 0xFF);

	// Enable timer, select divider bit 3
	memory.write(0xFF07, 0x05);

	// First falling edge: overflow
	memory.tickTimer(16);

	std::cout << "After overflow: "
		<< (int)memory.read(0xFF05) << '\n';

	// Three more cycles
	memory.tickTimer(3);

	std::cout << "After 3 cycles: "
		<< (int)memory.read(0xFF05) << '\n';

	// Fourth cycle: reload
	memory.tickTimer(1);

	std::cout << "After 4 cycles: "
		<< (int)memory.read(0xFF05) << '\n';

	// Check timer interrupt flag
	std::cout << "IF: "
		<< (int)memory.read(0xFF0F) << '\n';

	while (cpu.running) {

		//cpu.running = false;

		int elapsed = cpu.step();

		memory.tickTimer(elapsed);

		//cpu.printState();

		ppu.step(elapsed);
		//SDL_Delay(10);
	}
}
#include "Emulator.h"
#include "GPU.h"
#include <iostream>

Emulator::Emulator() : cartridge(), memory(&cartridge), cpu(&memory){
	for (int i = 0; i < 16; i += 2)
	{
		memory.write(0x8000 + i, 0xAA);
		memory.write(0x8000 + i + 1, 0x00);
	}
};

void Emulator::run() {
	cartridge.load("haltTest2.gb");


	PPU ppu(&memory);

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

		cpu.running = false;

		int elapsed = cpu.step();

		memory.tickTimer(elapsed);

		cpu.printState();

		//ppu.step();
		//ppu.render();
		//SDL_Delay(10);
	}
}
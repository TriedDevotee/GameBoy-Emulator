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
};

void Emulator::run() {

	cartridge.load("Mario.gb");
	PPU ppu(&memory);
	memory.attachPPU(&ppu);

	memory.write(0xFE02, 1);

	memory.write(0xFE02, 1);

	memory.write(0xFF00, 0xFF);

	std::cout << "OAM tile immediately after write: "
		<< (int)memory.read(0xFE02) << '\n';

	std::cout << std::hex
		<< "Tile 1: "
		<< (int)memory.read(0x8010) << " "
		<< (int)memory.read(0x8011) << '\n';

	while (cpu.running) {

		//cpu.running = false;

		int elapsed = cpu.step();

		memory.tickTimer(elapsed);

		//cpu.printState();

		ppu.step(elapsed);
		//SDL_Delay(10);
	}
}
#include "Emulator.h"
#include "GPU.h"

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

	while (cpu.running) {

		cpu.step();
		cpu.printState();

		//ppu.step();
		//ppu.render();
		//SDL_Delay(10);
	}
}
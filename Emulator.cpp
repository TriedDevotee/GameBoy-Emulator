#include "Emulator.h"
#include "GPU.h"
#include <iostream>
#include <SDL.h>

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

	cartridge.load("chess.gb");
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

        SDL_Event event;

        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_QUIT) {
                cpu.running = false;
            }

            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {

                bool pressed = (event.type == SDL_KEYDOWN);

                switch (event.key.keysym.sym) {

                    // D-pad
                case SDLK_RIGHT:
                case SDLK_d:
                    memory.setJoypadDpad(0, pressed);
                    break;

                case SDLK_LEFT:
                case SDLK_a:
                    memory.setJoypadDpad(1, pressed);
                    break;

                case SDLK_UP:
                case SDLK_w:
                    memory.setJoypadDpad(2, pressed);
                    break;

                case SDLK_DOWN:
                case SDLK_s:
                    memory.setJoypadDpad(3, pressed);
                    break;

                    // Buttons
                case SDLK_k:
                    memory.setJoypadButton(0, pressed);
                    break;

                case SDLK_j:
                    memory.setJoypadButton(1, pressed);
                    break;

                case SDLK_RSHIFT:
                    memory.setJoypadButton(2, pressed);
                    break;

                case SDLK_RETURN:
                    memory.setJoypadButton(3, pressed);
                    break;

                case SDLK_ESCAPE:
                    if (pressed)
                        cpu.running = false;
                    break;
                }
            }
        }

		int elapsed = cpu.step();

		memory.tickTimer(elapsed);

		//cpu.printState();

		ppu.step(elapsed);
		//SDL_Delay(10);
	}
}
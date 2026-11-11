#pragma once
#include "Memory.h"
#include <cstdint>
#include <SDL.h>

class PPU {
	public:
		PPU(Memory* memory);
		~PPU();
		void step();
		void render();
		void drawTile(uint16_t tileIndex, int screenX, int screenY);
		uint32_t convertColor(uint8_t color);
		void drawBackground();
	private:
	    Memory* memory;
	    int cycles;
	    uint32_t framebuffer[160 * 144];

	    SDL_Window* Window;
	    SDL_Renderer* Renderer;
		SDL_Texture* Texture;
};
#pragma once

#include <cstdint>
#include <SDL.h>

class Memory;

class PPU {
	public:

		enum class PPUmode {
			HBlank = 0,
			VBlank = 1,
			OAMScan = 2,
			Drawing = 3
		};

		PPU(Memory* memory);
		~PPU();
		void step(int tCycles);
		void render();
		void drawTile(uint16_t tileIndex, int screenX, int screenY);
		uint32_t convertColor(uint8_t color);
		void drawBackground();
		void drawScanline();

		uint8_t readRegister(uint16_t address);
		void writeRegister(uint16_t address, uint8_t value);

	private:

		uint8_t LCDC = 0x91;
		uint8_t STAT = 0x00;

		uint8_t SCY = 0;
		uint8_t SCX = 0;

		uint8_t LY = 0;
		uint8_t LYC = 0;

		uint8_t BGP = 0xFC;

	    Memory* memory;
	    uint32_t framebuffer[160 * 144];

	    SDL_Window* Window;
	    SDL_Renderer* Renderer;
		SDL_Texture* Texture;

		int cycles = 0;
		PPUmode mode = PPUmode::OAMScan;

		void updateMode();
		void updateSTAT();

};
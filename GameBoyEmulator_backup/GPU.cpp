#include "GPU.h"

PPU::PPU(Memory* memory) {
	for (int i = 0; i < 160 * 144; i++)
	{
		framebuffer[i] = 0xFFFFFFFF;
	}

	this->memory = memory;

	Window = SDL_CreateWindow(
		"GameBoy Emulator",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		640,
		576,
		SDL_WINDOW_SHOWN
	);

	Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED);

	Texture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 160, 144);

	framebuffer[72 * 160 + 80] = 0xFF000000;
}

PPU::~PPU() {
	SDL_DestroyTexture(Texture);
	SDL_DestroyRenderer(Renderer);
	SDL_DestroyWindow(Window);
}

void PPU::render() {
	SDL_UpdateTexture(Texture, NULL, framebuffer, 160 * sizeof(uint32_t));
	SDL_RenderClear(Renderer);
	SDL_RenderCopy(Renderer, Texture, NULL, NULL);
	SDL_RenderPresent(Renderer);
}

void PPU::step() {

	std::fill(framebuffer, framebuffer + 160 * 144, 0xFFFFFFFF);

	drawBackground();

	for (int y = 0; y < 18; y++)
	{
		for (int x = 0; x < 20; x++)
		{
			if ((x + y) % 2 == 0)
			{
				memory->write(0x9800 + y * 32 + x, 0);
			}
			else
			{
				memory->write(0x9800 + y * 32 + x, 1);
			}
		}
	}
}

void PPU::drawBackground()
{
	for (int y = 0; y < 18; y++)
	{
		for (int x = 0; x < 20; x++)
		{
			uint8_t tile =
				memory->read(0x9800 + y * 32 + x);

			drawTile(
				0x8000 + tile * 16,
				x * 8,
				y * 8
			);
		}
	}
}


void PPU::drawTile(uint16_t address, int screenX, int screenY) {
	for (int row = 0; row < 8; row++) {
		uint8_t low = memory->read(address + row * 2);
		uint8_t high = memory->read(address + row * 2 + 1);

		for (int col = 0; col < 8; col++) {
			int bit = 7 - col;
			uint8_t lowBit = (low >> bit) & 1;
			uint8_t highBit = (high >> bit) & 1;
			uint8_t color = (highBit << 1) | lowBit;

			framebuffer[(screenY + row) * 160 + (screenX + col)] = convertColor(color);
		}
	}
}

uint32_t PPU::convertColor(uint8_t color) {
	switch (color) {
		case 0:
			return 0xFFFFFFFF;
		case 1:
			return 0xFFAAAAAA;
		case 2:
			return 0xFF555555;
		case 3:
			return 0xFF000000;
		default:
			return 0xFFFFFFFF;
		}
}


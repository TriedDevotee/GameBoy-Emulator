#include "GPU.h"
#include "Memory.h"
#include "interrupts.h"
#include <iostream>

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

	static int frames = 0;
	static Uint64 last = SDL_GetPerformanceCounter();

	frames++;

	Uint64 now = SDL_GetPerformanceCounter();

	if ((now - last) >= SDL_GetPerformanceFrequency())
	{
		std::cout << "FPS: " << frames << '\n';
		frames = 0;
		last = now;
	}
}

void PPU::step(int tCycles) {
	if (!(LCDC & 0x80)) return;

	cycles += tCycles;

	switch (mode) {
		case PPUmode::OAMScan:
			if (cycles >= 80) {
				cycles -= 80;
				mode = PPUmode::Drawing;
			}
			break;
		case PPUmode::Drawing:
			if (cycles >= 172) {
				cycles -= 172;

				drawScanline();

				mode = PPUmode::HBlank;
			}
			break;
		case PPUmode::HBlank:
			if (cycles >= 204) {
				cycles -= 204;
				LY++;

				/*if (LY == 144 || LY == 148 || LY == 153) {
					std::cout << "LY REACHED: " << (int)LY
						<< " | MODE: " << (int)mode << '\n';
				}*/

				if (LY == 144) {
					mode = PPUmode::VBlank;

					static int frameCount = 0;
					frameCount++;

					if (frameCount % 60 == 0) {
						int nonWhitePixels = 0;

						for (int i = 0; i < 160 * 144; i++) {
							if (framebuffer[i] != 0xFFFFFFFF)
								nonWhitePixels++;
						}

						/*std::cout << "FRAME: " << std::dec << frameCount
							<< " | LY: " << (int)LY
							<< " | LCDC: " << std::hex << (int)LCDC
							<< " | BGP: " << (int)BGP
							<< " | Non-white pixels: " << std::dec << nonWhitePixels
							<< '\n';*/
					}

					render();

					memory->requestInterrupt(Interrupts::VBLANK);
				}
				else {
					mode = PPUmode::OAMScan;
				}
			}
			break;
		case PPUmode::VBlank:
			if (cycles >= 456) {
				cycles -= 456;
				LY++;

				/*if (LY == 144 || LY == 148 || LY == 153) {
					std::cout << "LY REACHED: " << (int)LY
						<< " | MODE: " << (int)mode << '\n';
				}*/


				if (LY > 153) {
					LY = 0;
					mode = PPUmode::OAMScan;
				}
			}
			break;
	}

	updateSTAT();
}

void PPU::drawScanline()
{

	//std::cout << "Drawing scanline: " << (int)LY << '\n';

	if (!(LCDC & 0x01)) {

		for (int x = 0; x < 160; x++) {
			framebuffer[LY * 160 + x] = convertColor(0, BGP);
		}

		drawSpritesForScanline();

		return;
	}

	uint16_t mapAddress = (LCDC & 0x08) ? 0x9C00 : 0x9800;

	int y = LY;

	uint8_t bgY = (y + SCY) & 0xFF;

	for (int x = 0; x < 160; x++)
	{
		uint8_t bgX = (x + SCX) & 0xFF;

		uint16_t mapIndex =
			mapAddress + (bgY / 8) * 32 + (bgX / 8);

		uint8_t tile = memory->read(mapIndex);

		uint16_t tileAddress = 0;

		if (LCDC & 0x10)
			tileAddress = 0x8000 + tile * 16;
		else
			tileAddress = 0x9000 + static_cast<int8_t>(tile) * 16;

		uint8_t row = bgY % 8;
		uint8_t col = bgX % 8;

		uint8_t low = memory->read(tileAddress + row * 2);
		uint8_t high = memory->read(tileAddress + row * 2 + 1);

		int bit = 7 - col;

		uint8_t color =
			(((high >> bit) & 1) << 1) |
			((low >> bit) & 1);

		framebuffer[y * 160 + x] = convertColor(color, BGP);
	}

	drawSpritesForScanline();
}

void PPU::drawBackground()
{
	uint16_t mapAddress = (LCDC & 0x08) ? 0x9C00 : 0x9800;

	for (int y = 0; y < 144; y++)
	{
		for (int x = 0; x < 160; x++)
		{
			uint8_t bgX = (x + SCX) & 0xFF;
			uint8_t bgY = (y + SCY) & 0xFF;

			uint16_t mapIndex =
				mapAddress + (bgY / 8) * 32 + (bgX / 8);

			uint8_t tile = memory->read(mapIndex);

			uint16_t tileAddress;

			if (LCDC & 0x10)
				tileAddress = 0x8000 + tile * 16;
			else
				tileAddress = 0x9000 + (int8_t) (tile) * 16;

			uint8_t row = bgY % 8;
			uint8_t col = bgX % 8;

			uint8_t low = memory->read(tileAddress + row * 2);
			uint8_t high = memory->read(tileAddress + row * 2 + 1);

			int bit = 7 - col;

			uint8_t color =
				(((high >> bit) & 1) << 1) |
				((low >> bit) & 1);

			framebuffer[y * 160 + x] = convertColor(color, BGP);
		}
	}
}


void PPU::drawTile(uint16_t address, int screenX, int screenY) {
	for (int row = 0; row < 8; row++) {
		uint8_t low = memory->read(address + (row * 2));
		uint8_t high = memory->read((address + (row * 2)) + 1);

		for (int col = 0; col < 8; col++) {
			int bit = 7 - col;
			uint8_t lowBit = (low >> bit) & 1;
			uint8_t highBit = (high >> bit) & 1;
			uint8_t color = (highBit << 1) | lowBit;

			framebuffer[(screenY + row) * 160 + (screenX + col)] = convertColor(color, BGP);
		}
	}
}

uint32_t PPU::convertColor(uint8_t color, uint8_t palette) {

	uint8_t shade = (palette >> (color * 2)) & 0x03;

	switch (shade) {
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

uint8_t PPU::readRegister(uint16_t address) {
	switch (address) {
		case 0xFF40:
			return LCDC;
		case 0xFF41:
			return STAT;
		case 0xFF42:
			return SCY;
		case 0xFF43:
			return SCX;
		case 0xFF44:
			return LY;
		case 0xFF45:
			return LYC;
		case 0xFF47:
			return BGP;
		case 0xFF48:
			return OBP0;
		case 0xFF49:
			return OBP1;
	}

	return 0;
}

void PPU::writeRegister(uint16_t address, uint8_t value) {
	switch (address) {
		case 0xFF40:
		{

			/*std::cout << "LCDC WRITE: "
				<< std::hex << (int)LCDC
				<< " -> " << (int)value << '\n';*/

			bool wasEnabled = LCDC & 0x80;
			LCDC = value;
			bool isEnabled = LCDC & 0x80;

			if (wasEnabled && !isEnabled) {
				LY = 0;
				cycles = 0;
				mode = PPUmode::HBlank;
				updateSTAT();
			}
			else if (!wasEnabled && isEnabled) {
				LY = 0;
				cycles = 0;
				mode = PPUmode::OAMScan;
				updateSTAT();
			}

			break;
		}
		case 0xFF41:
			STAT = (STAT & 0x07) | (value & 0xF8);
			break;
		case 0xFF42:
			SCY = value;
			break;
		case 0xFF43:
			SCX = value;
			break;
		case 0xFF44:
			break; //apparently LY is not writeable. Boring
		case 0xFF45:
			LYC = value;
			updateSTAT();
			break;
		case 0xFF47:
			BGP = value;
			break;
		case 0xFF48:
			OBP0 = value;
			break;
		case 0xFF49:
			OBP1 = value;
			break;
	}

	return;
}

void PPU::updateSTAT() {

	STAT = (STAT & 0xFC) | static_cast<uint8_t>(mode);

	if (LY == LYC) {
		STAT |= 0x04;
	}
	else {
		STAT &= ~0x04;
	}
}

void PPU::drawSpritesForScanline() {

	if (!(LCDC & 0x02)) return;


	for (int i = 39; i >= 0; i--) {
		uint16_t address = 0xFE00 + (i * 4);

		uint8_t spriteY = memory->read(address);
		uint8_t spriteX = memory->read(address + 1);
		uint8_t tile = memory->read(address + 2);
		uint8_t attributes = memory->read(address + 3);

		int screenY = spriteY - 16;
		int screenX = spriteX - 8;

		int row = LY - screenY;

		if (row >= 0 && row < 8) {
			if (attributes & 0x40) {
				row = 7 - row;
			}

			uint16_t tileAddress = 0x8000 + tile * 16;
			
			uint8_t low = memory->read(tileAddress + row * 2);
			uint8_t high = memory->read(tileAddress + row * 2 + 1);

			bool flipX = attributes & 0x20;

			uint8_t palette = 0;
			if (attributes & 0x10) {
				palette = OBP1;
			}
			else {
				palette = OBP0;
			}

			for (int col = 0; col < 8; col++) {
				int bit = flipX ? col : 7 - col;

				uint8_t color = (((high >> bit) & 1) << 1) | ((low >> bit) & 1);


				if (!(color == 0)) {
					int x = screenX + col;

					if (x >= 0 && x < 160) {

						framebuffer[LY * 160 + x] = convertColor(color, palette);
					}
				}
				else {
					//std::cout << "Ha no color is 0\n";
				}
			}

		}
	}
}
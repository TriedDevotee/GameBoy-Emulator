#pragma once
#include "Cartridge.h"
#include <cstdint>
#include "GPU.h"

class Timer;

class Memory {
private:
	Cartridge* cartridge;
	uint8_t data [65536];
	Timer* timer;
	PPU* ppu = nullptr;

public:

	Memory(Cartridge* cartridge);
	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t value);

	void attachPPU(PPU* ppu);
	
	void requestInterrupt(uint8_t interrupt);
	void tickTimer(int cycles);
};
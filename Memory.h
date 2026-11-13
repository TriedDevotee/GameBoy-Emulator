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

	uint8_t joypadSelect = 0x30;  // bits 4-5 as last written
	uint8_t buttonState = 0x0F;  // bit0 A, bit1 B, bit2 Select, bit3 Start (0 = pressed)
	uint8_t dpadState = 0x0F;  // bit0 Right, bit1 Left, bit2 Up, bit3 Down (0 = pressed)

public:

	Memory(Cartridge* cartridge);
	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t value);

	void attachPPU(PPU* ppu);
	
	void requestInterrupt(uint8_t interrupt);
	void tickTimer(int cycles);

	uint64_t LCDCWriteCount = 0;
	uint64_t getLCDCCount();
};
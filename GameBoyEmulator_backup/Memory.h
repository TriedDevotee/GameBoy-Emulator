#pragma once
#include "Cartridge.h"
#include <cstdint>

class Memory {
private:
	Cartridge* cartridge;
	uint8_t data [65536];
public:
	Memory(Cartridge* cartridge);
	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t value);
	
	void requestInterrupt(uint8_t interrupt);
};
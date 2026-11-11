#include <iostream>
#include <cstdint>
#include "Memory.h"
#include "Cartridge.h"
#include "interrupts.h"

Memory::Memory(Cartridge* cartridge) {
	std::cout << "Memory is Online\n";

	for (int i = 0; i < 65536; i++) {
		data[i] = 0;
	}
	this->cartridge = cartridge;
	memset(data, 0, sizeof(data));
}

uint8_t Memory::read(uint16_t address) {
	if (address < 0x8000) {
		return cartridge->read(address);
	}

	return data[address];
}

void Memory::write(uint16_t address, uint8_t value) {
	data[address] = value;
}

void Memory::requestInterrupt(uint8_t interrupt) {
	uint8_t IF = read(Interrupts::IF);
	IF |= (1 << interrupt);
	write(Interrupts::IF, IF);
}
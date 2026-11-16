#include <iostream>
#include <cstdint>
#include <cstring>
#include "Memory.h"
#include "Cartridge.h"
#include "interrupts.h"
#include "timer.h"

Memory::Memory(Cartridge* cartridge) {
	std::cout << "Memory is Online\n";

	this->cartridge = cartridge;
	memset(data, 0, sizeof(data));

	timer = new Timer(this);
}

void Memory::dumpVRAM() {
	printf("--- BG MAP 9800 ---\n");
	for (int row = 0; row < 32; row++) {
		for (int col = 0; col < 32; col++)
			printf("%02X ", data[0x9800 + row * 32 + col]);
		printf("\n");
	}

	int nonZero = 0;
	for (int i = 0x8000; i < 0x9800; i++)
		if (data[i] != 0) nonZero++;

	printf("Tile data non-zero bytes: %d / 6144\n", nonZero);
	printf("LCDC=%02X SCX=%02X SCY=%02X BGP=%02X\n",
		ppu->readRegister(0xFF40), ppu->readRegister(0xFF43),
		ppu->readRegister(0xFF42), ppu->readRegister(0xFF47));
}

uint8_t Memory::read(uint16_t address) {
	if (address == 0xFF00) {
		uint8_t result = 0xC0 | (joypadSelect & 0x30) | 0x0F;
		if (!(joypadSelect & 0x20)) result &= (buttonState | 0xF0);  // bit 5 low selects buttons
		if (!(joypadSelect & 0x10)) result &= (dpadState | 0xF0);  // bit 4 low selects d-pad
																   // And thats what broke everything
		return result;
	}

	if (address >= 0xA000 && address <= 0xBFFF) {
		return cartridge->readRam(address);
	}

	if (address >= 0xFF04 && address <= 0xFF07) {
		return timer->read(address);
	}

	if ((address >= 0xFF40 && address <= 0xFF45) || (address >= 0xFF47 && address <= 0xFF49)) {
		return ppu->readRegister(address);
	}

	if (address < 0x8000) {
		return cartridge->read(address);
	}

	return data[address];
}

void Memory::write(uint16_t address, uint8_t value) {

	if (address == 0xFF02 && value == 0x81) {
		std::cout << (char)data[0xFF01] << std::flush;
		data[0xFF02] = 0;
		return;
	}

	if (address >= 0xA000 && address <= 0xBFFF) {
		cartridge->writeRam(address, value);
		return;
	}

	if (address == 0xFF00) {
		joypadSelect = value & 0x30;
		return;
	}

	if (address == 0xFF40)
		LCDCWriteCount++;
	
	if (address < 0x8000) {
		cartridge->write(address, value);
		return;
	}

	if (address == 0xFF46) {
		uint16_t source = value << 8;
		for (int i = 0; i < 0xA0; i++) {
			data[0xFE00 + i] = read(source + i);
		}
		data[0xFF46] = value;
		return;
	}

	if (address >= 0xFF04 && address <= 0xFF07) {
		timer->write(address, value);
		return;
	}

	if ((address >= 0xFF40 && address <= 0xFF45) || (address >= 0xFF47 && address <= 0xFF49)) {
		ppu->writeRegister(address, value);
		return;
	}

	data[address] = value;
}

void Memory::requestInterrupt(uint8_t interrupt) {

	uint8_t IF = read(Interrupts::IF);
	IF |= (1 << interrupt);
	write(Interrupts::IF, IF);
}

void Memory::tickTimer(int cycles) {
	timer->tick(cycles);
}

void Memory::attachPPU(PPU* ppu) {
	this->ppu = ppu;
}

uint64_t Memory::getLCDCCount() {
	return LCDCWriteCount;
}

void Memory::setJoypadButton(uint8_t bit, bool pressed) {
	if (bit > 3) return;

	uint8_t mask = 1 << bit;

	if (pressed)
		buttonState &= ~mask;
	else
		buttonState |= mask;
}

void Memory::setJoypadDpad(uint8_t bit, bool pressed) {
	if (bit > 3) return;

	uint8_t mask = 1 << bit;

	if (pressed)
		dpadState &= ~mask;
	else
		dpadState |= mask;
}
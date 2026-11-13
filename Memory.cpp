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

uint8_t Memory::read(uint16_t address) {
	if (address == 0xFF00) {
		uint8_t result = 0xC0 | (joypadSelect & 0x30) | 0x0F;
		if (!(joypadSelect & 0x20)) result &= (buttonState | 0xF0);  // bit 5 low selects buttons
		if (!(joypadSelect & 0x10)) result &= (dpadState | 0xF0);  // bit 4 low selects d-pad
		return result;
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
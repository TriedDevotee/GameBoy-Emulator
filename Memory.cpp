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
	if (address >= 0xFF04 && address <= 0xFF07) {
		return timer->read(address);
	}

	if (address >= 0xFF40 && address <= 0xFF45 || address == 0xFF47) {
		return ppu->readRegister(address);
	}

	if (address < 0x8000) {
		return cartridge->read(address);
	}

	return data[address];
}

void Memory::write(uint16_t address, uint8_t value) {
	
	if (address >= 0xFF04 && address <= 0xFF07) {
		timer->write(address, value);
		return;
	}

	if (address >= 0xFF40 && address <= 0xFF45 || address == 0xFF47) {
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
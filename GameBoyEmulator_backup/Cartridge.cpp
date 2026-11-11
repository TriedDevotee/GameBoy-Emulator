#include <iostream>
#include <string>
#include <fstream>
#include <iterator>
#include <ios>
#include <vector>
#include "Cartridge.h"

std::vector<uint8_t> rom;

Cartridge::Cartridge() {
	std::cout << "Cartridge is Online\n";
}

bool Cartridge::load(std::string filename) {
	std::ifstream file(filename, std::ios::binary);
	if (!file) {
		std::cout << "Failed to load ROM\n";
		return false;
	}

	rom.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	return true;
}

uint8_t Cartridge::read(uint16_t address) {
	return rom[address];
}
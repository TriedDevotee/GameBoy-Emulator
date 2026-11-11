#include <iostream>
#include <string>
#include <fstream>
#include <iterator>
#include <ios>
#include <vector>
#include <stdexcept>
#include "Cartridge.h"

//std::vector<uint8_t> rom;

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

	std::cout << "Loaded ROM size: " << rom.size() << " bytes\n";

	return true;
}

uint8_t Cartridge::read(uint16_t address) {

	if (address >= rom.size()) {
		std::cout << "ROM OUT OF BOUNDS!\n";
		std::cout << "Address: " << std::hex << address << '\n';
		std::cout << "ROM size: " << std::dec << rom.size() << '\n';

		throw std::out_of_range("Cartridge ROM read out of bounds");
	}
		

	return rom[address];
}
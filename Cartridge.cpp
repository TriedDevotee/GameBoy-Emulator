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

	if (rom.size() < 0x150) {
		std::cout << "ROM is too small to contain a valid header";
		rom.clear();
		return false;
	}

	cartridgeType = rom[0x147];
	romSize = rom[0x148];
	ramSize = rom[0x149];

	std::cout << "Loaded ROM size: " << rom.size() << " bytes\n";

    printHeader();

	return true;
}

void Cartridge::printHeader() {
	std::cout << "\n--- Cartridge Header ---\n";

	std::cout << "Title: ";

	for (int i = 0x134; i <= 0x143; i++) {
		char c = (char)(rom[i]);

		if (c == '\0') {
			break;
		}

		std::cout << c;
	}

	std::cout << "\nCartridge type: 0x"
		<< std::hex << (int)cartridgeType;

	std::cout << "\nROM size code: 0x"
		<< (int)romSize;

	std::cout << "\nRAM size code: 0x"
		<< (int)ramSize;

	std::cout << "\n------------------------\n";

	std::cout << std::dec;
}

uint8_t Cartridge::read(uint16_t address) {
    if (address >= 0x8000) {
        throw std::out_of_range("Invalid cartridge ROM address");
    }

    size_t bank = 0;

    bool mbc1 = cartridgeType >= 0x01 && cartridgeType <= 0x03;

    if (!mbc1) {
        bank = (address >= 0x4000) ? 1 : 0;
    }
    else {
        if (address < 0x4000) {
            bank = (bankingMode == 1) ? (bankHigh2 << 5) : 0;
        }
        else {
            bank = (bankHigh2 << 5) | romBankLow5;
        }
    }

    size_t bankCount = (rom.size() + 0x3FFF) / 0x4000;

    if (bankCount == 0) {
        return 0xFF;
    }

    bank %= bankCount;

    size_t index = bank * 0x4000 + (address & 0x3FFF);

    if (index >= rom.size()) {
        return 0xFF;
    }

    return rom[index];
}

void Cartridge::write(uint16_t address, uint8_t value) {
    if (cartridgeType < 0x01 || cartridgeType > 0x03) {
        return;
    }

    if (address >= 0x2000 && address <= 0x3FFF) {
        romBankLow5 = value & 0x1F;

        if (romBankLow5 == 0) {
            romBankLow5 = 1;
        }
    }
    else if (address >= 0x4000 && address <= 0x5FFF) {
        bankHigh2 = value & 0x03;
    }
    else if (address >= 0x6000 && address <= 0x7FFF) {
        bankingMode = value & 0x01;
    }
}


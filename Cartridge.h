#pragma once
#include <string>
#include <vector>

class Cartridge {
public:
	Cartridge();
	bool load(std::string filename);
	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t value);
	void printHeader();
private:
	std::vector<uint8_t> rom;

	uint8_t cartridgeType = 0;
	uint8_t romSize = 0;
	uint8_t ramSize = 0;

	uint8_t romBankLow5 = 1;
	uint8_t bankHigh2 = 0;
	uint8_t bankingMode = 0;
};
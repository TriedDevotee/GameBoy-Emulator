#pragma once
#include <string>
#include <vector>

class Cartridge {
public:
	Cartridge();
	bool load(std::string filename);
	uint8_t read(uint16_t address);
private:
	std::vector<uint8_t> rom;
};
#pragma once

#include "CPU.h"
#include "Memory.h"
#include "Cartridge.h"
#include <cstring>

class Emulator {
private:
	Cartridge cartridge;
	Memory memory;
	CPU cpu;
public:
	Emulator();
	void run(std::string romName);
};
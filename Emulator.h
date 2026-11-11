#pragma once

#include "CPU.h"
#include "Memory.h"
#include "Cartridge.h"

class Emulator {
private:
	Cartridge cartridge;
	Memory memory;
	CPU cpu;
public:
	Emulator();
	void run();
};
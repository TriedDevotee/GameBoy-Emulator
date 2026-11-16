#pragma once
#include <cstdint>

class Memory;

class Timer {
public:
	Timer(Memory* memory);

	void tick(int tCycles);

	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t value);

private:
	Memory* memory;

	uint16_t dividerCounter = 0;
	uint8_t TIMA = 0;
	uint8_t TMA = 0;
	uint8_t TAC = 0;

	int overflow_delay = -1;

	bool timerSignal() const;
	void incrementTIMA();
};
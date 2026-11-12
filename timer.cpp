#include "Memory.h"
#include "timer.h"
#include "interrupts.h"
#include <cstdint>

Timer::Timer(Memory* memory) {
	this->memory = memory;

}

void Timer::tick(int tCycles) {
	for (int i = 0; i < tCycles; i++) {

		bool reloaded = false;

		if (overflow_delay > 0) {

			overflow_delay--;

			if (overflow_delay == 0) {
				TIMA = TMA;
				overflow_delay = -1;

				memory->requestInterrupt(Interrupts::TIMER);
				reloaded = true;
			}
		}

		bool oldSignal = timerSignal();

		dividerCounter++;

		bool newSignal = timerSignal();

		if (oldSignal && !newSignal) {
			incrementTIMA();
		} 
	}
}

uint8_t Timer::read(uint16_t address) {
	switch (address) {
		case 0xFF04:
			return dividerCounter >> 8;
		case 0xFF05:
			return TIMA;
		case 0xFF06:
			return TMA;
		case 0xFF07:
			return TAC;
	}

	return 0xFF;
}

void Timer::write(uint16_t address, uint8_t value) {
	switch (address) {
		case 0xFF04:
			dividerCounter = 0;
			break;
		case 0xFF05:
			TIMA = value;
			if (overflow_delay > 0) {
				overflow_delay = -1;
			}
			break;
		case 0xFF06:
			TMA = value;
			break;
		case 0xFF07:
			TAC = value & 0x07;
			break;
	}
}

bool Timer::timerSignal() const {
	if (!(TAC & 0x04)) return false;

	int bit = 0;

	switch (TAC & 0x03) {
		case 0:
			bit = 9;
			break;
		case 1:
			bit = 3;
			break;
		case 2:
			bit = 5;
			break;
		case 3:
			bit = 7;
			break;
	}

	return (dividerCounter & (1 << bit)) != 0;
}

void Timer::incrementTIMA() {
	if (overflow_delay != -1) {
		return;
	}

	if (TIMA == 0xFF) {
		TIMA = 0x00;
		overflow_delay = 4;
	}

	else {
		TIMA++;
	}
}
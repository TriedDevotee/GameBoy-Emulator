#define SDL_MAIN_HANDLED

#include <iostream>
#include <SDL.h>
#include "Emulator.h"

int main() {

	Emulator emulator;

	emulator.run();

	return 0;
}
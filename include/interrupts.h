#pragma once
#include <cstdint>

namespace Interrupts {
	constexpr uint16_t IF = 0xFF0F;
	constexpr uint16_t IE = 0xFFFF;

	constexpr uint8_t VBLANK = 0;
	constexpr uint8_t LCD_STAT = 1;
	constexpr uint8_t TIMER = 2;
	constexpr uint8_t SERIAL = 3;
	constexpr uint8_t JOYPAD = 4;

	constexpr uint16_t vectors[] = {
		0x0040,
		0x0048,
		0x0050,
		0x0058,
		0x0060
	};
}
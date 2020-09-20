#include "vram_address.h"

uint16_t vram_address(uint16_t base, uint8_t x, uint8_t y) {
    return base + (x % 128 >= 64 ? 0x1000 : 0) + x % 64 + (y % 64) * 64;
}

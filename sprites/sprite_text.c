#include "sprite_text.h"

#include "vdp.h"
#include "math_util.h"

#include "sprite_buffer.h"
#include "sprite_text_attributes.h"

static void write_char(char c, uint32_t *x, uint32_t y);
static void write_hex_char(uint32_t value, uint32_t *x, uint32_t y);

static uint16_t div_mod_10(uint16_t dividend, uint32_t *remainder);

void st_write(const char *text, uint32_t x, uint32_t y) {
    while (*text) {
        write_char(*text++, &x, y);
    }
}

void st_write_decimal(uint8_t value, uint32_t x, uint32_t y) {
    uint32_t ones_digit = 0;
    uint32_t tens_digit = div_mod_10(value, &ones_digit);

    if (tens_digit) {
        write_char('0' + tens_digit, &x, y);
    }
    write_char('0' + ones_digit, &x, y);
}

void st_write_hex(uint16_t value, uint32_t x, uint32_t y) {
    write_hex_char(value >> 12 & 0xf, &x, y);
    write_hex_char(value >> 8 & 0xf, &x, y);
    write_hex_char(value >> 4 & 0xf, &x, y);
    write_hex_char(value & 0xf, &x, y);
}

static void write_hex_char(uint32_t value, uint32_t *x, uint32_t y) {
    if (value >= 10) {
        value += 'A' - 10;
    } else {
        value += '0';
    }

    write_char(value, x, y);
}

static void write_char(char c, uint32_t *x, uint32_t y) {
    const uint16_t tile_base = 0x180;
    const uint8_t palette = 9;

    if (c == ' ') {
        *x += 8;
        return;
    }

    // Default to uppercase
    if (c >= 'a' && c != 'x') {
        c -= ('a' - 'A');
    }

    const SpriteTextCharAttributes *attributes = st_char_attributes(c);

    uint16_t tile = attributes->base_tile;

    uint32_t x_block = *x & 0x3ff;

    uint32_t y_block = y;
    y_block |= SPRITE_16_TALL;
    y_block |= (attributes->wide ? SPRITE_16_WIDE : 0);

    uint32_t g_block = tile_base + tile;
    g_block |= palette << SPRITE_PAL_SHIFT;
    g_block |= 3 << SPRITE_PRIORITY_SHIFT;

    sb_write_priority(x_block, y_block, g_block);

    *x += attributes->width;
}

// This is only intended for small dividends where the fixed point approximation works
static uint16_t div_mod_10(uint16_t dividend, uint32_t *remainder) {
    const int32_t div_Q_1 = 0x4000;
    const int32_t inverse_divisor = (div_Q_1 / 10) + 1;

    uint16_t quotient = sys_multiply(dividend, inverse_divisor) / div_Q_1;
    if (remainder) {
        *remainder = dividend - sys_multiply(quotient, 10);
    }

    return quotient;
}

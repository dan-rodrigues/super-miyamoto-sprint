#include "sprite_text.h"

#include "vdp.h"
#include "math_util.h"

#include "sprite_buffer.h"

static const uint8_t BASE_TILE_BLANK = 0x7f;

typedef struct {
    uint8_t base_tile;
    uint8_t width : 4;
    bool wide : 1;
} CharAttributes;

static void write_char(char c, uint32_t *x, uint32_t y);
static void write_hex_char(uint32_t value, uint32_t *x, uint32_t y);

static uint16_t div_mod_10(uint16_t dividend, uint32_t *remainder);

static const CharAttributes *char_attributes(char c);

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

    const CharAttributes *attributes = char_attributes(c);

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

static const CharAttributes attributes[128] = {
    { 0 },   // U+0000 (nul)
    { 0 },   // U+0001
    { 0 },   // U+0002
    { 0 },   // U+0003
    { 0 },   // U+0004
    { 0 },   // U+0005
    { 0 },   // U+0006
    { 0 },   // U+0007
    { 0 },   // U+0008
    { 0 },   // U+0009
    { 0 },   // U+000A
    { 0 },   // U+000B
    { 0 },   // U+000C
    { 0 },   // U+000D
    { 0 },   // U+000E
    { 0 },   // U+000F
    { 0 },   // U+0010
    { 0 },   // U+0011
    { 0 },   // U+0012
    { 0 },   // U+0013
    { 0 },   // U+0014
    { 0 },   // U+0015
    { 0 },   // U+0016
    { 0 },   // U+0017
    { 0 },   // U+0018
    { 0 },   // U+0019
    { 0 },   // U+001A
    { 0 },   // U+001B
    { 0 },   // U+001C
    { 0 },   // U+001D
    { 0 },   // U+001E
    { 0 },   // U+001F
    { .base_tile = BASE_TILE_BLANK },                   // U+0020 (space)
    { .base_tile = 0x42, .width = 7, .wide = false },   // U+0021 (!)
    { 0 },   // U+0022 (")
    { 0 },   // U+0023 (#)
    { 0 },   // U+0024 ($)
    { 0 },   // U+0025 (%)
    { 0 },   // U+0026 (&)
    { 0 },   // U+0027 (')
    { 0 },   // U+0028 (()
    { 0 },   // U+0029 ())
    { .base_tile = 0x45, .width = 11, .wide = true },   // U+002A (*)
    { 0 },   // U+002B (+)
    { 0 },   // U+002C (,)
    { 0 },   // U+002D (-)
    { .base_tile = 0x47, .width = 5, .wide = false },   // U+002E (.)
    { .base_tile = 0x48, .width = 9, .wide = false },   // U+002F (/)
    { .base_tile = 0x60, .width = 9, .wide = false },   // U+0030 (0)
    { .base_tile = 0x61, .width = 9, .wide = false },   // U+0031 (1)
    { .base_tile = 0x62, .width = 9, .wide = false },   // U+0032 (2)
    { .base_tile = 0x63, .width = 9, .wide = false },   // U+0033 (3)
    { .base_tile = 0x64, .width = 10, .wide = true },   // U+0034 (4)
    { .base_tile = 0x66, .width = 9, .wide = false },   // U+0035 (5)
    { .base_tile = 0x67, .width = 10, .wide = true },   // U+0036 (6)
    { .base_tile = 0x69, .width = 9, .wide = false },   // U+0037 (7)
    { .base_tile = 0x6a, .width = 9, .wide = false },   // U+0038 (8)
    { .base_tile = 0x6b, .width = 9, .wide = false },   // U+0039 (9)
    { .base_tile = 0x4a, .width = 4, .wide = false },   // U+003A (:)
    { 0 },    // U+003B (;)
    { 0 },    // U+003C (<)
    { 0 },    // U+003D (=)
    { 0 },    // U+003E (>)
    { .base_tile = 0x4b, .width = 9, .wide = false },   // U+003F (?)
    { .base_tile = 0x4c, .width = 11, .wide = true },   // U+0040 (@)
    { .base_tile = 0x00, .width = 9, .wide = false },   // U+0041 (A)
    { .base_tile = 0x01, .width = 9, .wide = false },   // U+0042 (B)
    { .base_tile = 0x02, .width = 9, .wide = false },   // U+0043 (C)
    { .base_tile = 0x03, .width = 10, .wide = true },   // U+0044 (D)
    { .base_tile = 0x05, .width = 9, .wide = false },   // U+0045 (E)
    { .base_tile = 0x06, .width = 9, .wide = false },   // U+0046 (F)
    { .base_tile = 0x07, .width = 9, .wide = false },   // U+0047 (G)
    { .base_tile = 0x08, .width = 9, .wide = false },   // U+0048 (H)
    { .base_tile = 0x09, .width = 9, .wide = false },   // U+0049 (I)
    { .base_tile = 0x0a, .width = 10, .wide = true },   // U+004A (J)
    { .base_tile = 0x0c, .width = 9, .wide = false },   // U+004B (K)
    { .base_tile = 0x0d, .width = 9, .wide = false },   // U+004C (L)
    { .base_tile = 0x0e, .width = 11, .wide = true },   // U+004D (M)
    { .base_tile = 0x20, .width = 10, .wide = true },   // U+004E (N)
    { .base_tile = 0x22, .width = 9, .wide = false },   // U+004F (O)
    { .base_tile = 0x23, .width = 9, .wide = false },   // U+0050 (P)
    { .base_tile = 0x24, .width = 10, .wide = true },   // U+0051 (Q)
    { .base_tile = 0x26, .width = 10, .wide = true },   // U+0052 (R)
    { .base_tile = 0x28, .width = 9, .wide = false },   // U+0053 (S)
    { .base_tile = 0x29, .width = 9, .wide = false },   // U+0054 (T)
    { .base_tile = 0x2a, .width = 9, .wide = false },   // U+0055 (U)
    { .base_tile = 0x2b, .width = 9, .wide = false },   // U+0056 (V)
    { .base_tile = 0x2c, .width = 12, .wide = true },   // U+0057 (W)
    { .base_tile = 0x2e, .width = 10, .wide = true },   // U+0058 (X)
    { .base_tile = 0x40, .width = 9, .wide = false },   // U+0059 (Y)
    { .base_tile = 0x41, .width = 9, .wide = false },   // U+005A (Z)
    { 0 },    // U+005B ([)
    { 0 },    // U+005C (\)
    { 0 },    // U+005D (])
    { 0 },    // U+005E (^)
    { 0 },    // U+005F (_)
    { 0 },    // U+0060 (`)
    { 0 },    // U+0061 (a)
    { 0 },    // U+0062 (b)
    { 0 },    // U+0063 (c)
    { 0 },    // U+0064 (d)
    { 0 },    // U+0065 (e)
    { 0 },    // U+0066 (f)
    { 0 },    // U+0067 (g)
    { 0 },    // U+0068 (h)
    { 0 },    // U+0069 (i)
    { 0 },    // U+006A (j)
    { 0 },    // U+006B (k)
    { 0 },    // U+006C (l)
    { 0 },    // U+006D (m)
    { 0 },    // U+006E (n)
    { 0 },    // U+006F (o)
    { 0 },    // U+0070 (p)
    { 0 },    // U+0071 (q)
    { 0 },    // U+0072 (r)
    { 0 },    // U+0073 (s)
    { 0 },    // U+0074 (t)
    { 0 },    // U+0075 (u)
    { 0 },    // U+0076 (v)
    { 0 },    // U+0077 (w)
    { .base_tile = 0x49, .width = 9, .wide = false },   // U+0078 (x)
    { 0 },   // U+0079 (y)
    { 0 },   // U+007A (z)
    { 0 },   // U+007B ({)
    { 0 },   // U+007C (|)
    { 0 },   // U+007D (})
    { 0 },   // U+007E (~)
    { 0 }    // U+007F
};

static const CharAttributes *char_attributes(char c) {
    return &attributes[(uint8_t)c];
}

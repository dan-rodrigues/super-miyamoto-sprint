#include "hero_life_meter.h"

#include <stdbool.h>

#include "vdp.h"

#include "hero.h"
#include "sprite_buffer.h"
#include "sprite_text.h"
#include "sound_effects.h"

static void draw_heart(uint32_t x, uint32_t y, bool filled);

void hero_increase_hit_point_max(Hero *hero) {
    hero->max_life++;
    hero->life = hero->max_life;
    se_powerup();
}

void hero_draw_life_meter(const Hero *hero) {
    const uint16_t x = 16;
    const uint16_t y = 48;

    for (uint32_t i = 0; i < hero->max_life; i++) {
        draw_heart(x, y + i * 16, (hero->life > i));
    }
}

void hero_draw_draw_coin_counter(const Hero *hero) {
    const uint16_t x = 48;
    const uint16_t y = 48;
    const uint16_t tile = 0x00e;

    // Coin graphic

    uint32_t x_block = x;

    uint32_t y_block = y;
    y_block |= SPRITE_16_TALL | SPRITE_16_WIDE;

    uint32_t g_block = tile;
    g_block |= 6 << SPRITE_PAL_SHIFT;
    g_block |= 3 << SPRITE_PRIORITY_SHIFT;

    sb_write(x_block, y_block, g_block);

    // Digits

    st_write("x", x + 16, y + 6);
    st_write_decimal(hero->coins, x + 24, y + 6);
}

static void draw_heart(uint32_t x, uint32_t y, bool filled) {
    const uint16_t tile = filled ? 0x08e : 0x08c;
    const uint8_t palette = 10;

    uint32_t x_block = x;

    uint32_t y_block = y;
    y_block |= SPRITE_16_TALL | SPRITE_16_WIDE;

    uint32_t g_block = tile;
    g_block |= palette << SPRITE_PAL_SHIFT;
    g_block |= 3 << SPRITE_PRIORITY_SHIFT;

    sb_write_priority(x_block, y_block, g_block);
}

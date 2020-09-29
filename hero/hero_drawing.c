#include <stddef.h>

#include "hero_drawing.h"

#include "hero.h"

#include "vdp.h"
#include "assert.h"

#include "miyamoto_tiles.h"
#include "vram_command_queue.h"
#include "vram_layout.h"

void hero_draw(Hero *hero, int16_t sprite_tile, Camera *camera) {
    if (!hero->visible) {
        return;
    }
    
    static const HeroTileOffset sprint_frame_offsets[] = {
        { .x = 0, .y = 0 },
        { .x = 0, .y = -16 },
        { .x = 16, .y = 0 }
    };

    static const HeroTileOffset kick_frame_offsets[] = {
        { .x = 0, .y = 0 },
        { .x = 0, .y = -16 },
        { .x = -16, .y = 0 }
    };

    static const HeroTileOffset standard_offsets[] = {
        { .x = 0, .y = 0 },
        { .x = 0, .y = -16 }
    };

    static const HeroTileOffset driving_offsets[] = {
        { .x = 4, .y = -2 },
        { .x = 4, .y = -18 }
    };

    static const HeroTileOffset carry_offsets[] = {
        { .x = 1, .y = 0 },
        { .x = 0, .y = -16 }
    };

    static const HeroFrameLayout hero_frames[] = {
        {RUN0, 2, 0, {0x000, 0x0e0, -1}, standard_offsets, {0, -1}},
        {RUN1, 2, 0, {0x002, 0x0e0, -1}, standard_offsets},
        {RUN2, 2, 0, {0x004, 0x0e0, -1}, standard_offsets, {0, 0}},

        {SPRINT0, 2, 0, {0x028, 0x0e0, 0x2c2}, sprint_frame_offsets, {0, -1}},
        {SPRINT1, 2, 0, {0x026, 0x0e0, 0x2c2}, sprint_frame_offsets},
        {SPRINT2, 2, 0, {0x024, 0x0e0, 0x2c2}, sprint_frame_offsets, {0, 0}},

        {CARRY0, 2, 0, {0x04c, 0x0e0, -1}, carry_offsets, {0, -1}},
        {CARRY1, 2, 0, {0x04e, 0x0e0, -1}, carry_offsets},
        {CARRY2, 2, 0, {0x060, 0x0e0, -1}, carry_offsets, {0, 0}},

        {DRIVING, 6, 0, {0x004, 0x0e0, -1}, driving_offsets, {0, 0}},

        {KICK, 2, 0, {0x062, 0x0e0, 0x2c6}, kick_frame_offsets, {0, 0}},

        {CLIMBING0, 0, 0, {0x06c, 0x08c, -1}, standard_offsets, {0, 0}},
        {CLIMBING1, 0, 1, {0x06c, 0x08c, -1}, standard_offsets, {0, 0}},

        {RUN_TURNING, 2, 0, {0x008, 0x2c4, -1}, standard_offsets, {0, 0}},

        {JUMP_RISING, 2, 0, {0x006, 0x0e8, -1}, standard_offsets, {0, 0}},
        {JUMP_FALLING, 2, 0, {0x1a4, 0x18e, -1}, standard_offsets, {0, 0}},

        {JUMP_SPRINTING, 2, 0, {0x02a, 0x18e, 0x2c0}, sprint_frame_offsets, {0, 0}},

        {LOOKING_UP, 0, 0, {0x004, 0x140, -1}, standard_offsets, {0, 0}},

        {LOOKING_SCREEN_DIRECT, 0, 0, {0x00e, 0x108, -1}, standard_offsets, {0, 0}},
        {LOOKING_SCREEN_LEFT, 0, 0, {0x00e, 0x0e0, -1}, standard_offsets, {0, 0}},
        {LOOKING_SCREEN_RIGHT,0, 1, {0x00e, 0x0e0, -1}, standard_offsets, {0, 0}},

        {PEACE_SIGN, 0, 0, {0x16e, 0x166, -1}, standard_offsets, {0, 0}},

        {DUCKING, 5, 0, {0x00c, -1}, standard_offsets, {0, 0}},
    };

    static const size_t hero_frame_count = sizeof(hero_frames) / sizeof(HeroFrameLayout);

    const uint8_t hero_palette = 8;
    const uint8_t hero_priority = 3;

    // This can be removed when the dictionary is replaced with a table for fast lookup

    const HeroFrameLayout *sprite_frame = NULL;
    for (uint16_t i = 0; i < hero_frame_count; i++) {
        if (hero_frames[i].frame == hero->frame) {
            sprite_frame = &hero_frames[i];
            break;
        }
    }

    assert(sprite_frame);

    const int16_t x_offset = -11;
    const int16_t y_offset = -17;

    int32_t sprite_y = hero->position.y + y_offset - camera->scroll.y;
    int32_t tile_y = sprite_y;

    const HeroTileOffset *tile_offsets = sprite_frame->tile_offsets;

    bool hero_needs_flip = (hero->direction == RIGHT);
    bool frame_needs_flip = sprite_frame->x_flip;
    bool x_flip = hero_needs_flip ^ frame_needs_flip;
    int8_t x_flip_offset = x_flip ? sprite_frame->x_flip_offset : 0;

    int32_t sprite_x = hero->position.x + sprite_frame->offset.x + x_flip_offset + x_offset;
    sprite_x -= camera->scroll.x;

    int32_t tile_x = sprite_x;

    bool priority_drawing = hero_has_draw_priority(hero);

    for (uint32_t i = 0; i < HERO_FRAME_MAX_TILES; i++) {
        int16_t frame_tile = sprite_frame->tiles[i];
        if (frame_tile == HERO_FRAME_TILES_END) {
            break;
        }

        // Per-tile offset

        tile_x = sprite_x + (x_flip ? -tile_offsets[i].x : tile_offsets[i].x);
        tile_y = sprite_y + tile_offsets[i].y;

        // Sprite metadata

        uint32_t x_block = tile_x;
        x_block &= 0x3ff;
        x_block |= (x_flip ? SPRITE_X_FLIP : 0);

        uint32_t y_block = tile_y + sprite_frame->offset.y;
        y_block &= 0x1ff;
        y_block |= SPRITE_16_TALL | SPRITE_16_WIDE;

        uint32_t g_block = sprite_tile;
        g_block |= hero_priority << SPRITE_PRIORITY_SHIFT | hero_palette << SPRITE_PAL_SHIFT;

        // Queue 16x16 block of graphics to upload..
        vcq_add_16x16(SPRITE_TILE_BASE, sprite_tile, frame_tile, miyamoto_tiles);
        // ..and point a sprite tile to this block
        if (priority_drawing) {
            sb_write_priority(x_block, y_block, g_block);
        } else {
            sb_write(x_block, y_block, g_block);
        }

        sprite_tile += 2;
    }
}

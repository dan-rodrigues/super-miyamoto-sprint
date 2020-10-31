#include <stddef.h>

#include "hero_drawing.h"

#include "hero.h"

#include "vdp.h"
#include "assert.h"

#include "miyamoto_tiles.h"
#include "vram_command_queue.h"
#include "vram_layout.h"
#include "palette_buffer.h"
#include "hero_frames.h"
#include "sprite_drawing.h"

void hero_draw(Hero *hero, int16_t sprite_tile, const Camera *camera) {
    bool should_update_palette = (
        hero->transluscent != hero->uploaded_transluscent_palette &&
        !hero->goal_reached
    );

    if (should_update_palette) {
        uint8_t alpha = (hero->transluscent ? 0x8 : 0xf);
        pb_alpha_mask_palette(8, alpha, false);

        hero->uploaded_transluscent_palette = hero->transluscent;
    }

    if (!hero->visible) {
        return;
    }

    HeroFrameLayout layout;
    hero_frame_layout(hero->frame, &layout);

    const uint8_t hero_palette = 8;
    const uint8_t hero_priority = 3;

    const int16_t x_offset = -9;
    const int16_t y_offset = -16;

    int32_t sprite_y = hero->position.y + y_offset - camera->scroll.y;
    int32_t tile_y = sprite_y;

    const HeroTileOffset *tile_offsets = layout.tile_offsets;

    bool hero_needs_flip = (hero->direction == RIGHT);
    bool frame_needs_flip = layout.x_flip;
    bool x_flip = hero_needs_flip ^ frame_needs_flip;
    int8_t x_flip_offset = (x_flip ? layout.x_flip_offset : 0);

    int32_t sprite_x = hero->position.x + layout.offset.x + x_flip_offset + x_offset;
    sprite_x -= camera->scroll.x;

    int32_t tile_x = sprite_x;

    bool priority_drawing = hero_has_draw_priority(hero);

    bool needs_frame_upload = (hero->frame != hero->uploaded_frame);
    hero->uploaded_frame = hero->frame;

    for (uint32_t i = 0; i < HERO_FRAME_MAX_TILES; i++) {
        int16_t frame_tile = layout.tiles[i];
        if (frame_tile == HERO_FRAME_TILES_END) {
            break;
        }

        // Per-tile offset

        tile_x = sprite_x + (x_flip ? -tile_offsets[i].x : tile_offsets[i].x);
        tile_y = sprite_y + tile_offsets[i].y;

        // Sprite metadata

        int32_t x_block = tile_x;
        int32_t y_block = tile_y + layout.offset.y;

        if (!sa_draw_needed(tile_x, tile_y, 32)) {
            continue;
        }

        x_block &= 0x3ff;
        x_block |= (x_flip ? SPRITE_X_FLIP : 0);

        y_block &= 0x1ff;
        y_block |= SPRITE_16_TALL | SPRITE_16_WIDE;

        uint32_t g_block = sprite_tile;
        g_block |= hero_priority << SPRITE_PRIORITY_SHIFT | hero_palette << SPRITE_PAL_SHIFT;

        if (needs_frame_upload) {
            // Queue 16x16 block of graphics to upload..
            vcq_add_16x16(SPRITE_TILE_BASE, sprite_tile, frame_tile, miyamoto_tiles);
        }

        // ..and point a sprite tile to this block
        if (priority_drawing) {
            sb_write_priority(x_block, y_block, g_block);
        } else {
            sb_write(x_block, y_block, g_block);
        }

        sprite_tile += 2;
    }
}

void hero_16x16_arrangements(HeroFrame frame, HeroTileArrangement *arrangements, size_t limit) {
    HeroFrameLayout layout;
    hero_frame_layout(frame, &layout);

    for (uint32_t i = 0; i < HERO_FRAME_MAX_TILES; i++) {
        int16_t frame_tile = layout.tiles[i];
        if ((frame_tile == HERO_FRAME_TILES_END) || (i >= limit)) {
            break;
        }

        HeroTileArrangement arrangement = {
            .x = layout.offset.x + layout.tile_offsets[i].x,
            .y = layout.offset.y + layout.tile_offsets[i].y,
            .top_row = miyamoto_tiles + layout.tiles[i] * 0x10,
            .bottom_row = miyamoto_tiles + layout.tiles[i] * 0x10 + 0x100,
        };

        arrangements[i] = arrangement;
    }
}

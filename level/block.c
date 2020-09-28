#include "block.h"

#include <stddef.h>

#include "math_util.h"
#include "vram_command_queue.h"
#include "vram_layout.h"
#include "block_map_table.h"
#include "vram_address.h"
#include "camera.h"
#include "vdp.h"
#include "assert.h"

#include "debug_print.h"
#include "block_attributes.h"

// Stride is configurable but it must be within this total limit
#define LEVEL_BLOCKS_MAX 8192

static const uint16_t BLOCK_ID_CLEAR = 0x025;

static uint16_t current_stride;
// Temporarily fixing height
static const uint16_t current_height = 0x1e0 / 0x10;

static uint16_t current_level[LEVEL_BLOCKS_MAX];

static uint32_t index_at(uint32_t x, uint32_t y);
static bool in_bounds(uint32_t x, uint32_t y);

void block_load_level(const uint16_t *data, uint16_t stride) {
    current_stride = stride;
//  current_height = (from level attributes..)

    for (uint32_t i = 0; i < LEVEL_BLOCKS_MAX; i++) {
        current_level[i] = data[i];
    }
}

uint16_t block_lookup(uint32_t x, uint32_t y) {
    if (!in_bounds(x, y)) {
        return BLOCK_ID_CLEAR;
    }

    return current_level[index_at(x, y)];
}

uint16_t block_lookup_pixel(uint32_t x, uint32_t y) {
    return block_lookup(x / 16, y / 16);
}

void erase_block(uint32_t x, uint32_t y, const Camera *camera) {
    // Fix pixel coordinates to 16x16 block coordinatess
    x &= ~0xf;
    y &= ~0xf;
    x /= 0x10;
    y /= 0x10;

    assert(in_bounds(x, y));

    // 1. Erase the block in the level data
    current_level[index_at(x, y)] = BLOCK_ID_CLEAR;

    // 2. Erase the block from screen *if* it happens to be in view
    // This should be permissive since there's a lot of buffer space offscreen

    const int32_t bounds_margin = 32;

    bool block_visible = camera_point_visible(x * 16 + bounds_margin, 0, camera);
    block_visible |= camera_point_visible(x * 16 + bounds_margin + 8 + SCREEN_ACTIVE_WIDTH, 0, camera);

    if (!block_visible) {
        return;
    }

    // Note the words for clear block could just be set as constants here...
    // This is slower but more flexible incase the block def does actually change
    uint32_t index = BLOCK_ID_CLEAR * 4;

    vcq_add_2x2_map(vram_address(SCROLL_MAP_BASE, x * 2, y * 2),
                    block_map_table[index + 0],
                    block_map_table[index + 2],
                    block_map_table[index + 1],
                    block_map_table[index + 3]);
}

typedef struct {
    uint16_t block;
    BlockInteractionAttributes attributes;
} BlockInteractionAttributeTuple;

BlockInteractionAttributes block_get_attributes(uint16_t block) {
    return block_attributes[block];
}

static uint32_t index_at(uint32_t x, uint32_t y) {
    return sys_multiply(y, current_stride) + x;
}

static bool in_bounds(uint32_t x, uint32_t y) {
    return (y < current_height) && (x < current_stride);
}

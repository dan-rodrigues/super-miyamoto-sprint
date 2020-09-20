#include "block.h"

#include <stddef.h>

#include "math_util.h"
#include "vram_command_queue.h"
#include "vram_layout.h"
#include "block_map_table.h"
#include "vram_address.h"
#include "camera.h"
#include "vdp.h"

#include "debug_print.h"

// Stride is configurable but it must be within this total limit
#define LEVEL_BLOCKS_MAX 8192

static uint16_t current_stride;
static uint16_t current_level[LEVEL_BLOCKS_MAX];

static uint32_t index_at(uint32_t x, uint32_t y);

void block_load_level(const uint16_t *data, uint16_t stride) {
    current_stride = stride;

    for (uint32_t i = 0; i < LEVEL_BLOCKS_MAX; i++) {
        current_level[i] = data[i];
    }
}

uint16_t block_lookup(uint32_t x, uint32_t y) {
    return current_level[index_at(x, y)];
}

uint16_t block_lookup_pixel(uint32_t x, uint32_t y) {
    return block_lookup(x / 16, y / 16);
}

void erase_block(uint32_t x, uint32_t y, const Camera *camera) {
    // Fix pixel coordinates to 8x8 block coordinatess
    x &= ~0xf;
    y &= ~0xf;
    x /= 0x10;
    y /= 0x10;

    // 1. Erase the block in the level data
    const uint16_t clear_block = 0x025;
    current_level[index_at(x, y)] = clear_block;

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
    uint32_t index = clear_block * 4;

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

// To be replaced with a flash based large table instead of this iterative lookup
// This makes it easy to add and modify things as progress is made

static const BlockInteractionAttributeTuple block_interaction_attribute_map[] = {
    {.block = 0x025, .attributes = BLOCK_EMPTY},

    // Foreground
    {.block = 0x100, .attributes = BLOCK_CAN_STAND},
    {.block = 0x101, .attributes = BLOCK_CAN_STAND},
    {.block = 0x103, .attributes = BLOCK_CAN_STAND},
    {.block = 0x104, .attributes = BLOCK_CAN_STAND},
    {.block = 0x106, .attributes = BLOCK_CAN_STAND},

    // Foregound - solid
    {.block = 0x145, .attributes = BLOCK_CAN_STAND | BLOCK_SOLID_BOTTOM | BLOCK_SOLID_SIDE},
    {.block = 0x148, .attributes = BLOCK_CAN_STAND | BLOCK_SOLID_BOTTOM | BLOCK_SOLID_SIDE},
    {.block = 0x14b, .attributes = BLOCK_CAN_STAND | BLOCK_SOLID_BOTTOM | BLOCK_SOLID_SIDE},
    {.block = 0x14c, .attributes = BLOCK_CAN_STAND | BLOCK_SOLID_BOTTOM | BLOCK_SOLID_SIDE},
    {.block = 0x1e2, .attributes = BLOCK_CAN_STAND | BLOCK_SOLID_BOTTOM | BLOCK_SOLID_SIDE},

    // Vertical ledge blocks
    {.block = 0x145, .attributes = BLOCK_CAN_STAND | BLOCK_SOLID_BOTTOM | BLOCK_SOLID_SIDE},
    {.block = 0x1e2, .attributes = BLOCK_CAN_STAND | BLOCK_SOLID_BOTTOM | BLOCK_SOLID_SIDE},

    // Solid blocks
    {.block = 0x124, .attributes = BLOCK_CAN_STAND | BLOCK_SOLID_BOTTOM | BLOCK_SOLID_SIDE},

    // Pillars
    {.block = 0x107, .attributes = BLOCK_CAN_STAND},
    {.block = 0x109, .attributes = BLOCK_CAN_STAND},
    {.block = 0x108, .attributes = BLOCK_CAN_STAND},
    {.block = 0x10a, .attributes = BLOCK_CAN_STAND},

    // Cement
    {.block = 0x130, .attributes = BLOCK_CAN_STAND | BLOCK_SOLID_BOTTOM | BLOCK_SOLID_SIDE},
    // Brown
    {.block = 0x132, .attributes = BLOCK_CAN_STAND | BLOCK_SOLID_BOTTOM | BLOCK_SOLID_SIDE},
    // Munching plant
    {.block = 0x12f, .attributes = BLOCK_CAN_STAND | BLOCK_SOLID_BOTTOM | BLOCK_SOLID_SIDE | BLOCK_DAMAGE},

    // Coins
    {.block = 0x02b, .attributes = BLOCK_COIN},

    {.block = 0x006, .attributes = BLOCK_CLIMB}
};

static const size_t block_attribute_map_size = sizeof(block_interaction_attribute_map) / sizeof(BlockInteractionAttributeTuple);

BlockInteractionAttributes block_get_attributes(uint16_t block) {
    for (uint32_t i = 0; i < block_attribute_map_size; i++) {
        if (block_interaction_attribute_map[i].block == block) {
            return block_interaction_attribute_map[i].attributes;
        }
    }

    return BLOCK_EMPTY;
}

static uint32_t index_at(uint32_t x, uint32_t y) {
    return sys_multiply(y, current_stride) + x;
}

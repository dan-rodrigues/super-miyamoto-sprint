#ifndef block_h
#define block_h

#include <stdint.h>

#include "camera_types.h"

typedef enum {
    BLOCK_CAN_STAND = (1 << 0),
    BLOCK_SOLID_BOTTOM = (1 << 1),
    BLOCK_SOLID_SIDE = (1 << 2),

    BLOCK_EMPTY = (1 << 3),
    BLOCK_DAMAGE = (1 << 4),
    BLOCK_CLIMB = (1 << 5),
    BLOCK_COIN = (1 << 6)
    // ...
} BlockInteractionAttributes;

void block_load_level(const uint16_t *data, uint16_t stride);

uint16_t block_lookup(uint32_t x, uint32_t y);
uint16_t block_lookup_pixel(uint32_t x, uint32_t y);
void erase_block(uint32_t x, uint32_t y, const Camera *camera);

BlockInteractionAttributes block_get_attributes(uint16_t block);

#endif /* block_h */

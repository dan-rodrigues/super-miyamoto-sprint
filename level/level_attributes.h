#ifndef level_attributes_h
#define level_attributes_h

#include <stdint.h>
#include <stddef.h>

typedef struct {
    int16_t x, y;
} LevelPosition;

typedef struct {
    LevelPosition start_position;

    uint16_t stride;
    const uint16_t *block_data;

    const void *sprite_data;
    const size_t *sprite_data_length;
} LevelAttributes;

extern const size_t level_count;
const LevelAttributes *level_attributes(uint32_t level);

#endif /* level_attributes_h */

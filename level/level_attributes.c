#include "level_attributes.h"

#include "assert.h"

#include "level1.h"
#include "sprites1.h"

#include "level2.h"
#include "sprites2.h"

const size_t LEVEL_COUNT = 2;

const LevelAttributes *level_attributes(uint32_t level_id) {
    static const LevelAttributes levels[] = {
        {
            .start_position = { 16, 384 },
            .mid_position = { 2416, 340 },

            .block_data = level1,
            .stride = 0x10 * 18,

            .sprite_data = sprites1,
            .sprite_data_length = &sprites1_length
        },
        {
            .start_position = { 16, 192 },
            .mid_position = { 2200, 304 },

            .block_data = level2,
            .stride = 0x10 * 18,

            .sprite_data = sprites2,
            .sprite_data_length = &sprites2_length
        }
    };

    assert(level_id < LEVEL_COUNT);

    return &levels[level_id];
}

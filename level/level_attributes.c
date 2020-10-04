#include "level_attributes.h"

// Only 1 level for now
#include "level.h"
#include "sprites.h"

const size_t level_count = 1;

const LevelAttributes *level_attributes(uint32_t level_id) {
    static const LevelAttributes levels[] = {
        {
            .start_position = { 16, 384 },
//            .start_position = { 2416, 340 - 64 },

//            .mid_position = { 2416, 340 - 64 },
            .mid_position = { 2416, 340 },

            .block_data = level,
            .stride = 0x10 * 18,

            .sprite_data = sprites,
            .sprite_data_length = &sprites_length
        }
    };

    return &levels[level_id];
}

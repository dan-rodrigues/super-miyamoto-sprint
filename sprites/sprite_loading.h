#ifndef sprite_loading_h
#define sprite_loading_h

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "camera_types.h"
#include "hero_types.h"
#include "sprite_loading_types.h"

// The total number of allowed sprites in any one level
#define SPRITE_INDEX_MAX 127

extern const int8_t SPRITE_INDEX_UNDEFINED;

// The model as it is stored in flash, without any extra padding:
struct SpriteLevelEncoded {
    int16_t x, y;
    uint8_t type;

    uint8_t padding;
} __attribute__((packed));

struct SpriteLoadingBounds {
    int32_t left, right;
};

struct SpriteLoadingContext {
    const SpriteLevelEncoded *sprite_data_base;
    const SpriteLevelEncoded *sprite_data_end;
    uint8_t total_sprite_count;

    bool loadable_sprite_indexes[SPRITE_INDEX_MAX];

    int8_t next_left_index;
    int8_t next_right_index;

    uint16_t previous_left_scroll;
    uint16_t previous_right_scroll;
};

void sprite_level_data_load_new(SpriteLoadingContext *context, const Camera *camera, const Hero *hero);
void sprite_level_data_prevent_index_reloading(SpriteLoadingContext *context, int8_t level_index);

#endif /* sprite_loading_h */

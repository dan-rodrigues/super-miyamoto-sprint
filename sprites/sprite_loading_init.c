#include "sprite_loading_init.h"

#include "assert.h"
#include "vdp.h"
#include "math_util.h"

#include "camera.h"
#include "hero.h"
#include "gcc_lib.h"
#include "sprite_loading.h"

void sprite_level_data_init(SpriteLoadingContext *context, const void *sprite_data, size_t sprite_data_length) {
    // Assumes offset (0, 0) in the level
    context->sprite_data_base = sprite_data;

    size_t count = sprite_data_length / sizeof(SpriteLevelEncoded);
    context->sprite_data_end = context->sprite_data_base + count;
    context->total_sprite_count = count;

    memset(context->loadable_sprite_indexes, true, sizeof(context->loadable_sprite_indexes));
}

// Load all sprites that are currently visible on screen

void sprite_level_data_perform_initial_load(SpriteLoadingContext *context, const Camera *camera, const Hero *hero) {
    assert(context->sprite_data_base);
    assert(context->sprite_data_end);

    // Seek to sprite data starting from the center of the screen

    int32_t center = camera->scroll.x + SCREEN_ACTIVE_WIDTH / 2;

    int32_t center_index = MAX(0, context->total_sprite_count - 1);
    for (uint32_t i = 0; i < context->total_sprite_count; i++) {
        if (context->sprite_data_base[i].x >= center) {
            center_index = i;
            break;
        }
    }

    int32_t left_index = center_index;
    int32_t right_index = MIN(context->total_sprite_count - 1, left_index + 1);
    right_index = MAX(0, right_index);

    context->next_left_index = left_index;
    context->next_right_index = right_index;

    // Then load initial left / right half of screen sprites

    context->previous_left_scroll = center;
    context->previous_right_scroll = center;
    sprite_level_data_load_new(context, camera, hero);
}

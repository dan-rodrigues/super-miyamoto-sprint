#include "sprite_loading.h"

#include <stdint.h>

#include "vdp.h"
#include "assert.h"

#include "math_util.h"
#include "debug_print.h"
#include "camera.h"
#include "hero.h"
#include "sprite_actor.h"
#include "gcc_lib.h"

const int8_t SPRITE_INDEX_UNDEFINED = -1;

static void init_sprite(SpriteLoadingContext *context, const SpriteLevelEncoded *sprite, const Hero *hero, int8_t index);
static SpriteActor *sprite_factory(const SpriteLevelEncoded *sprite, const SpritePosition *position);

static void update_left_scroll(SpriteLoadingContext *context, int32_t new_scroll, const Hero *hero);
static void update_right_scroll(SpriteLoadingContext *context, int32_t new_scroll, const Hero *hero);

static void loading_bounds(const Camera *camera, SpriteLoadingBounds *bounds) {
    const int32_t padding = 48;

    bounds->left = camera->scroll.x - padding;
    bounds->right = camera->scroll.x + SCREEN_ACTIVE_WIDTH + padding;
}

static void update_left_scroll(SpriteLoadingContext *context, int32_t new_scroll, const Hero *hero) {
    if (context->previous_left_scroll == new_scroll) {
        return;
    }

    int8_t *next_left_index = &context->next_left_index;
    uint8_t total_sprite_count = context->total_sprite_count;

    bool moved_left = new_scroll < context->previous_left_scroll;
    if (moved_left) {
        while (next_left_index >= 0) {
            SpriteLevelEncoded next_left_sprite = context->sprite_data_base[*next_left_index];
            if (next_left_sprite.x <= new_scroll) {
                break;
            }

            init_sprite(context, &next_left_sprite, hero, *next_left_index);
            (*next_left_index)--;
        }
    } else {
        while (context->sprite_data_base[++(*next_left_index)].x <= new_scroll) {
            if (*next_left_index == total_sprite_count) {
                break;
            }
        }

        (*next_left_index)--;
    }

    context->previous_left_scroll = new_scroll;
}

static void update_right_scroll(SpriteLoadingContext *context, int32_t new_scroll, const Hero *hero) {
    if (context->previous_right_scroll == new_scroll) {
        return;
    }

    int8_t *next_right_index = &context->next_right_index;
    uint8_t total_sprite_count = context->total_sprite_count;

    bool moved_right = new_scroll > context->previous_right_scroll;
    if (moved_right) {
        while (*next_right_index < total_sprite_count) {
            SpriteLevelEncoded next_right_sprite = context->sprite_data_base[*next_right_index];
            if (next_right_sprite.x > new_scroll) {
                break;
            }

            init_sprite(context, &next_right_sprite, hero, *next_right_index);
            (*next_right_index)++;
        }
    } else {
        while (context->sprite_data_base[--(*next_right_index)].x > new_scroll) {
            if (*next_right_index < 0) {
                break;
            }
        }
        (*next_right_index)++;
    }

    context->previous_right_scroll = new_scroll;
}

// Load sprites that have just come into bounds of the camera
// They should be slightly offscreen when they are loaded
// This assumes the sprites are not culled within this region or they'll never appear

void sprite_level_data_load_new(SpriteLoadingContext *context, const Camera *camera, const Hero *hero) {
    SpriteLoadingBounds bounds;
    loading_bounds(camera, &bounds);

    update_left_scroll(context, bounds.left, hero);
    update_right_scroll(context, bounds.right, hero);

    // The indexes are allowed to go 1 beyond their limit
    assert(context->next_left_index >= -1);
    assert(context->next_right_index <= SPRITE_INDEX_MAX);
    assert(context->next_right_index <= context->total_sprite_count);
}

static void init_sprite(SpriteLoadingContext *context,
                        const SpriteLevelEncoded *sprite,
                        const Hero *hero,
                        int8_t index)
{
    // Don't load this sprite if any active one was already loaded at this index
    if (sa_level_index_live(index)) {
        return;
    }

    // Don't load this sprite if it was already killed or otherwise marked as no-load
    if (!context->loadable_sprite_indexes[index]) {
        return;
    }

    SpritePosition position = {
        .x = sprite->x,
        .y = sprite->y,
        .x_fraction = 0,
        .y_fraction = 0
    };

    SpriteActor *actor = sprite_factory(sprite, &position);
    if (!actor) {
        return;
    }
    
    actor->level_data_index = index;

    // By default, face the hero
    // Eventually certain sprites may have their preferred way of doing this instead
    bool hero_on_left = (hero->position.x < position.x);
    actor->direction = hero_on_left ? LEFT : RIGHT;
}

typedef enum {
    SPRITE_ID_LAYERED = 0x04,
    SPRITE_ID_BASIC = 0x0f,
    SPRITE_ID_BASIC_SPIKED = 0x10,
    SPRITE_ID_BALL_CURLED = 0xda,
    SPRITE_ID_BALL = 0xdb,
    SPRITE_ID_PLATFORM_X = 0x5b,
    SPRITE_ID_PLATFORM_Y = 0xc4,
    SPRITE_ID_TANK = 0x1c,
    SPRITE_ID_JUMPER = 0x05,
    SPRITE_ID_JETPACK = 0x0a,
    SPRITE_ID_GOAL = 0x2c,
    SPRITE_ID_MIDPOINT = 0x7b,
    SPRITE_ID_UNDEFINED = 0xff
} SpriteID;

static SpriteActor *sprite_factory(const SpriteLevelEncoded *sprite, const SpritePosition *position) {
    switch ((SpriteID)sprite->type) {
        case SPRITE_ID_BASIC:
            return basic_enemy_sprite_init(position, false);
        case SPRITE_ID_BASIC_SPIKED:
            return basic_enemy_sprite_init(position, true);
        case SPRITE_ID_JETPACK:
            return jetpack_enemy_sprite_init(position);
        case SPRITE_ID_BALL_CURLED:
            return ball_enemy_sprite_init(position, false);
        case SPRITE_ID_BALL:
            return ball_enemy_sprite_init(position, true);
        case SPRITE_ID_JUMPER:
            return jumping_enemy_sprite_init(position);
        case SPRITE_ID_PLATFORM_X:
            return platform_sprite_init(position, PLATFORM_MOTION_AUTO_X);
        case SPRITE_ID_PLATFORM_Y:
            return platform_sprite_init(position, PLATFORM_MOTION_AUTO_Y);
        case SPRITE_ID_TANK:
            return tank_sprite_init(position, true);
        case SPRITE_ID_GOAL:
            return goal_sprite_init(position);
        case SPRITE_ID_MIDPOINT:
            return midpoint_sprite_init(position);
        case SPRITE_ID_LAYERED:
            return layered_enemy_sprite_init(position);
        case SPRITE_ID_UNDEFINED:
            return NULL;
        default:
            fatal_error("Unrecognized sprite ID");
    }
}

void sprite_level_data_prevent_index_reloading(SpriteLoadingContext *context, int8_t level_index) {
    if (level_index != SPRITE_INDEX_UNDEFINED) {
        context->loadable_sprite_indexes[level_index] = false;
    }
}

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

// The total number of allowed sprites in any one level
#define SPRITE_INDEX_MAX 127

const int8_t SPRITE_INDEX_UNDEFINED = -1;

// The model as it is stored in flash:
typedef struct {
    uint16_t x, y;
    uint8_t type;

    uint8_t padding;
} __attribute__((packed)) SpriteLevelEncoded;

static const SpriteLevelEncoded *sprite_data_base;
static const SpriteLevelEncoded *sprite_data_end;
static uint16_t total_sprite_count = 0;

// Can pack this into bit-array if needed
static bool loadable_sprite_indexes[SPRITE_INDEX_MAX];

static int32_t next_left_index = 0;
static int32_t next_right_index = 0;

static CameraScroll scroll_last_update = {0, 0};

static void init_sprite(const SpriteLevelEncoded *sprite, const Hero *hero, int8_t index);
static SpriteActor *sprite_factory(const SpriteLevelEncoded *sprite, const SpritePosition *position);

void sprite_level_data_init(const void *sprite_data, size_t sprite_data_length) {
    // Assumes offset (0, 0) in the level
    sprite_data_base = sprite_data;

    size_t count = sprite_data_length / sizeof(SpriteLevelEncoded);
    sprite_data_end = sprite_data_base + count;
    total_sprite_count = count;

    next_left_index = 0;
    next_right_index = count - 1;

    memset(loadable_sprite_indexes, true, sizeof(loadable_sprite_indexes));
}

// Load all sprites from the left span of the screen to the right
void sprite_level_data_perform_initial_load(const Camera *camera, const Hero *hero) {
    assert(sprite_data_base);
    assert(sprite_data_end);

    scroll_last_update.x = -1;
    scroll_last_update.y = -1;

    // Start loading from left of current screen position..
    sprite_level_data_load_new(camera, hero);

    // ..then sweep from left to right and load all sprites
    int32_t right = camera->scroll.x + SCREEN_ACTIVE_WIDTH;
    Camera dummy_camera = *camera;
    dummy_camera.scroll.x = right;
    sprite_level_data_load_new(&dummy_camera, hero);
}

// Load sprites that have just come into bounds of the camera
// They should be slightly offscreen when they are loaded
// This assumes the sprites are not culled within this region or they'll never appear

__attribute__((optimize("-fno-strict-aliasing")))
void sprite_level_data_load_new(const Camera *camera, const Hero *hero) {
    if (scroll_last_update.x == camera->scroll.x) {
        return;
    }

    bool moved_left = scroll_last_update.x > camera->scroll.x;

    const int32_t left_bound_margin = 32;
    int32_t left_bound = camera->scroll.x - left_bound_margin;
    int32_t right_bound = camera->scroll.x + SCREEN_ACTIVE_WIDTH;

    if (moved_left) {
        // Moving towards left

        // Is the next sprite to be loaded on screen?
        while (next_left_index >= 0) {
            SpriteLevelEncoded next_left_sprite = sprite_data_base[next_left_index];
            if (next_left_sprite.x <= left_bound) {
                // No more sprites to see on screen (yet)
                break;
            }

            init_sprite(&next_left_sprite, hero, next_left_index);
            next_left_index--;
        }

        // Fix next-right sprite as the right bound may have moved beyond it
        while (sprite_data_base[--next_right_index].x > right_bound) {}
        next_right_index++;
    } else {
        // Moving towards right

        // Is the next sprite to be loaded on screen?
        while (next_right_index < total_sprite_count) {
            SpriteLevelEncoded next_right_sprite = sprite_data_base[next_right_index];
            if (next_right_sprite.x > right_bound) {
                // No more sprites to see on screen (yet)
                break;
            }

            init_sprite(&next_right_sprite, hero, next_right_index);
            next_right_index++;
        }

        // Fix next-left sprite, as above
        while (sprite_data_base[++next_left_index].x <= left_bound) {}
        next_left_index--;
    }

    scroll_last_update = camera->scroll;

    // The indexes are allowed to go 1 beyond their limit
    assert(next_left_index >= -1);
    assert(next_right_index <= SPRITE_INDEX_MAX);
    assert(next_right_index <= total_sprite_count);
}

static void init_sprite(const SpriteLevelEncoded *sprite, const Hero *hero, int8_t index) {
    // Don't load this sprite if any active one was already loaded at this index
    if (sa_level_index_live(index)) {
        return;
    }

    // Don't load this sprite if it was already killed or otherwise marked as no-load
    if (!loadable_sprite_indexes[index]) {
        return;
    }

    SpritePosition position = {
        .x = sprite->x,
        .y = sprite->y,
        .x_fraction = 0,
        .y_fraction = 0
    };

    SpriteActor *actor = sprite_factory(sprite, &position);
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
    SPRITE_ID_PLATFORM_Y = 0xc4
} SpriteID;

static SpriteActor *sprite_factory(const SpriteLevelEncoded *sprite, const SpritePosition *position) {
    switch ((SpriteID)sprite->type) {
        case SPRITE_ID_BASIC:
            return basic_enemy_sprite_init(position, false);
        case SPRITE_ID_BASIC_SPIKED:
            return basic_enemy_sprite_init(position, true);
        case SPRITE_ID_BALL_CURLED:
            return ball_enemy_sprite_init(position, false);
        case SPRITE_ID_BALL:
            return ball_enemy_sprite_init(position, true);
        case SPRITE_ID_PLATFORM_X:
            return platform_sprite_init(position, PLATFORM_MOTION_AUTO_X);
        case SPRITE_ID_PLATFORM_Y:
            return platform_sprite_init(position, PLATFORM_MOTION_AUTO_Y);
        case SPRITE_ID_LAYERED: default:
            return layered_enemy_sprite_init(position);
    }
}

void sprite_level_data_prevent_index_reloading(int8_t level_index) {
    if (level_index != SPRITE_INDEX_UNDEFINED) {
        loadable_sprite_indexes[level_index] = false;
    }
}

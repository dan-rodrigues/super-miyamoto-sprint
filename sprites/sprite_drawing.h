#ifndef sprite_drawing_h
#define sprite_drawing_h

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "sprite_drawing_types.h"
#include "sprite_actor_types.h"
#include "sprite_position_types.h"

#define SPRITE_DRAW_PARAM_COUNT(x) (sizeof(x) / sizeof(SpriteDrawParams))

// Contrast with HeroFrameLayout

struct SpriteDrawParams {
    int8_t offset_x, offset_y;
    uint16_t tile;
    uint32_t palette : 4;

    bool wide : 1, tall : 1;
    bool x_flip : 1, y_flip : 1;
}  __attribute__((__packed__));

void sa_draw_standard_8x8(const SpriteActor *self, const SpriteEnvironment *env, uint16_t tile, uint8_t palette);
void sa_draw_standard_8x8_light(const SpriteActorLight *self, const SpriteEnvironment *env, uint16_t tile, uint8_t palette);
void sa_draw_standard_16x16_light(const SpriteActorLight *self, const SpriteEnvironment *env, uint16_t tile, uint8_t palette);

void sa_draw_standard_16x16(const SpriteActor *self, const SpriteEnvironment *env, uint16_t tile, uint8_t palette);

void sa_draw_standard(const SpriteActor *self, const SpriteEnvironment *env, const SpriteDrawParams *params);
void sa_draw_standard_light(const SpriteActorLight *self, const SpriteEnvironment *env, const SpriteDrawParams *params);

void sa_draw_standard_multiple(const SpriteActor *self, const SpriteEnvironment *env, const SpriteDrawParams *params, size_t count);
void sa_draw_standard_light_multiple(const SpriteActorLight *self, const SpriteEnvironment *env, const SpriteDrawParams *params, size_t count);

#endif /* sprite_drawing_h */

#include "sprite_drawing.h"

#include <stdbool.h>

#include "sprite_actor.h"
#include "camera.h"
#include "sprite_position.h"
#include "vdp.h"
#include "sprite_buffer.h"

static bool onscreen(int32_t screen_x, int32_t screen_y, int32_t padding);

static void sa_draw_standard_8x8_impl(const SpritePosition *position,
                                      SpriteDirection direction,
                                      const SpriteEnvironment *env,
                                      uint16_t tile,
                                      uint8_t palette,
                                      bool above_other_sprites);

static void sa_draw_standard_impl(const SpritePosition *position,
                                  SpriteDirection direction,
                                  const SpriteEnvironment *env,
                                  const SpriteDrawParams *params,
                                  bool above_other_sprites);

static void sa_draw_standard_multiple_impl(const SpritePosition *position,
                                           SpriteDirection direction,
                                           const SpriteEnvironment *env,
                                           const SpriteDrawParams *params,
                                           size_t count,
                                           bool above_other_sprites);

static void sa_draw_standard_16x16_impl(const SpritePosition *position,
                                        SpriteDirection direction,
                                        const SpriteEnvironment *env,
                                        uint16_t tile,
                                        uint8_t palette,
                                        bool above_other_sprites);

void sa_draw_standard_16x16(const SpriteActor *self, const SpriteEnvironment *env, uint16_t tile, uint8_t palette) {
    SpriteDrawParams params = {
        .offset_x = -8,
        .offset_y = -17,
        .palette = palette,
        .tile = tile,
        .wide = true, .tall = true,
        .x_flip = false, .y_flip = false
    };

    sa_draw_standard_impl(&self->position, self->direction, env, &params, false);
}

void sa_draw_standard_16x16_light(const SpriteActorLight *self, const SpriteEnvironment *env, uint16_t tile, uint8_t palette) {
    sa_draw_standard_16x16_impl(&self->position, self->direction, env, tile, palette, true);
}

void sa_draw_standard_8x8(const SpriteActor *self, const SpriteEnvironment *env, uint16_t tile, uint8_t palette) {
    sa_draw_standard_8x8_impl(&self->position, self->direction, env, tile, palette, false);
}

void sa_draw_standard_8x8_light(const SpriteActorLight *self, const SpriteEnvironment *env, uint16_t tile, uint8_t palette) {
    sa_draw_standard_8x8_impl(&self->position, self->direction, env, tile, palette, true);
}

void sa_draw_standard(const SpriteActor *self, const SpriteEnvironment *env, const SpriteDrawParams *params) {
    sa_draw_standard_impl(&self->position, self->direction, env, params, false);
}

void sa_draw_standard_light(const SpriteActorLight *self,
                            const SpriteEnvironment *env,
                            const SpriteDrawParams *params)
{
    sa_draw_standard_impl(&self->position, self->direction, env, params, true);
}

void sa_draw_standard_light_multiple(const SpriteActorLight *self,
                                     const SpriteEnvironment *env,
                                     const SpriteDrawParams *params,
                                     size_t count)
{
    sa_draw_standard_multiple_impl(&self->position, self->direction, env, params, count, true);
}

void sa_draw_standard_multiple(const SpriteActor *self,
                               const SpriteEnvironment *env,
                               const SpriteDrawParams *params,
                               size_t count)
{
    sa_draw_standard_multiple_impl(&self->position, self->direction, env, params, count, false);
}

static void sa_draw_standard_multiple_impl(const SpritePosition *position,
                                           SpriteDirection direction,
                                           const SpriteEnvironment *env,
                                           const SpriteDrawParams *params,
                                           size_t count,
                                           bool above_other_sprites)
{
    for (uint32_t i = 0; i < count ; i++) {
        sa_draw_standard_impl(position, direction, env, &params[i], above_other_sprites);
    }
}

static void sa_draw_standard_8x8_impl(const SpritePosition *position,
                                      SpriteDirection direction,
                                      const SpriteEnvironment *env,
                                      uint16_t tile,
                                      uint8_t palette,
                                      bool above_other_sprites)
{
    SpriteDrawParams params = {
        .offset_x = -4,
        .offset_y = -9,
        .palette = palette,
        .tile = tile,
        .wide = false, .tall = false,
        .x_flip = false, .y_flip = false
    };

    sa_draw_standard_impl(position, direction, env, &params, above_other_sprites);
}

static void sa_draw_standard_16x16_impl(const SpritePosition *position,
                                        SpriteDirection direction,
                                        const SpriteEnvironment *env,
                                        uint16_t tile,
                                        uint8_t palette,
                                        bool above_other_sprites)
{
    SpriteDrawParams params = {
        .offset_x = -8,
        .offset_y = -17,
        .palette = palette,
        .tile = tile,
        .wide = true, .tall = true,
        .x_flip = false, .y_flip = false
    };

    sa_draw_standard_impl(position, direction, env, &params, above_other_sprites);
}

static void sa_draw_standard_impl(const SpritePosition *position,
                                  SpriteDirection direction,
                                  const SpriteEnvironment *env,
                                  const SpriteDrawParams *params,
                                  bool above_other_sprites)
{
    bool x_flip = ((direction == RIGHT) != params->x_flip);

    uint32_t x = position->x - env->camera->scroll.x;
    x += (x_flip ? -params->offset_x - 16 : params->offset_x);

    uint32_t y = position->y - env->camera->scroll.y + params->offset_y;

    int32_t screen_padding = 32;
    if (!onscreen(x, y, screen_padding)) {
        return;
    }

    uint32_t x_block = x;
    x_block &= 0x3ff;
    x_block |= (x_flip ? SPRITE_X_FLIP : 0);

    uint32_t y_block = y;
    y_block &= 0x1ff;
    y_block |= params->tall ? SPRITE_16_TALL : 0;
    y_block |= params->wide ? SPRITE_16_WIDE : 0;
    y_block |= params->y_flip ? SPRITE_Y_FLIP : 0;

    uint32_t g_block = params->tile;
    g_block |= 3 << SPRITE_PRIORITY_SHIFT;
    g_block |= params->palette << SPRITE_PAL_SHIFT;

    if (above_other_sprites) {
        sb_write_priority(x_block, y_block, g_block);
    } else {
        sb_write(x_block, y_block, g_block);
    }
}

static bool onscreen(int32_t screen_x, int32_t screen_y, int32_t padding) {
    return
        (screen_x + padding >= 0) &&
        (screen_x - padding < SCREEN_ACTIVE_WIDTH) &&
        (screen_y + padding >= 0) &&
        (screen_y - padding < SCREEN_ACTIVE_HEIGHT);
}

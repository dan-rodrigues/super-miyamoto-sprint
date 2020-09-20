#include "glitter_sprite.h"

#include "sprite_actor.h"

#include "vdp.h"
#include "camera.h"
#include "debug_print.h"
#include "sprite_drawing.h"

static const SpriteDrawParams *draw_params(uint8_t lifetime_step);

void glitter_sprite_main(SpriteActorLight *self, const SpriteEnvironment *env) {
    const uint8_t frames_total = 3;
    const uint8_t frame_tiles = 4;
    const uint8_t lifetime = frames_total * frame_tiles - 1;

    if (self->glitter.lifetime++ == lifetime) {
        sa_free_light(self);
        return;
    }

    const SpriteDrawParams *params = draw_params(self->glitter.lifetime);
    sa_draw_standard_light_multiple(self, env, params, frame_tiles);
}

static const SpriteDrawParams *draw_params(uint8_t lifetime_step) {
    // Could touch this up with more variety as the glitter fades
    static const SpriteDrawParams params[3][4] = {
        {
            {
                .tile = 0x20,
                .offset_x = -8 + 4, .offset_y = -17 + 5,
                .palette = 9, .wide = false, .tall = false, .x_flip = false, .y_flip = false
            },
            {
                .tile = 0x20,
                .offset_x = -8, .offset_y = -17 + 9,
                .palette = 9, .wide = false, .tall = false, .x_flip = false, .y_flip = false
            },
            {
                .tile = 0x20,
                .offset_x = -8 + 8, .offset_y = -17 + 12,
                .palette = 9, .wide = false, .tall = false, .x_flip = false, .y_flip = false
            },
            {
                .tile = 0x20,
                .offset_x = -8 + 3, .offset_y = -17 + 15,
                .palette = 9, .wide = false, .tall = false, .x_flip = false, .y_flip = false
            }
        },
        {
            {
                .tile = 0x21,
                .offset_x = -8 + 4, .offset_y = -17 + 5,
                .palette = 9, .wide = false, .tall = false, .x_flip = false, .y_flip = false
            },
            {
                .tile = 0x21,
                .offset_x = -8, .offset_y = -17 + 9,
                .palette = 9, .wide = false, .tall = false, .x_flip = false, .y_flip = false
            },
            {
                .tile = 0x21,
                .offset_x = -8 + 8, .offset_y = -17 + 12,
                .palette = 9, .wide = false, .tall = false, .x_flip = false, .y_flip = false
            },
            {
                .tile = 0x21,
                .offset_x = -8 + 3, .offset_y = -17 + 15,
                .palette = 9, .wide = false, .tall = false, .x_flip = false, .y_flip = false
            }
        },
        {
            {
                .tile = 0x30,
                .offset_x = -8 + 4, .offset_y = -17 + 5,
                .palette = 9, .wide = false, .tall = false, .x_flip = false, .y_flip = false
            },
            {
                .tile = 0x30,
                .offset_x = -8, .offset_y = -17 + 9,
                .palette = 9, .wide = false, .tall = false, .x_flip = false, .y_flip = false
            },
            {
                .tile = 0x30,
                .offset_x = -8 + 8, .offset_y = -17 + 12,
                .palette = 9, .wide = false, .tall = false, .x_flip = false, .y_flip = false
            },
            {
                .tile = 0x30,
                .offset_x = -8 + 3, .offset_y = -17 + 15,
                .palette = 9, .wide = false, .tall = false, .x_flip = false, .y_flip = false
            }
        },
    };

    return params[lifetime_step / 4];
}

SpriteActorLight *glitter_sprite_init(const SpritePosition *position) {
    SpriteActorLight *actor = sa_alloc_light();
    if (!actor) {
        return NULL;
    }

    sa_init_light(actor, glitter_sprite_main);
    actor->position = *position;
    actor->interacts_with_sprites = false;
    actor->glitter.lifetime = 0;

    return actor;

}

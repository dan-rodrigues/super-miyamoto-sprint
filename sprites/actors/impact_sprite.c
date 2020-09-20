#include "impact_sprite.h"

#include "sprite_actor.h"

#include "vdp.h"
#include "vdp_print.h"
#include "vram_layout.h"
#include "sprite_buffer.h"
#include "camera.h"
#include "debug_print.h"
#include "sprite_drawing.h"

void impact_sprite_main(SpriteActorLight *self, const SpriteEnvironment *env) {
    static const SpriteDrawParams params[] = {
        {
            .tile = 0x7c,
            .offset_x = -8, .offset_y = -9,
            .palette = 9,
            .wide = false, .tall = false,
            .x_flip = false, .y_flip = false
        },
        {
            .tile = 0x7d,
            .offset_x = -8 + 8, .offset_y = -9,
            .palette = 9,
            .wide = false, .tall = false,
            .x_flip = false, .y_flip = false
        },
        {
            .tile = 0x7c,
            .offset_x = -8, .offset_y = -9 + 8,
            .palette = 9,
            .wide = false, .tall = false,
            .x_flip = false, .y_flip = true
        },
        {
            .tile = 0x7d,
            .offset_x = -8 + 8, .offset_y = -9 + 8,
            .palette = 9,
            .wide = false, .tall = false,
            .x_flip = false, .y_flip = true
        },
    };

    sa_draw_standard_light_multiple(self, env, params, SPRITE_DRAW_PARAM_COUNT(params));

    const uint8_t lifetime = 6;
    self->impact.lifetime++;
    if (self->impact.lifetime == lifetime) {
        sa_free_light(self);
    }
}

SpriteActorLight *impact_sprite_init(const SpritePosition *position) {
    SpriteActorLight *actor = sa_alloc_light();
    if (!actor) {
        return NULL;
    }

    sa_init_light(actor, impact_sprite_main);
    actor->position = *position;
    actor->interacts_with_sprites = false;
    actor->impact.lifetime = 0;

    return actor;
}

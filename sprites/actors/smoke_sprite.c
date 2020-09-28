#include "smoke_sprite.h"

#include "sprite_actor.h"
#include "vdp_print.h"
#include "vram_layout.h"
#include "sprite_buffer.h"
#include "camera.h"
#include "debug_print.h"
#include "sprite_drawing.h"

void smoke_sprite_main(SpriteActorLight *self, const SpriteEnvironment *env) {
    SmokeSprite *sub = &self->smoke;

    uint8_t frame = sub->life_counter / 4;
    uint16_t tile = 0x60 + frame * 2;

    const uint8_t palette = 9;

    if (sub->size == SMOKE_SMALL) {
        sa_draw_standard_8x8_light(self, env, tile, palette);
    } else {
        sa_draw_standard_16x16_light(self, env, tile, palette);
    }

    const uint8_t lifetime = 16;

    if (++sub->life_counter == lifetime) {
        sa_free_light(self);
    }
}

SpriteActorLight *smoke_sprite_init(SpritePosition *position, SmokeSpriteSize size) {
    SpriteActorLight *actor = sa_alloc_light();
    if (!actor) {
        return NULL;
    }
    
    sa_init_light(actor, smoke_sprite_main);
    actor->position = *position;
    actor->interacts_with_sprites = false;

    SmokeSprite *sub = &actor->smoke;
    sub->size = size;
    sub->life_counter = (size == SMOKE_SMALL ? 4 : 0);

    return actor;
}

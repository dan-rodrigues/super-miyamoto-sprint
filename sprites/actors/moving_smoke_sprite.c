#include "moving_smoke_sprite.h"

#include "sprite_actor.h"
#include "sprite_drawing.h"

void moving_smoke_sprite_main(SpriteActorLight *self, const SpriteEnvironment *env) {
    MovingSmokeSprite *sub = &self->moving_smoke;

    sa_apply_velocity(&self->velocity, &self->position);

    uint8_t frame = sub->life_counter / 4;
    uint16_t tile = 0x60 + frame * 2;

    const uint8_t palette = 9;

    sa_draw_standard_8x8_light(self, env, tile, palette);

    const uint8_t lifetime = 16;

    if (++sub->life_counter == lifetime) {
        sa_free_light(self);
    }
}

SpriteActorLight *moving_smoke_sprite_init(SpritePosition *position) {
    SpriteActorLight *actor = sa_alloc_light();
    if (!actor) {
        return NULL;
    }

    sa_init_light(actor, moving_smoke_sprite_main);
    actor->position = *position;
    actor->interacts_with_sprites = false;

    MovingSmokeSprite *sub = &actor->moving_smoke;
    sub->life_counter = 4;

    return actor;
}

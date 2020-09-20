#include "smoke_sprite.h"

#include "vdp.h"

#include "sprite_actor.h"
#include "vdp_print.h"
#include "vram_layout.h"
#include "sprite_buffer.h"
#include "camera.h"
#include "debug_print.h"
#include "sprite_drawing.h"

static const uint8_t SMOKE_LIFETIME = 32;

void smoke_sprite_main(SpriteActorLight *self, const SpriteEnvironment *env) {
    self->smoke.custom_field += 1;

    uint8_t frame = self->smoke.life_counter / 8;
    uint16_t tile = 0x60 + frame * 2;

    const uint8_t palette = 9;
    sa_draw_standard_8x8_light(self, env, tile, palette);

    self->smoke.life_counter++;
    if (self->smoke.life_counter == SMOKE_LIFETIME) {
        sa_free_light(self);
    }
}

SpriteActorLight *smoke_sprite_init(SpritePosition *position) {
    SpriteActorLight *actor = sa_alloc_light();
    if (!actor) {
        return NULL;
    }
    
    sa_init_light(actor, smoke_sprite_main);
    actor->position = *position;
    actor->interacts_with_sprites = false;
    actor->smoke.life_counter = 0;

    // (parameterize for size)
    actor->smoke.large = false;

    if (!actor->smoke.large) {
        actor->smoke.life_counter = 8;
    }

    return actor;
}

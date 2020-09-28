#include "spark_sprite.h"

#include "sprite_actor.h"
#include "vdp_print.h"
#include "vram_layout.h"
#include "sprite_buffer.h"
#include "camera.h"
#include "debug_print.h"
#include "sprite_drawing.h"

static const uint8_t SPARK_LIFETIME = 4;

void spark_sprite_main(SpriteActorLight *self, const SpriteEnvironment *env) {
    SparkSprite *sub = &self->spark;

    static const uint16_t frames[] = { 0x10d, 0x11d, 0x10e, 0x11e };

    uint8_t frame = sub->life_counter;
    uint16_t tile = frames[frame];

    const uint8_t palette = 12;

    sa_draw_standard_8x8_light(self, env, tile, palette);

    sub->life_counter++;
    if (sub->life_counter == SPARK_LIFETIME) {
        sa_free_light(self);
    }
}

SpriteActorLight *spark_sprite_init(SpritePosition *position) {
    SpriteActorLight *actor = sa_alloc_light();
    if (!actor) {
        return NULL;
    }

    sa_init_light(actor, spark_sprite_main);
    actor->position = *position;
    actor->interacts_with_sprites = false;

    SparkSprite *sub = &actor->spark;
    sub->life_counter = 0;

    return actor;
}

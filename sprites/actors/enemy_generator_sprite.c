#include "enemy_generator_sprite.h"

#include "vdp.h"
#include "vdp_print.h"

#include "sprite_actor.h"
#include "vram_layout.h"
#include "sprite_buffer.h"
#include "camera.h"
#include "debug_print.h"
#include "sprite_drawing.h"
#include "hero.h"

static const uint8_t SPAWN_INTERVAL = 130;

void enemy_generator_sprite_main(SpriteActor *self, const SpriteEnvironment *env) {
    EnemyGeneratorSprite *sub = &self->enemy_generator;

    // Move

    if (env->hero->pad.l) {
        self->position.x--;
    }
    if (env->hero->pad.r) {
        self->position.x++;
    }

    // Generate

    sub->spawn_counter++;
    if (sub->spawn_counter >= SPAWN_INTERVAL) {
        // Right:
        basic_enemy_sprite_init(&self->position, sub->spawn_toggle);
        sub->spawn_counter = 0;
        // Left:
        SpriteActor *left = basic_enemy_sprite_init(&self->position, !sub->spawn_toggle);
        left->direction = LEFT;

        sub->spawn_counter = 0;
        sub->spawn_toggle = !sub->spawn_toggle;
    }

    // Draw

    const uint16_t tile = 0x0ae;
    const uint8_t palette = 10;
    sa_draw_standard_16x16(self, env, tile, palette);
}

SpriteActor *enemy_generator_sprite_init(SpritePosition *position) {
    SpriteActor *actor = sa_alloc();
    sa_init(actor, enemy_generator_sprite_main);
    actor->position = *position;
    actor->interacts_with_sprites = false;
    actor->enemy_generator.spawn_counter = SPAWN_INTERVAL;
    actor->enemy_generator.spawn_toggle = false;

    return actor;
}

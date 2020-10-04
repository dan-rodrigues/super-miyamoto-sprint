#include "midpoint_sprite.h"

#include "sprite_actor.h"

#include "sprite_block_interaction.h"
#include "sprite_collision.h"
#include "sprite_drawing.h"
#include "extra_task.h"
#include "hero.h"
#include "sound_effects.h"

static void draw(const SpriteActor *self, const SpriteEnvironment *env);

void midpoint_sprite_main(SpriteActor *self, const SpriteEnvironment *env) {
    // Immediately erase self if hero already reached midpoint
    if (env->hero->midpoint_reached) {
        sa_free(self);
        return;
    }

    if (sa_hero_standard_collision(self, env->hero)) {
        hero_mark_midpoint_reached(env->hero);
        sa_free(self);

        se_powerup();

        return;
    }

    draw(self, env);
}

static void draw(const SpriteActor *self, const SpriteEnvironment *env) {
    const uint16_t tile = 0x0e4;
    const uint8_t palette = 11;

    sa_draw_standard_16x16(self, env, tile, palette);
}

SpriteActor *midpoint_sprite_init(const SpritePosition *position) {
    SpriteActor *actor = sa_alloc();
    sa_init(actor, midpoint_sprite_main);
    actor->position = *position;

    static const SpriteBoundingBox sprite_box = {
        .offset = { -8, -16 },
        .size = { 16, 16 }
    };
    actor->bounding_box = sprite_box;
    actor->stompable = false;
    actor->touch_hurts_hero = false;
    actor->interacts_with_sprites = false;
    actor->immune_to_projectiles = true;

    return actor;
}

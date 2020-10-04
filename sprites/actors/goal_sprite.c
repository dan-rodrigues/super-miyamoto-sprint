#include "goal_sprite.h"

#include "sprite_actor.h"

#include "sprite_block_interaction.h"
#include "sprite_collision.h"
#include "sprite_drawing.h"
#include "extra_task.h"

static void draw(const SpriteActor *self, const SpriteEnvironment *env);

void goal_sprite_main(SpriteActor *self, const SpriteEnvironment *env) {
    if (sa_hero_standard_collision(self, env->hero)) {
        // TODO: handle actual end-of-level event
        level_reload_sequence_task_init(10);

        sa_free(self);
        return;
    }

    draw(self, env);
}

static void draw(const SpriteActor *self, const SpriteEnvironment *env) {
    const uint16_t tile = 0x0e2;
    const uint8_t palette = 11;

    sa_draw_standard_16x16(self, env, tile, palette);
}

SpriteActor *goal_sprite_init(const SpritePosition *position) {
    SpriteActor *actor = sa_alloc();
    sa_init(actor, goal_sprite_main);
    actor->position = *position;

    static const SpriteBoundingBox sprite_box = {
        .offset = { -8, -16 },
        .size = { 16, 16 }
    };
    actor->bounding_box = sprite_box;
    actor->stompable = false;
    actor->touch_hurts_hero = false;

    return actor;
}

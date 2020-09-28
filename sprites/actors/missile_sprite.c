#include "missile_sprite.h"

#include "sprite_actor.h"
#include "sprite_collision.h"
#include "sprite_drawing.h"
#include "sprite_block_interaction.h"

#include "global_timers.h"
#include "smoke_sprite.h"
#include "sound_effects.h"

static void explode(SpriteActorLight *self);

void missile_sprite_main(SpriteActorLight *self, const SpriteEnvironment *env) {
    MissileSprite *sub = &self->missile;

    sa_apply_velocity(&self->velocity, &self->position);

    sa_draw_standard_16x16_light(self, env, 0x108, 12);

    // Block collision

    const SpriteOffset missile_head_offset_left = { 0, -8 };
    const SpriteOffset missile_head_offset_right = { 7, -8 };

    const SpriteOffset missile_head_offset = (self->direction == LEFT
                                              ? missile_head_offset_left : missile_head_offset_right);

    SpritePosition block_test_point = self->position;
    sa_apply_offset(&missile_head_offset, &block_test_point);

    if (sa_point_inside_solid_block(&block_test_point)) {
        explode(self);
        return;
    }

    // Hero collision

    if (sa_hero_standard_collision_light(self, env->hero)) {
        explode(self);
        return;
    }

    // Sprite collision

    if (sa_light_sprite_collision(self)) {
        explode(self);
        return;
    }

    // Smoke trail effects

    const SpriteOffset smoke_offset_left = { 9, -5 };
    const SpriteOffset smoke_offset_right = { -7, -5 };

    const SpriteOffset smoke_offset = (self->direction == LEFT
                                       ? smoke_offset_left : smoke_offset_right);

    SpritePosition smoke_point = self->position;
    sa_apply_offset(&smoke_offset, &smoke_point);

    if (gt_mod_8() == sub->index_mod_4) {
        smoke_sprite_init(&smoke_point, SMOKE_SMALL);
    }
}

static void explode(SpriteActorLight *self) {
    smoke_sprite_init(&self->position, SMOKE_LARGE);
    sa_free_light(self);

    se_explosion();
}

SpriteActorLight *missile_sprite_init(const SpritePosition *position) {
    SpriteActorLight *actor = sa_alloc_light();
    if (!actor) {
        return NULL;
    }

    sa_init_light(actor, missile_sprite_main);
    actor->position = *position;

    static const SpriteBoundingBox box = {
        .offset = { -6, -9 },
        .size = { 13, 4 }
    };
    actor->bounding_box = box;

    MissileSprite *sub = &actor->missile;
    sub->index_mod_4 = actor->handle.index % 8;

    return actor;
}

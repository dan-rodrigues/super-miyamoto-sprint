#include "basic_enemy_sprite.h"

#include "sprite_actor.h"

#include "vdp.h"
#include "vram_layout.h"
#include "camera.h"
#include "sprite_block_interaction.h"
#include "sprite_collision.h"
#include "sprite_drawing.h"

#include "debug_print.h"

static void move(SpriteActor *self);
static void draw(SpriteActor *self, const SpriteEnvironment *env);

void basic_enemy_sprite_main(SpriteActor *self, const SpriteEnvironment *env) {
    BasicEnemy *sub = &self->basic_enemy;

    move(self);

    if (self->killed) {
        if (sub->squished && !(--sub->expiry_counter)) {
            sa_free(self);
            return;
        }
    } else if (sa_hero_standard_collision(self, env) == SA_HERO_COLLISION_STOMP) {
        sub->squished = true;
    }

    sa_other_sprite_collision(self, env);

    draw(self, env);
}

// This can be the reference for a reusable generic sprite interaction

static void move(SpriteActor *self) {
    if (sa_perform_default_movement(self)) {
        return;
    }

    static const SpriteBoundingBox horizontal_box = {
        .offset = { -6, -10 },
        .size = { 12, 6 }
    };

    static const SpriteBoundingBox vertical_box = {
        .offset = { -4, -13 },
        .size = { 4 + 3, 13 }
    };

    sa_apply_velocity_with_gravity(self, &vertical_box);
    sa_apply_horizontal_block_interaction_updates(self, &horizontal_box);

    if (!self->killed) {
        const int32_t speed = Q_1 / 2;
        self->velocity.x = sa_velocity_from_speed(speed, self->direction);
    } else {
        self->velocity.x = 0;
    }
}

static void draw(SpriteActor *self, const SpriteEnvironment *env) {
    BasicEnemy *sub = &self->basic_enemy;

    sub->frame_counter++;

    uint16_t tile_base = 0x080;
    if (sub->spiked) {
        tile_base += 0x20;
    }

    static const uint8_t animation_loop[] = {0, 1, 0, 2};
    const uint8_t tile_squished = 3;
    const uint8_t tile_falling = 4;
    uint8_t tile;

    if (!self->killed) {
        tile = animation_loop[(sub->frame_counter / 8) % 4];
    } else if (!self->falls_off_when_killed) {
        tile = tile_squished;
    } else {
        tile = tile_falling;
    }

    const uint8_t palette = 10;
    sa_draw_standard_16x16(self, env, tile_base + tile * 2, palette);
}

SpriteActor *basic_enemy_sprite_init(const SpritePosition *position, bool spiked) {
    SpriteActor *actor = sa_alloc();
    sa_init(actor, basic_enemy_sprite_main);
    actor->position = *position;
    actor->direction = RIGHT;
    actor->stomp_hurts_hero = spiked;
    actor->falls_off_when_killed = false;
    actor->can_ride = true;

    const uint8_t time_to_expire = 40;

    static const SpriteBoundingBox sprite_box = {
        .offset = { -8, -10 },
        .size = { 16, 10 }
    };
    actor->bounding_box = sprite_box;

    BasicEnemy *sub = &actor->basic_enemy;
    sub->expiry_counter = time_to_expire;
    sub->spiked = spiked;
    sub->squished = false;

    return actor;
}

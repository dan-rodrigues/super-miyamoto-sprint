#include "jetpack_enemy_sprite.h"

#include "math_util.h"

#include "sprite_actor.h"

#include "sprite_block_interaction.h"
#include "sprite_collision.h"
#include "sprite_drawing.h"
#include "global_timers.h"

static void move(SpriteActor *self, JetpackEnemySprite *sub);
static void draw(const SpriteActor *self, const JetpackEnemySprite *sub, const SpriteEnvironment *env);

void jetpack_enemy_sprite_main(SpriteActor *self, const SpriteEnvironment *env) {
    JetpackEnemySprite *sub = &self->jetpack_enemy;

    move(self, sub);

    if (!self->killed) {
        self->direction = sa_direction_facing_hero(self, env->hero);

        if (sub->grounded) {
            const int32_t decel = Q_1 / 8;
            sa_horizontal_deceleration(&self->velocity, decel);
        } else {
            const int32_t travel_speed = Q_1 / 2;
            const int32_t accel = Q_1 / 32;
            sa_horizontal_acceleration(&self->velocity, self->direction, accel, travel_speed);
        }

        sub->boosting = !sa_above_hero(self, env->hero);
        if (sub->boosting) {
            const int32_t rise_accel = Q_1 / 3;
            const int32_t rise_max_speed = Q_1;
            self->velocity.y -= rise_accel;
            self->velocity.y = MAX(-rise_max_speed, self->velocity.y);
        } else {
            // Gravity will lower position as needed
            const int32_t coasting_accel = Q_1 / 5;
            const int32_t fall_max_speed = Q_1;
            self->velocity.y -= coasting_accel;
            self->velocity.y = MIN(fall_max_speed, self->velocity.y);
        }

        sa_hero_standard_collision(self, env);
        sa_other_sprite_collision(self, env);
    }

    draw(self, sub, env);
}

static void move(SpriteActor *self, JetpackEnemySprite *sub) {
    if (sa_perform_default_movement(self)) {
        return;
    }

    static const SpriteBoundingBox horizontal_box = {
        .offset = { -7, -8 },
        .size = { 13, 4 }
    };

    static const SpriteBoundingBox vertical_box = {
        .offset = { -4, -15 },
        .size = { 4 + 3, 15 }
    };

    sub->grounded = sa_apply_velocity_with_gravity(self, &vertical_box);
    sa_apply_horizontal_block_interaction_updates(self, &horizontal_box);
}

static void draw(const SpriteActor *self, const JetpackEnemySprite *sub, const SpriteEnvironment *env) {
    const uint16_t airborne_tile = 0x0ce;
    const uint16_t killed_tile = 0x0e0;
    const uint8_t palette = 11;

    uint16_t live_tile = airborne_tile;
    uint16_t tile = (self->killed ? killed_tile : live_tile);

    sa_draw_standard_16x16(self, env, tile, palette);

    if (sub->boosting) {
        static const SpriteDrawParams params[] = {
            {
                .tile = 0x10f,
                .offset_x = 3, .offset_y = -5,
                .palette = 12,
                .wide = false, .tall = false,
                .x_flip = false, .y_flip = false
            },
            {
                .tile = 0x11f,
                .offset_x = 3, .offset_y = -5,
                .palette = 12,
                .wide = false, .tall = false,
                .x_flip = false, .y_flip = false
            }
        };

        uint32_t frame_index = (gt() / 4) % 2;
        sa_draw_standard(self, env, &params[frame_index]);
    }
}

SpriteActor *jetpack_enemy_sprite_init(const SpritePosition *position) {
    SpriteActor *actor = sa_alloc();
    sa_init(actor, jetpack_enemy_sprite_main);
    actor->position = *position;

    static const SpriteBoundingBox sprite_box = {
        .offset = { -5, -12 },
        .size = { 12, 12 }
    };
    actor->bounding_box = sprite_box;
    actor->invert_velocity_upon_hitting_wall = false;

    JetpackEnemySprite *sub = &actor->jetpack_enemy;
    sub->grounded = false;
    sub->boosting = false;

    return actor;
}

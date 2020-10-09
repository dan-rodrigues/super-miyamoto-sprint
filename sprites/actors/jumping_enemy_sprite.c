#include "jumping_enemy_sprite.h"

#include "sprite_actor.h"

#include "sprite_block_interaction.h"
#include "sprite_collision.h"
#include "sprite_drawing.h"

static void move(SpriteActor *self, JumpingEnemySprite *sub);
static void draw(const SpriteActor *self, const JumpingEnemySprite *sub, const SpriteEnvironment *env);

void jumping_enemy_sprite_main(SpriteActor *self, const SpriteEnvironment *env) {
    JumpingEnemySprite *sub = &self->jumping_enemy;

    move(self, sub);

    if (!self->killed) {
        if (sub->grounded) {
            self->velocity.x = 0;

            bool jump = !(sub->jump_timer--);

            if (jump) {
                const uint8_t default_jump_delay = 60;
                const int32_t jump_speed_x = Q_1;
                const int32_t jump_speed_y = Q_1 * 3;

                self->direction = sa_direction_facing_hero(self, env->hero);
                self->velocity.y = -jump_speed_y;
                self->velocity.x = sa_velocity_from_speed(jump_speed_x, self->direction);

                sub->jump_timer = default_jump_delay;
            }
        }

        sa_hero_standard_collision(self, env);
        sa_other_sprite_collision(self, env);
    } else {
        sub->grounded = false;
    }

    draw(self, sub, env);
}

static void move(SpriteActor *self, JumpingEnemySprite *sub) {
    if (sa_perform_default_movement(self)) {
        return;
    }

    static const SpriteBoundingBox horizontal_box = {
        .offset = { -6, -8 },
        .size = { 14, 4 }
    };

    static const SpriteBoundingBox vertical_box = {
        .offset = { -4, -16 },
        .size = { 4 + 3, 16 }
    };

    sub->grounded = sa_apply_velocity_with_gravity(self, &vertical_box);
    sa_apply_horizontal_block_interaction_updates(self, &horizontal_box);
}

static void draw(const SpriteActor *self, const JumpingEnemySprite *sub, const SpriteEnvironment *env) {
    const uint16_t grounded_tile = 0x0ca;
    const uint16_t airborne_tile = 0x0cc;
    const uint16_t killed_tile = 0x0e0;
    const uint8_t palette = 11;

    uint16_t live_tile = (sub->grounded ? grounded_tile : airborne_tile);
    uint16_t tile = (self->killed ? killed_tile : live_tile);

    sa_draw_standard_16x16(self, env, tile, palette);
}

SpriteActor *jumping_enemy_sprite_init(const SpritePosition *position) {
    SpriteActor *actor = sa_alloc();
    sa_init(actor, jumping_enemy_sprite_main);
    actor->position = *position;

    static const SpriteBoundingBox sprite_box = {
        .offset = { -5, -12 },
        .size = { 12, 12 }
    };
    actor->bounding_box = sprite_box;

    JumpingEnemySprite *sub = &actor->jumping_enemy;
    sub->grounded = false;
    sub->jump_timer = 0;

    return actor;
}

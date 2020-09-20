#include "layered_enemy_sprite.h"

#include "sprite_actor.h"
#include "sprite_collision.h"
#include "sprite_drawing.h"

static void move(SpriteActor *self);
static void draw(SpriteActor *self, const SpriteEnvironment *env);
static const SpriteBoundingBox *sprite_interaction_box(const SpriteActor *self);

void layered_enemy_sprite_main(SpriteActor *self, const SpriteEnvironment *env) {
    LayeredEnemy *sub = &self->layered_enemy;

    move(self);

    // Hero interaction

    if (!self->killed && (sa_hero_standard_collision(self, env->hero) == SA_HERO_COLLISION_STOMP)) {
        if (sub->small) {
            sa_kill_sprite(self);
        } else {
            // Sprite shrinks to its smaller size when initially stomped
            sub->small = true;
            self->bounding_box = *sprite_interaction_box(self);
        }
    }

    sa_other_sprite_collision(self);

    draw(self, env);
}

static void move(SpriteActor *self) {
    if (sa_perform_default_movement(self)) {
        return;
    }

    static const SpriteBoundingBox horizontal_box = {
        .offset_x = -5,
        .offset_y = -10,
        .width = 12,
        .height = 6
    };

    static const SpriteBoundingBox vertical_box = {
        .offset_x = -4, .offset_y = -13,
        .width = 4 + 3, .height = 13
    };

    sa_apply_velocity_with_gravity(self, &vertical_box);
    sa_apply_horizontal_block_interaction_updates(self, &horizontal_box);

    if (!self->killed) {
        const int32_t speed = Q_1 / 2;
        self->velocity.x = (self->direction == RIGHT ? speed : -speed);
    } else {
        self->velocity.x = 0;
    }
}

static void draw(SpriteActor *self, const SpriteEnvironment *env) {
    LayeredEnemy *sub = &self->layered_enemy;

    // Tall frames
    static const SpriteDrawParams params[2][2] = {
        {
            {
                .tile = 0x0c0,
                .offset_x = -8, .offset_y = -17,
                .palette = 11,
                .wide = true, .tall = true,
                .x_flip = false, .y_flip = false
            },
            {
                .tile = 0x0c8,
                .offset_x = -8, .offset_y = -17 - 15,
                .palette = 11,
                .wide = true, .tall = true,
                .x_flip = false, .y_flip = false
            }
        },
        {
            {
                 .tile = 0x0c2,
                 .offset_x = -8, .offset_y = -17,
                 .palette = 11,
                 .wide = true, .tall = true,
                 .x_flip = false, .y_flip = false
             },
             {
                 .tile = 0x0c8,
                 .offset_x = -8, .offset_y = -17 - 16,
                 .palette = 11,
                 .wide = true, .tall = true,
                 .x_flip = false, .y_flip = false
             }
        }
    };

    // Short frames
    static const SpriteDrawParams small_params[] = {
        {
            .tile = 0x0c4,
            .offset_x = -8, .offset_y = -17,
            .palette = 11,
            .wide = true, .tall = true,
            .x_flip = false, .y_flip = false
        },
        {
            .tile = 0x0c6,
            .offset_x = -8, .offset_y = -17,
            .palette = 11,
            .wide = true, .tall = true,
            .x_flip = false, .y_flip = false
        }
    };

    sub->frame_counter++;

    uint8_t frame_index = (sub->frame_counter / 8) % 2;
    if (self->killed) {
        frame_index = 0;
    }

    if (sub->small) {
        sa_draw_standard(self, env, &small_params[frame_index]);
    } else {
        sa_draw_standard_multiple(self, env, params[frame_index], 2);
    }
}

SpriteActor *layered_enemy_sprite_init(const SpritePosition *position) {
    SpriteActor *actor = sa_alloc();
    sa_init(actor, layered_enemy_sprite_main);
    actor->position = *position;
    actor->direction = RIGHT;
    actor->dies_upon_stomp = false;
    actor->can_ride = true;

    actor->layered_enemy.small = false;
    actor->layered_enemy.frame_counter = 0;

    actor->bounding_box = *sprite_interaction_box(actor);

    return actor;
}

static const SpriteBoundingBox *sprite_interaction_box(const SpriteActor *self) {
    static const SpriteBoundingBox tall_box = {
        .height = 22,
        .width = 16,
        .offset_x = -8,
        .offset_y = -22
    };

    static const SpriteBoundingBox short_box = {
        .height = 14,
        .width = 16,
        .offset_x = -8,
        .offset_y = -14
    };

    return self->layered_enemy.small ? &short_box : &tall_box;
}

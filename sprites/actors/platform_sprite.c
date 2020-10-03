#include "platform_sprite.h"

#include "math_util.h"

#include "sprite_actor.h"
#include "sprite_collision.h"
#include "sprite_drawing.h"
#include "sprite_position.h"
#include "hero.h"

static void move(SpriteActor *self, const PadInputDecoded *pad);
static const SpriteBoundingBox *interaction_box(const SpriteActor *actor);
static void platform_deferred_draw(SpriteActor *self, const SpriteEnvironment *env);
static void auto_motion(SpriteActor *self, int16_t position_component, int32_t *velocity_component);

static void test_movement(SpriteActor *self, const PadInputDecoded *pad);

void platform_sprite_main(SpriteActor *self, const SpriteEnvironment *env) {
    PlatformSprite *sub = &self->platform;

    move(self, &env->hero->pad);

    // Platform has no defined direction; always "face left"
    self->direction = LEFT;

    // Hero interaction
    // Note this would use the LAST frame box with how the loop is configured

    Hero *hero = env->hero;
    hero_platform_interaction(hero, self);

    // Other sprites interaction

    sa_other_sprite_platform_check(self);

    // 1. Cache current position
    SpritePosition position_prev = self->position;

    // 2. Update position
    sa_apply_velocity(&self->velocity, &self->position);

    // 3. Store offset from previous for use by riding sprites
    SpriteOffset offset = {
        .x = self->position.x - position_prev.x,
        .y = self->position.y - position_prev.y
    };
    sub->frame_offset = offset;

    // Defer drawing as camera params are only set *after* hero is drawn
    sa_add_deferred_draw_task(self, platform_deferred_draw);
}

static void move(SpriteActor *self, const PadInputDecoded *pad) {
    PlatformSprite *sub = &self->platform;

    switch (sub->motion) {
        case PLATFORM_MOTION_AUTO_X:
            auto_motion(self, self->position.x, &self->velocity.x);
            break;
        case PLATFORM_MOTION_AUTO_Y:
            auto_motion(self, self->position.y, &self->velocity.y);
            break;
        case PLATFORM_MOTION_NONE:
            break;
        case PLATFORM_MOTION_GAMEPAD:
            test_movement(self, pad);
            break;
    }
}

static void auto_motion(SpriteActor *self, int16_t position_component, int32_t *velocity_component) {
    PlatformSprite *sub = &self->platform;

    const int32_t travel_max_speed = Q_1;
    const int32_t travel_accel = Q_1 / 20;

    if (sub->travelling_towards_lower_bound) {
        bool travel_lower = position_component > sub->lower_target_bound;
        if (travel_lower) {
            *velocity_component -= travel_accel;
            *velocity_component = MAX(*velocity_component, -travel_max_speed);
        } else {
            sub->travelling_towards_lower_bound = false;
        }
    } else {
        bool travel_upper = position_component < sub->upper_target_bound;
        if (travel_upper) {
            *velocity_component += travel_accel;
            *velocity_component = MIN(*velocity_component, travel_max_speed);
        } else {
            sub->travelling_towards_lower_bound = true;
        }
    }
}

// Deferred drawings

static void platform_deferred_draw(SpriteActor *self, const SpriteEnvironment *env) {
    static const SpriteDrawParams params[] = {
        {
            .tile = 0x022,
            .offset_x = -16, .offset_y = -17,
            .palette = 9,
            .wide = true, .tall = true,
            .x_flip = false, .y_flip = false
        },
        {
            .tile = 0x023,
            .offset_x = -8, .offset_y = -17,
            .palette = 9,
            .wide = true, .tall = true,
            .x_flip = false, .y_flip = false
        },
        {
            .tile = 0x024,
            .offset_x = -8, .offset_y = -17,
            .palette = 9,
            .wide = true, .tall = true,
            .x_flip = false, .y_flip = false
        }
    };

    sa_draw_standard(self, env, &params[0]);

    SpriteDrawParams middle_params = params[1];
    for (uint32_t i = 0; i < 4; i++) {
        sa_draw_standard(self, env, &middle_params);
        middle_params.offset_x += 16;
    }

    SpriteDrawParams end_params = params[2];
    end_params.offset_x += middle_params.offset_x;
    sa_draw_standard(self, env, &end_params);
}

SpriteActor *platform_sprite_init(const SpritePosition *position, PlatformSpriteMotion motion) {
    SpriteActor *actor = sa_alloc();
    sa_init(actor, platform_sprite_main);
    actor->position = *position;

    // Need to review how vertical positions are offset in sprite defs
    actor->position.y--;

    actor->rideable = true;
    actor->interacts_with_sprites = false;

    actor->bounding_box = *interaction_box(actor);

    PlatformSprite *sub = &actor->platform;
    sub->frame_offset.x = 0;
    sub->frame_offset.y = 0;
    sub->motion = motion;
    
    const int16_t travel_distance = 48;
    bool x_motion = (motion == PLATFORM_MOTION_AUTO_X);
    int16_t bound_component = (x_motion ? actor->position.x : actor->position.y);
    sub->lower_target_bound = bound_component - travel_distance / 2;
    sub->upper_target_bound = bound_component + travel_distance / 2;

    sub->travelling_towards_lower_bound = true;

    return actor;
}

// Debug movement using pad
    
static void test_movement(SpriteActor *self, const PadInputDecoded *pad) {
    self->velocity.x = (pad->l ? Q_1 * 1 : 0);
    self->velocity.y = (pad->r ? -Q_1 * 1 : 0);
}

// (TODO: configurable length)
static const SpriteBoundingBox *interaction_box(const SpriteActor *actor) {
    static const SpriteBoundingBox box = {
        .size = { 16 * 5, 16 },
        .offset = { -16, -16 }
    };

    return &box;
}

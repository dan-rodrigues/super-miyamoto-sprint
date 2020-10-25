#include "tank_sprite.h"

#include "math_util.h"
#include "assert.h"

#include "sprite_actor.h"
#include "sprite_collision.h"
#include "sprite_drawing.h"

#include "hero.h"
#include "sound_effects.h"

static void move(SpriteActor *self);
static void draw(const SpriteActor *self, TankSprite *sub, const SpriteEnvironment *env);

static void enemy_driver_update(SpriteActor *self, TankSprite *sub, const Hero *hero);
static bool can_see_hero(const SpriteActor *self, const Hero *hero);
static void position_driver(SpriteActor *self, TankSprite *sub);
static void missile_launch_update(SpriteActor *self, TankSprite *sub);
static void prepare_missile_firing(TankSprite *sub);
static bool enemy_is_driving(SpriteActor *self, TankSprite *sub);
static void exhaust_decoration(const SpriteActor *self, TankSprite *sub, const Hero *hero);

void tank_sprite_main(SpriteActor *self, const SpriteEnvironment *env) {
    TankSprite *sub = &self->tank;
    Hero *hero = env->hero;

    bool hero_driving = hero_in_vehicle(hero, self);
    bool enemy_driving = enemy_is_driving(self, sub);

    if (self->killed) {
        move(self);

        if (enemy_driving) {
            SpriteActor *driver = sa_get(sub->driver_enemy_handle);
            driver->killed = true;
        }
    } else if (hero_driving) {
        self->direction = hero->direction;
        self->position = hero->position;
    } else if (enemy_driving) {
        enemy_driver_update(self, sub, hero);
        move(self);
        position_driver(self, sub);

        sa_hero_standard_collision(self, env);

        sa_other_sprite_collision(self, env);
    } else {
        // Hero can enter it

        move(self);

        const int32_t horizontal_decel = Q_1 / 16;
        sa_horizontal_deceleration(&self->velocity, horizontal_decel);

        if (sub->hero_collision_grace) {
            sub->hero_collision_grace--;
        }

        bool hero_should_enter = !hero_in_any_vehicle(hero);
        hero_should_enter &= (sa_hero_standard_collision(self, env) != SA_HERO_COLLISION_NONE);
        hero_should_enter &= !sub->hero_collision_grace;
        hero_should_enter &= hero->velocity.y > 0;
        hero_should_enter &= hero->position.y < self->position.y;

        if (hero_should_enter) {
            hero_enter_vehicle(hero, self);
            hero->velocity.x = 0;
        }

        sa_other_sprite_collision(self, env);
    }

    self->can_ride = !hero_driving;
    self->stomp_hurts_hero = enemy_driving;
    self->touch_hurts_hero = enemy_driving;

    // Missile launch update since it spans multiple frames

    missile_launch_update(self, sub);

    // Decorations

    exhaust_decoration(self, sub, hero);

    draw(self, sub, env);
}

static int32_t speed_x(const SpriteActor *self, const Hero *hero) {
    return ABS(hero_in_vehicle(hero, self) ? hero->velocity.x : self->velocity.x);
}

static void exhaust_decoration(const SpriteActor *self, TankSprite *sub, const Hero *hero) {
    int32_t speed = speed_x(self, hero);
    speed += Q_1;

    uint32_t smoke_acc = sub->exahust_sprite_acc;
    smoke_acc += speed / (Q_1 / 8);
    if (smoke_acc > 128) {
        smoke_acc -= 128;

        const SpriteOffset smoke_offset = { 20, -18 };

        SpritePosition smoke_position = self->position;
        sa_apply_offset_flip(&smoke_offset, &smoke_position, self->direction);

        SpriteActorLight *moving_smoke = moving_smoke_sprite_init(&smoke_position);
        moving_smoke->velocity.x = 0;
        moving_smoke->velocity.y = -Q_1 / 2;

    }
    sub->exahust_sprite_acc = smoke_acc;
}

static bool enemy_is_driving(SpriteActor *self, TankSprite *sub) {
    if (!sa_handle_live(sub->driver_enemy_handle)) {
        return false;
    }

    const SpriteActor *driver = sa_get(sub->driver_enemy_handle);
    return !driver->killed;
}

static void prepare_missile_firing(TankSprite *sub) {
    sub->firing = true;
    sub->launch_pending = false;
    sub->launch_counter = 0;
    sub->missiles_launched = 0;
}

void tank_sprite_hero_drive_update(SpriteActor *self, Hero *hero, SpriteVehicleHeroContext *hero_context) {
    TankSprite *sub = &self->tank;

    HeroVehicleControl control = { 0 };
    bool idle = !hero_vehicle_control_state(hero, &control);

    const int32_t slow_accel = Q_1 / 16;
    const int32_t fast_accel = Q_1 / 8;

    const int32_t max_speed = Q_1 * 2;
    const int32_t idle_decel = Q_1 / 8;

    if ((control.action || sub->launch_pending) && !sub->firing) {
        prepare_missile_firing(sub);
    } else if (control.action) {
        sub->launch_pending = true;
    }

    int32_t accel = (control.move_fast ? fast_accel : slow_accel);

    if (control.move_left) {
        sa_horizontal_acceleration(&hero->velocity, LEFT, accel, max_speed);
    } else if (control.move_right) {
        sa_horizontal_acceleration(&hero->velocity, RIGHT, accel, max_speed);
    }

    if (control.jump) {
        // Standard hero jump is used, but this is used for decoration
        static const int8_t x_velocities[] = { 0, -8, 8 };
        for (uint32_t i = 0; i < sizeof(x_velocities) / sizeof(int8_t); i++) {
            SpritePosition position = self->position;
            position.x += x_velocities[i];
            SpriteActorLight *moving_smoke = moving_smoke_sprite_init(&position);
            if (!moving_smoke) {
                break;
            }

            moving_smoke->velocity.x = 0;
            moving_smoke->velocity.y = Q_1;
        }
    }

    if (control.eject) {
        const int32_t eject_speed = Q_1 * 4;
        const int32_t eject_y_displacement = -12;

        self->velocity.x = hero->velocity.x;
        self->velocity.y = (hero->velocity.y < 0 ? hero->velocity.y / 2 : hero->velocity.y);
        sub->hero_collision_grace = 5;

        hero->velocity.y = -eject_speed;

        // This does alter the hero position but this is done before caching hero AABB
        hero->position.y += eject_y_displacement;

        sa_handle_clear(&hero->vehicle_sprite_handle);
    }

    if (idle) {
        sa_horizontal_deceleration(&hero->velocity, idle_decel);
    }

    // Sprite interaction box to replace hero's, used to ride platforms and enemy collision

    static const SpriteBoundingBox sprite_box = {
        .offset = { -12, -16 },
        .size = { 24, 16 }
    };

    hero_context->sprite_box = sprite_box;

    // Block interaction boxes for hero:

    // Center mass

    static const SpriteBoundingBox horizontal_box = {
        .offset = { -11, -12 },
        .size = { 22, 8 }
    };

    static const SpriteBoundingBox vertical_box = {
        .offset = { -8, -16 },
        .size = { 16, 16 }
    };

    hero_context->horizontal_block_boxes[0] = horizontal_box;
    hero_context->vertical_block_boxes[0] = vertical_box;

    // Hero portion

    static const SpriteBoundingBox hero_horizontal_box = {
        .offset = { -8, -20 },
        .size = { 16, 16 }
    };

    static const SpriteBoundingBox hero_vertical_box = {
        .offset = { -4, -26 },
        .size = { 8, 26 }
    };

    hero_context->horizontal_block_boxes[1] = hero_horizontal_box;
    hero_context->vertical_block_boxes[1] = hero_vertical_box;
    hero_context->box_count = 2;
}

static void missile_launch_update(SpriteActor *self, TankSprite *sub) {
    if (!sub->firing) {
        return;
    }

    static const SpriteOffset launch_offsets[] = {
        { -8, -3 }, { -8, -7 }, { -8, 1 }, { -8, -3 }
    };
    static const size_t offset_count = sizeof(launch_offsets) / sizeof(SpriteOffset);

    // There's a gap between missile launches

    if (sub->launch_counter) {
        sub->launch_counter--;
        return;
    } else if (sub->missiles_launched == offset_count) {
        if (sub->launch_pending) {
            prepare_missile_firing(sub);
        } else {
            sub->firing = false;
            return;
        }
    }

    // Launch another

    const uint32_t launch_interval = 4;
    sub->launch_counter = launch_interval;

    SpritePosition launch_position = self->position;
    sa_apply_offset_flip(&launch_offsets[sub->missiles_launched++], &launch_position, self->direction);

    SpriteActorLight *missile = missile_sprite_init(&launch_position);
    if (!missile) {
        return;
    }

    spark_sprite_init(&launch_position);
    se_missile_launch();

    const int32_t launch_x_speed = Q_1 * 4;

    missile->direction = self->direction;
    missile->velocity.x = sa_velocity_from_speed(launch_x_speed, missile->direction);
    missile->velocity.y = 0;
    missile->hurts_hero = enemy_is_driving(self, sub);
}

static void move(SpriteActor *self) {
    if (sa_perform_default_movement(self)) {
        return;
    }

    static const SpriteBoundingBox horizontal_box = {
        .offset = { -11, -12 },
        .size = { 23, 8 }
    };

    static const SpriteBoundingBox vertical_box = {
        .offset = { -8, -16 },
        .size = { 16, 16 }
    };

    sa_apply_velocity_with_gravity(self, &vertical_box);
    sa_apply_horizontal_block_interaction_updates(self, &horizontal_box);
}

static void transition_enemy_driver_state(SpriteActor *self, TankSprite *sub, TankDrivenState new_state) {
    switch (new_state) {
        case TANK_STATE_PATROL_ACCEL:
            self->direction = sa_direction_flipped(self->direction);
            break;
        case TANK_STATE_PATROL_DECEL:
            break;
        case TANK_STATE_CHASE_HERO:
            break;
    }

    sub->enemy_driver_state = new_state;
    sub->current_state_duration = 0;
    sub->frames_since_direction_change = 0;
}

static bool can_see_hero(const SpriteActor *self, const Hero *hero) {
    bool hero_on_left = (hero->position.x < self->position.x);
    bool facing_left = (self->direction == LEFT);

    const int32_t visible_distance = 250;
    bool close_enough = ABS(hero->position.x - self->position.x) < visible_distance;

    return (hero_on_left == facing_left) && close_enough;
}

static void enemy_driver_update(SpriteActor *self, TankSprite *sub, const Hero *hero) {
    const uint8_t patrol_duration = 120;

    sub->current_state_duration++;

    switch (sub->enemy_driver_state) {
        case TANK_STATE_PATROL_ACCEL: {
            const int32_t accel = Q_1 / 32;
            const int32_t max_speed = Q_1 / 2;
            sa_horizontal_acceleration(&self->velocity, self->direction, accel, max_speed);

            if (can_see_hero(self, hero)) {
                transition_enemy_driver_state(self, sub, TANK_STATE_CHASE_HERO);
            } else if (sub->current_state_duration >= patrol_duration) {
                transition_enemy_driver_state(self, sub, TANK_STATE_PATROL_DECEL);
            }
        } break;
        case TANK_STATE_PATROL_DECEL: {
            const int32_t decel = Q_1 / 32;
            sa_horizontal_deceleration(&self->velocity, decel);

            if (can_see_hero(self, hero)) {
                transition_enemy_driver_state(self, sub, TANK_STATE_CHASE_HERO);
            } else if ((self->velocity.x == 0) && (sub->current_state_duration >= patrol_duration)) {
                transition_enemy_driver_state(self, sub, TANK_STATE_PATROL_ACCEL);
            }
        } break;
        case TANK_STATE_CHASE_HERO: {
            const int32_t accel = Q_1 / 24;
            const int32_t max_speed = Q_1;
            const uint8_t direction_change_min_frames = 60;

            bool hero_on_left = (hero->position.x < self->position.x);
            SpriteDirection target_direction = (hero_on_left ? LEFT : RIGHT);

            bool should_face_hero = (target_direction != self->direction);
            should_face_hero &= (sub->frames_since_direction_change >= direction_change_min_frames);
            if (should_face_hero) {
                self->direction = target_direction;
                sub->frames_since_direction_change = 0;
            } else if (sub->frames_since_direction_change < direction_change_min_frames) {
                sub->frames_since_direction_change++;
            }

            sa_horizontal_acceleration(&self->velocity, self->direction, accel, max_speed);

            const uint8_t launch_interval = 64;
            if (!(sub->current_state_duration % launch_interval)) {
                prepare_missile_firing(sub);
            }
        } break;
    }
}

static void position_driver(SpriteActor *self, TankSprite *sub) {
    SpriteActor *driver = sa_get(sub->driver_enemy_handle);

    const SpriteOffset driver_offset = { 3, -17 };
    driver->position = self->position;
    sa_apply_offset_flip(&driver_offset, &driver->position, self->direction);

    driver->direction = self->direction;
}

static void driver_init(SpriteActor *tank) {
    SpriteActor *driver = tank_driver_sprite_init(&tank->position);
    tank->tank.driver_enemy_handle = driver->handle;
}

SpriteActor *tank_sprite_init(const SpritePosition *position, bool include_driver) {
    SpriteActor *actor = sa_alloc();
    sa_init(actor, tank_sprite_main);

    actor->position = *position;
    actor->direction = LEFT;

    actor->falls_off_when_killed = false;
    actor->dies_upon_stomp = false;
    actor->touch_hurts_hero = false;
    actor->stompable = false;
    actor->immune_to_projectiles = true;
    actor->interacts_with_sprites = false;

    static const SpriteBoundingBox sprite_box = {
        .offset = { -12, -16 },
        .size = { 24, 16 }
    };
    actor->bounding_box = sprite_box;

    TankSprite *sub = &actor->tank;
    sub->hero_collision_grace = 0;

    sub->firing = false;
    sub->launch_pending = false;
    sub->launch_counter = 0;
    sub->missiles_launched = 0;
    sub->driver_enemy_handle = SA_HANDLE_FREE;
    sub->current_state_duration = 0;
    sub->exahust_sprite_acc = 0;
    sub->animation_acc = 0;

    if (include_driver) {
        driver_init(actor);
        sub->enemy_driver_state = TANK_STATE_PATROL_ACCEL;
    }

    return actor;
}

static void draw(const SpriteActor *self, TankSprite *sub, const SpriteEnvironment *env) {
    static const SpriteDrawParams body_params[2][2] = {
        {
            {
                .tile = 0x102,
                .offset_x = -12, .offset_y = -17,
                .palette = 12,
                .wide = true, .tall = true,
                .x_flip = false, .y_flip = false
            },
            {
                .tile = 0x103,
                .offset_x = -12 + 8, .offset_y = -17,
                .palette = 12,
                .wide = true, .tall = true,
                .x_flip = false, .y_flip = false
            },
        },
        {
            {
                .tile = 0x10a,
                .offset_x = -12, .offset_y = -17,
                .palette = 12,
                .wide = true, .tall = true,
                .x_flip = false, .y_flip = false
            },
            {
                .tile = 0x10b,
                .offset_x = -12 + 8, .offset_y = -17,
                .palette = 12,
                .wide = true, .tall = true,
                .x_flip = false, .y_flip = false
            }
        }
    };

    static const SpriteDrawParams params[] = {
        // Bottom (tracks)
        {
            .tile = 0x105,
            .offset_x = -12 + 8, .offset_y = -17 + 16,
            .palette = 12,
            .wide = false, .tall = false,
            .x_flip = false, .y_flip = false
        },
        {
            .tile = 0x105,
            .offset_x = -12 + 0, .offset_y = -17 + 16,
            .palette = 12,
            .wide = false, .tall = false,
            .x_flip = false, .y_flip = false
        },
        {
            .tile = 0x117,
            .offset_x = -12 + 8, .offset_y = -17 + 16,
            .palette = 12,
            .wide = false, .tall = false,
            .x_flip = false, .y_flip = false
        },
        {
            .tile = 0x115,
            .offset_x = -12 + 16, .offset_y = -17 + 16,
            .palette = 12,
            .wide = false, .tall = false,
            .x_flip = false, .y_flip = false
        },
        // Exhaust
        {
            .tile = 0x107,
            .offset_x = -12 + 24, .offset_y = -17 + 12,
            .palette = 12,
            .wide = false, .tall = false,
            .x_flip = false, .y_flip = false
        },
        {
            .tile = 0x116,
            .offset_x = -12 + 24, .offset_y = -17 + 4,
            .palette = 12,
            .wide = false, .tall = false,
            .x_flip = false, .y_flip = false
        },
        {
            .tile = 0x106,
            .offset_x = -12 + 24, .offset_y = -17 - 4,
            .palette = 12,
            .wide = false, .tall = false,
            .x_flip = false, .y_flip = false
        }
    };

    int32_t speed = speed_x(self, env->hero);
    uint32_t animation_acc = sub->animation_acc;
    animation_acc += speed / (Q_1 / 32);
    if (animation_acc > 64) {
        animation_acc -= 64;
        sub->animation_index++;
        sub->animation_index %= 2;
    }
    sub->animation_acc =animation_acc;

    const SpriteDrawParams *body = body_params[sub->animation_index];
    sa_draw_standard_multiple(self, env, body, SPRITE_DRAW_PARAM_COUNT(body_params[0]));
    sa_draw_standard_multiple(self, env, params, SPRITE_DRAW_PARAM_COUNT(params));
}

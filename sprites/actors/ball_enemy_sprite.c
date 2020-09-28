#include "ball_enemy_sprite.h"

#include "sprite_actor.h"

#include "vdp.h"
#include "vram_layout.h"
#include "camera.h"
#include "sprite_block_interaction.h"
#include "sprite_collision.h"
#include "sprite_drawing.h"
#include "hero.h"
#include "math_util.h"
#include "sound_effects.h"

#include "debug_print.h"

static void move(SpriteActor *self, const Hero *hero);
static void draw(SpriteActor *self, const SpriteEnvironment *env);

void ball_enemy_sprite_main(SpriteActor *self, const SpriteEnvironment *env) {
    BallEnemy *sub = &self->ball_enemy;

    move(self, env->hero);

    // This is important to stop hero getting hurt when falling onto the ball
    const uint8_t hero_collision_grace_default = 9;

    if (sub->hero_collision_grace) {
        sub->hero_collision_grace--;
    }
    bool suppress_hero_collision = (sub->hero_collision_grace > 0);

    if (!self->killed) {
        BallEnemyState state_prev = sub->state;

        const int32_t roll_speed = Q_1 * 2;
        bool set_rolling_velocity = false;

        switch (sub->state) {
            case BALL_ENEMY_WALKING: {
                const int32_t walk_speed = Q_1 / 2;

                self->velocity.x = sa_velocity_from_speed(walk_speed, self->direction);

                if (sa_hero_standard_collision(self, env->hero) == SA_HERO_COLLISION_STOMP) {
                    sub->state = BALL_ENEMY_CURLED;
                }
            } break;
            case BALL_ENEMY_ROLLING: {
                if (!suppress_hero_collision && sa_hero_standard_collision(self, env->hero) == SA_HERO_COLLISION_STOMP) {
                    self->velocity.x = 0;
                    sub->state = BALL_ENEMY_CURLED;
                } else {
                    set_rolling_velocity = true;
                }
            } break;
            case BALL_ENEMY_CURLED: {
                if (!suppress_hero_collision) {
                    // This carry interaction block may be useful to other carry sprites
                    // Could extract if other sprites requirements end up being similar enough
                    SpriteHeroCollisionResult result = sa_hero_standard_collision(self, env->hero);
                    
                    switch (result) {
                        case SA_HERO_COLLISION_CARRIED_KICK:
                            sub->extra_kick_speed = ABS(env->hero->velocity.x);
                            sub->state = BALL_ENEMY_ROLLING;

                            set_rolling_velocity = true;
                            break;
                        case SA_HERO_COLLISION_CARRIED_KICK_UP: {
                            const int32_t kick_up_speed = Q_1 * 9;

                            self->velocity.x = env->hero->velocity.x;
                            self->velocity.y = -kick_up_speed;

                            sub->hero_collision_grace = hero_collision_grace_default;
                        } break;
                        case SA_HERO_COLLISION_INTERSECT: {
                            const Hero *hero = env->hero;
                            impact_sprite_init(&hero->position);
                            se_stomp();

                            sub->state = BALL_ENEMY_ROLLING;

                            bool on_left = hero->position.x > self->position.x;
                            self->direction = on_left ? LEFT : RIGHT;
                        } break;
                        default:
                            break;
                    }
                }
            } break;
        }

        if (set_rolling_velocity) {
            uint32_t speed = roll_speed + sub->extra_kick_speed;
            self->velocity.x = sa_velocity_from_speed(speed, self->direction);
        }

        self->kills_other_sprites = (sub->state == BALL_ENEMY_ROLLING);
        self->kills_other_sprites |= (sub->state == BALL_ENEMY_CURLED) && (self->velocity.y != 0);

        self->can_be_carried = (sub->state == BALL_ENEMY_CURLED);
        self->touch_hurts_hero = (sub->state != BALL_ENEMY_CURLED);
        self->stompable = (sub->state != BALL_ENEMY_CURLED);
        self->thud_sound_upon_hitting_wall = (sub->state == BALL_ENEMY_ROLLING);

        if (state_prev != sub->state) {
            sub->hero_collision_grace = hero_collision_grace_default;
            sub->ball_animation_index = 0;
        }

        sa_other_sprite_collision(self);
    }

    draw(self, env);
}

static void move(SpriteActor *self, const Hero *hero) {
    if (sa_perform_default_movement(self) || self->killed) {
        return;
    }
    if (sa_perform_carryable_movement(self, hero)) {
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

    sa_apply_velocity_with_gravity(self, &vertical_box);
    sa_apply_horizontal_block_interaction_updates(self, &horizontal_box);
}

static void draw_ball(SpriteActor *self, const SpriteEnvironment *env);
static void draw_walking(SpriteActor *self, const SpriteEnvironment *env);

static void draw(SpriteActor *self, const SpriteEnvironment *env) {
    BallEnemy *sub = &self->ball_enemy;

    sub->frame_counter++;

    if (sub->state == BALL_ENEMY_WALKING) {
        draw_walking(self, env);
    } else {
        draw_ball(self, env);
    }
}

static void draw_ball(SpriteActor *self, const SpriteEnvironment *env) {
    BallEnemy *sub = &self->ball_enemy;

    if (sub->state != BALL_ENEMY_WALKING) {
        const uint8_t frame_duration = 32;
        
        sub->ball_animation_fraction += ABS(self->velocity.x) / (Q_1 / 8);
        if (sub->ball_animation_fraction > frame_duration) {
            sub->ball_animation_index++;
            sub->ball_animation_fraction = 0;
        }

        if (sub->ball_animation_index > 2) {
            sub->ball_animation_index = 0;
        }
    }

    static const uint16_t ball_tiles[] = {0x08a, 0x0a6, 0x0aa};

    SpriteDrawParams params = {
        .offset_x = -8,
        .offset_y = -17,
        .palette = 10,
        .tile = ball_tiles[sub->ball_animation_index],
        .wide = true, .tall = true,
        .x_flip = true, .y_flip = false
    };

    sa_draw_standard(self, env, &params);
}

static void draw_walking(SpriteActor *self, const SpriteEnvironment *env) {
    const uint16_t tile_killed = 0x0ac;
    static const uint16_t walking_tiles[] = {0x0ac, 0x0ae};

    uint16_t tile;
    if (!self->killed) {
        tile = walking_tiles[(self->ball_enemy.frame_counter / 8) % 2];
    } else {
        tile = tile_killed;
    }

    const uint8_t palette = 10;
    sa_draw_standard_16x16(self, env, tile, palette);
}

SpriteActor *ball_enemy_sprite_init(const SpritePosition *position, bool walking) {
    SpriteActor *actor = sa_alloc();
    sa_init(actor, ball_enemy_sprite_main);

    actor->position = *position;
    actor->direction = RIGHT;

    actor->falls_off_when_killed = false;
    actor->dies_upon_stomp = false;
    actor->can_ride = true;

    static const SpriteBoundingBox sprite_box = {
        .offset = { -8, -15 },
        .size = { 16, 15 }
    };
    actor->bounding_box = sprite_box;

    BallEnemy *sub = &actor->ball_enemy;
    sub->state = walking ? BALL_ENEMY_WALKING : BALL_ENEMY_CURLED;
    sub->frame_counter = 0;
    sub->ball_animation_fraction = 0;
    sub->hero_collision_grace = 0;
    sub->extra_kick_speed = 0;

    return actor;
}

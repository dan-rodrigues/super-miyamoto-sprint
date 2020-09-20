#ifndef ball_enemy_sprite_h
#define ball_enemy_sprite_h

#include <stdint.h>
#include <stdbool.h>

#include "sprite_actor_types.h"
#include "sprite_position.h"

typedef enum {
    BALL_ENEMY_WALKING,
    BALL_ENEMY_CURLED,
    BALL_ENEMY_ROLLING
} BallEnemyState;

typedef struct {
    BallEnemyState state;
    uint8_t frame_counter;
    uint8_t ball_animation_fraction;
    uint8_t ball_animation_index;
    uint8_t hero_collision_grace;
    uint32_t extra_kick_speed;
} BallEnemy;

void ball_enemy_sprite_main(SpriteActor *self, const SpriteEnvironment *env);
SpriteActor *ball_enemy_sprite_init(const SpritePosition *position, bool walking);

#endif /* ball_enemy_sprite_h */

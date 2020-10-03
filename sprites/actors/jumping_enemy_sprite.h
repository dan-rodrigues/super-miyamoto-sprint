#ifndef jumping_enemy_sprite_h
#define jumping_enemy_sprite_h

#include <stdint.h>
#include <stdbool.h>

#include "sprite_actor_types.h"
#include "sprite_position.h"

typedef struct {
    bool grounded;
    uint8_t jump_timer;
} JumpingEnemySprite;

void jumping_enemy_sprite_main(SpriteActor *self, const SpriteEnvironment *env);
SpriteActor *jumping_enemy_sprite_init(const SpritePosition *position);

#endif /* jumping_enemy_sprite_h */

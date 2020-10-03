#ifndef jetpack_enemy_sprite_h
#define jetpack_enemy_sprite_h

#include <stdbool.h>
#include <stdint.h>

#include "sprite_actor_types.h"
#include "sprite_position.h"

typedef struct {
    bool grounded;
    bool boosting;
} JetpackEnemySprite;

void jetpack_enemy_sprite_main(SpriteActor *self, const SpriteEnvironment *env);
SpriteActor *jetpack_enemy_sprite_init(const SpritePosition *position);

#endif /* jetpack_enemy_sprite_h */

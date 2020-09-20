#ifndef layered_enemy_sprite_h
#define layered_enemy_sprite_h

#include <stdint.h>
#include <stdbool.h>

#include "sprite_actor_types.h"
#include "sprite_position.h"

typedef struct {
    bool small;
    uint8_t frame_counter;
} LayeredEnemy;

void layered_enemy_sprite_main(SpriteActor *self, const SpriteEnvironment *env);
SpriteActor *layered_enemy_sprite_init(const SpritePosition *position);

#endif /* layered_enemy_sprite_h */

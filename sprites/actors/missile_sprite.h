#ifndef missile_sprite_h
#define missile_sprite_h

#include <stdint.h>

#include "sprite_position.h"
#include "sprite_actor_types.h"

typedef struct {
    uint32_t index_mod_4;
} MissileSprite;

void missile_sprite_main(SpriteActorLight *self, const SpriteEnvironment *env);
SpriteActorLight *missile_sprite_init(const SpritePosition *position);

#endif /* missile_sprite_h */

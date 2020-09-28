#ifndef moving_smoke_sprite_h
#define moving_smoke_sprite_h

#include <stdint.h>

#include "sprite_position.h"
#include "sprite_actor_types.h"

typedef struct {
    uint8_t life_counter;
} MovingSmokeSprite;

void moving_smoke_sprite_main(SpriteActorLight *self, const SpriteEnvironment *env);
SpriteActorLight *moving_smoke_sprite_init(SpritePosition *position);

#endif /* moving_smoke_sprite_h */

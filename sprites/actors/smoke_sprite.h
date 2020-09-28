#ifndef smoke_sprite_h
#define smoke_sprite_h

#include <stdint.h>

#include "sprite_position.h"
#include "sprite_actor_types.h"

typedef enum {
    SMOKE_SMALL,
    SMOKE_LARGE
} SmokeSpriteSize;

typedef struct {
    uint8_t life_counter;
    SmokeSpriteSize size;
} SmokeSprite;

void smoke_sprite_main(SpriteActorLight *self, const SpriteEnvironment *env);
SpriteActorLight *smoke_sprite_init(SpritePosition *position, SmokeSpriteSize size);

#endif /* smoke_sprite_h */

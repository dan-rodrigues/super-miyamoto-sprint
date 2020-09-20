#ifndef impact_sprite_h
#define impact_sprite_h

#include <stdint.h>

#include "sprite_position.h"
#include "sprite_actor_types.h"

typedef struct {
    uint8_t lifetime;
} ImpactSprite;

void impact_sprite_main(SpriteActorLight *self, const SpriteEnvironment *env);
SpriteActorLight *impact_sprite_init(const SpritePosition *position);

#endif /* impact_sprite_h */

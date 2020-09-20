#ifndef glitter_sprite_h
#define glitter_sprite_h

#include <stdint.h>

#include "sprite_position.h"
#include "sprite_actor_types.h"

typedef struct {
    uint8_t lifetime;
} GlitterSprite;

void glitter_sprite_main(SpriteActorLight *self, const SpriteEnvironment *env);
SpriteActorLight *glitter_sprite_init(const SpritePosition *position);

#endif /* glitter_sprite_h */

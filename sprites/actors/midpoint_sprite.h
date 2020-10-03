#ifndef midpoint_sprite_h
#define midpoint_sprite_h

#include <stdint.h>

#include "sprite_position.h"
#include "sprite_actor_types.h"

typedef struct {
    // ...
} MidpointSprite;

void midpoint_sprite_main(SpriteActor *self, const SpriteEnvironment *env);
SpriteActor *midpoint_sprite_init(const SpritePosition *position);

#endif /* midpoint_sprite_h */

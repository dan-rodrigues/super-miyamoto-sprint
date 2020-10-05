#ifndef tank_driver_sprite_h
#define tank_driver_sprite_h

#include <stdint.h>

#include "sprite_actor_types.h"
#include "sprite_position.h"

typedef struct {
    // ...
} TankDriverSprite;

void tank_driver_sprite_main(SpriteActor *self, const SpriteEnvironment *env);
SpriteActor *tank_driver_sprite_init(const SpritePosition *position);

#endif /* tank_driver_sprite_h */

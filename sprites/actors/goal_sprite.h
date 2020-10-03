#ifndef goal_sprite_h
#define goal_sprite_h

#include <stdint.h>

#include "sprite_position.h"
#include "sprite_actor_types.h"

typedef struct {
    // ...
} GoalSprite;

void goal_sprite_main(SpriteActor *self, const SpriteEnvironment *env);
SpriteActor *goal_sprite_init(const SpritePosition *position);

#endif /* goal_sprite_h */

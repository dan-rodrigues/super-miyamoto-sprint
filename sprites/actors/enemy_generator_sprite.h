#ifndef enemy_generator_sprite_h
#define enemy_generator_sprite_h

#include <stdint.h>

#include "sprite_position.h"
#include "sprite_actor_types.h"

typedef struct {
    uint16_t spawn_counter;
    bool spawn_toggle;
} EnemyGeneratorSprite;

void enemy_generator_sprite_main(SpriteActor *self, const SpriteEnvironment *env);
SpriteActor *enemy_generator_sprite_init(SpritePosition *position);

#endif /* enemy_generator_sprite_h */

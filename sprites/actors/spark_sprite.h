#ifndef spark_sprite_h
#define spark_sprite_h

#include <stdint.h>

#include "sprite_position.h"
#include "sprite_actor_types.h"

typedef struct {
    uint32_t life_counter;
} SparkSprite;

void spark_sprite_main(SpriteActorLight *self, const SpriteEnvironment *env);
SpriteActorLight *spark_sprite_init(SpritePosition *position);

#endif /* spark_sprite_h */

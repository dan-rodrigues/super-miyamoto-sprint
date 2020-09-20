#ifndef basic_enemy_sprite_h
#define basic_enemy_sprite_h

#include <stdint.h>

#include "sprite_position.h"
#include "sprite_actor_types.h"

typedef struct {
    uint32_t frame_counter;
    uint8_t expiry_counter;
    bool spiked;
    bool squished;
} BasicEnemy;

void basic_enemy_sprite_main(SpriteActor *self, const SpriteEnvironment *env);
SpriteActor *basic_enemy_sprite_init(const SpritePosition *position, bool spiked);

#endif /* basic_enemy_sprite_h */

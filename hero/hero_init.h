#ifndef hero_init_h
#define hero_init_h

#include <stdint.h>
#include <stdbool.h>

#include "hero_types.h"

#include "level_attributes.h"

void hero_level_init(Hero *hero,
                     const LevelAttributes *attributes,
                     uint8_t max_life,
                     bool midpoint_position);

#endif /* hero_init_h */

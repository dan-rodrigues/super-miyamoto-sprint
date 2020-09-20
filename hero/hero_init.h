#ifndef hero_init_h
#define hero_init_h

#include <stdint.h>

#include "hero_types.h"

#include "level_attributes.h"

void hero_level_init(Hero *hero, const LevelAttributes *attributes);

#endif /* hero_init_h */

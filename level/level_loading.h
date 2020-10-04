#ifndef level_loading_h
#define level_loading_h

#include <stdint.h>

#include "level_attributes.h"
#include "game_loop_types.h"

void level_init(const LevelAttributes *attributes, GameContext *context);
void level_load(const LevelAttributes *attributes);

#endif /* level_loading_h */

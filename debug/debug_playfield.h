#ifndef debug_playfield_h
#define debug_playfield_h

#include "game_loop_types.h"
#include "hero_types.h"
#include "camera_types.h"

void dbg_init_playfield(GameContext *context);
void dbg_frame_action(GameContext *context);

void dbg_spawn_platform(const Hero *hero);

void dbg_print_sandbox_instructions(void);

#endif /* debug_playfield_h */

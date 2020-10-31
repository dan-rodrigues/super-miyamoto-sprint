#ifndef title_loop_h
#define title_loop_h

#include "game_loop_types.h"

void title_loop_init(GameContext *context);
GameLoopAction title_step_frame(GameContext *context);
void title_frame_ended_update(GameContext *context);

#endif /* title_loop_h */

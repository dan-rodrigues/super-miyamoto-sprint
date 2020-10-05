#ifndef gameplay_loop_h
#define gameplay_loop_h

#include "game_loop.h"

GameLoopAction gameplay_step_frame(GameContext *context);
void gameplay_frame_ended_update(GameContext *context);

#endif /* gameplay_loop_h */

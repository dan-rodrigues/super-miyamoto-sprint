#ifndef credits_loop_h
#define credits_loop_h

#include "game_loop_types.h"

void credits_loop_init(GameContext *context);
GameLoopAction credits_step_frame(GameContext *context);
void credits_frame_ended_update(CreditsState state);

#endif /* credits_loop_h */

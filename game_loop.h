#ifndef game_loop_h
#define game_loop_h

#include <stdbool.h>

#include "game_loop_types.h"
#include "hero_types.h"
#include "camera_types.h"

enum GameLoopAction {
    GL_ACTION_NONE = 0,

    // Debug only:
    GL_ACTION_RESET_WORLD,
    GL_ACTION_SPAWN_SPRITES
};

struct PlayerContext {
    Hero *hero;
    Camera *camera;
    bool active;
};

struct GameContext {
    PlayerContext players[2];
    bool paused;
};

GameLoopAction gl_run_frame(GameContext *context);

#endif /* game_loop_h */

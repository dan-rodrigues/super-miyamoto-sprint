#ifndef game_loop_h
#define game_loop_h

#include <stdbool.h>
#include <stdint.h>

#include "game_loop_types.h"
#include "hero_types.h"
#include "camera_types.h"
#include "extra_task_handle.h"

enum GameLoopAction {
    GL_ACTION_NONE = 0,
    GL_ACTION_RESET_WORLD,
    GL_ACTION_RELOAD_LEVEL,

    // Debug only:
    GL_ACTION_SPAWN_SPRITES
};

struct PlayerContext {
    Hero *hero;
    Camera *camera;
    bool active;

    uint8_t max_life;
    bool midpoint_reached;
};

struct GameContext {
    PlayerContext players[2];
    bool paused;

    ExtraTaskHandle current_fade_handle;
};

GameLoopAction gl_run_frame(GameContext *context);
void gl_reset_context(GameContext *context, Hero *hero, Camera *camera);

#endif /* game_loop_h */

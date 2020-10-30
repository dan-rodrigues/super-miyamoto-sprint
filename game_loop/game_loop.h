#ifndef game_loop_h
#define game_loop_h

#include <stdbool.h>
#include <stdint.h>

#include "game_loop_types.h"
#include "hero_types.h"
#include "camera_types.h"
#include "extra_task_handle.h"
#include "affine.h"
#include "level_attributes.h"
#include "sprite_loading_types.h"

enum CreditsState {
    CREDITS_STATE_INITIAL_DELAY,

    CREDITS_STATE_FADE_IN_TECH,
    CREDITS_STATE_DISPLAYING_TECH,
    CREDITS_STATE_FADE_OUT_TECH,
    CREDITS_STATE_FADE_IN_ART,
    CREDITS_STATE_DISPLAYING_ART,
    CREDITS_STATE_FADE_OUT_ART,

    CREDITS_STATE_FADE_OUT_TEXT,
    CREDITS_STATE_FADE_OUT_HERO,
    CREDITS_STATE_FADE_OUT_MUSIC
};

enum TitleState {
    TITLE_STATE_INITIAL_DELAY,
    TITLE_STATE_INITIAL_FADE_IN,
    TITLE_STATE_DISPLAYING_TITLE,
    TITLE_STATE_DISPLAYING_MENU,
    TITLE_STATE_FADE_OUT
};

enum GameLoop {
    GL_TITLE,
    GL_GAMEPLAY,
    GL_CREDITS
};

enum GameLoopAction {
    GL_ACTION_NONE = 0,
    GL_ACTION_SHOW_TITLE,
    GL_ACTION_RESET_WORLD,
    GL_ACTION_RELOAD_LEVEL,
    GL_ACTION_SHOW_CREDITS
};

struct CreditsContext {
    AffineHeroContext affine_context;
    CreditsState state;
    uint16_t state_counter;
    bool showing_tech_attributions;
};

struct TitleContext {
    TitleState state;
    uint16_t state_counter;
    uint8_t presentation_delay;
    int16_t scale;
    uint8_t selected_menu_option;
    bool exiting;
};

struct PlayerContext {
    Hero *hero;
    Camera *camera;
    SpriteLoadingContext *sprite_context;
    bool active;

    uint8_t max_life;
    bool midpoint_reached;
};

struct GameContext {
    // 1P only for now
    PlayerContext players[1];

    union {
        CreditsContext credits;
        TitleContext title;
    };

    GameLoop game_loop;
    bool paused;
    uint8_t level;

    ExtraTaskHandle current_fade_handle;
};

void gl_reset_context(GameContext *context,
                      Hero *hero,
                      Camera *camera,
                      SpriteLoadingContext *sprite_context,
                      uint8_t level);

void gl_load_title(GameContext *context);
void gl_load_credits(GameContext *context);

GameLoopAction gl_run_frame(GameContext *context);
bool gl_fading(const GameContext *context);

#endif /* game_loop_h */

// main.c
//
// Copyright (C) 2020 Dan Rodrigues <danrr.gh.oss@gmail.com>
//
// SPDX-License-Identifier: MIT

#include <stdint.h>
#include <stdbool.h>

#include "vdp.h"
#include "assert.h"

#include "game_loop.h"
#include "affine.h"
#include "music.h"
#include "camera.h"
#include "camera_init.h"
#include "level_loading.h"
#include "hero.h"
#include "debug_playfield.h"
#include "debug_custom_assert.h"
#include "fade_task.h"
#include "sprite_loading.h"

static void handle_gl_action(GameLoopAction action, GameContext *context);

int main() {
    vdp_enable_copper(false);
    assert_set_handler(custom_assert_handler);

    vdp_enable_layers(0);
    vdp_set_single_palette_color(0, 0x0000);

    GameContext context;

    Hero hero;
    Camera camera;
    SpriteLoadingContext sprite_loading_context;

    gl_reset_context(&context, &hero, &camera, &sprite_loading_context);

    handle_gl_action(GL_ACTION_RESET_WORLD, &context);
//    handle_gl_action(GL_ACTION_SHOW_CREDITS, &context);

    music_start();
    
    while (true) {
        GameLoopAction action = gl_run_frame(&context);
        handle_gl_action(action, &context);
    }

    fatal_error("Unexpected exit of game loop");
}

static void handle_gl_action(GameLoopAction action, GameContext *context) {
    const PlayerContext *p1 = &context->players[0];
    const LevelAttributes *level = level_attributes(0);

    switch (action) {
        case GL_ACTION_RESET_WORLD:
            gl_reset_context(context, p1->hero, p1->camera, p1->sprite_context);
            level_init(level, context);
            break;
        case GL_ACTION_RELOAD_LEVEL:
            level_init(level, context);
            break;
        case GL_ACTION_SHOW_CREDITS:
            gl_load_credits(level, context);
            break;
        case GL_ACTION_NONE:
            break;
        default:
            fatal_error("Unexpected game loop action");
    }
}

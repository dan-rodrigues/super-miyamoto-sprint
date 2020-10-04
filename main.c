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

#include "music.h"
#include "camera.h"
#include "camera_init.h"
#include "level_loading.h"

#include "hero.h"

#include "debug_playfield.h"
#include "debug_custom_assert.h"

static void handle_gl_action(GameLoopAction action, GameContext *context);

int main() {
    assert_set_handler(custom_assert_handler);

    vdp_enable_layers(0);
    vdp_set_single_palette_color(0, 0x0000);

    Hero hero;
    Camera camera;
    GameContext context;
    gl_reset_context(&context, &hero, &camera);

    handle_gl_action(GL_ACTION_RESET_WORLD, &context);

    music_start();
    
    while (true) {
        GameLoopAction action = gl_run_frame(&context);
        handle_gl_action(action, &context);
    }

    fatal_error("Unexpected exit of game loop");
}

static void handle_gl_action(GameLoopAction action, GameContext *context) {
    switch (action) {
        case GL_ACTION_RESET_WORLD:
            gl_reset_context(context, context->players[0].hero, context->players[0].camera);
            level_init(level_attributes(0), context);
            break;
        case GL_ACTION_RELOAD_LEVEL:
            context->resetting = false;
            level_init(level_attributes(0), context);
            break;
        case GL_ACTION_SPAWN_SPRITES:
            dbg_reset_sprites(context->players[0].hero);
            break;
        case GL_ACTION_NONE:
            break;
        default:
            fatal_error("Unexpected game loop action");
    }
}

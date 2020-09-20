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

#include "hero.h"

#include "debug_playfield.h"
#include "debug_custom_assert.h"

int main() {
    assert_set_handler(custom_assert_handler);

    Hero hero;
    Camera camera;

    // Only single player for now

    GameContext context = {
        .players = {
            {.hero = &hero, .camera = &camera, .active = true},
            {.active = false}
        },
        .paused = false
    };

    dbg_init_playfield(&context);

    music_start();
    
    while (true) {
        GameLoopAction action = gl_run_frame(&context);

        switch (action) {
            case GL_ACTION_RESET_WORLD:
                dbg_init_playfield(&context);
                break;
            case GL_ACTION_SPAWN_SPRITES:
                dbg_reset_sprites(context.players[0].hero);
                break;
            case GL_ACTION_NONE:
                break;
            default:
                fatal_error("Unexpected game loop action");
        }
    }

    fatal_error("Unexpected exit of game loop");
}

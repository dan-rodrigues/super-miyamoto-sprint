#include "game_loop.h"
#include "assert.h"
#include "gamepad.h"
#include "vdp_print.h"
#include "vram_command_queue.h"
#include "palette_buffer.h"
#include "music.h"
#include "camera.h"
#include "camera_init.h"
#include "level_loading.h"
#include "sprite_text.h"
#include "hero.h"
#include "debug_playfield.h"
#include "audio_command_queue.h"
#include "global_timers.h"
#include "extra_task.h"
#include "affine.h"
#include "gameplay_loop.h"
#include "credits_loop.h"
#include "title_loop.h"

GameLoopAction gl_run_frame(GameContext *context) {
    const bool show_raster_counter = false;

    PlayerContext *p1 = &context->players[0];
    assert(p1->active);

    Hero *hero = p1->hero;

    pad_read(&hero->pad.raw, NULL, &hero->pad_edge.raw, NULL);
    pad_decode_input(hero->pad.raw, &hero->pad);
    pad_decode_input(hero->pad_edge.raw, &hero->pad_edge);

    // Main updates for this frame

    GameLoopAction result_action = GL_ACTION_NONE;

    switch (context->game_loop) {
        case GL_GAMEPLAY: {
            dbg_frame_action(context);
            
            if (hero->pad_edge.start && !gl_fading(context)) {
                context->paused = !context->paused;
                
                uint8_t alpha = context->paused ? 0x8 : 0xf;
                pb_alpha_mask_all(alpha, true);
            }
            
            if (!context->paused) {
                result_action = gameplay_step_frame(context);

                if (show_raster_counter) {
                    st_write_hex(VDP_CURRENT_RASTER_Y, 64, 8);
                }
            }
        } break;
        case GL_CREDITS: {
            result_action = credits_step_frame(context);

            if (show_raster_counter) {
                st_write_hex(VDP_CURRENT_RASTER_Y, 64, 8);
            }
        } break;
        case GL_TITLE:
            result_action = title_step_frame(context);
            break;
    }

    // Finished entire frame worth of processing hopefully before the cutoff line
    // If this isn't reached before the cutoff then user will notice slowdown
    vdp_wait_frame_ended();
    vdp_enable_copper(false);

    // Offscreen updates this frame

    switch (context->game_loop) {
        case GL_GAMEPLAY:
            if (!context->paused) {
                // Sprites are preserved when paused, otherwise they need redrawing every frame
                sb_upload();
            }

            gameplay_frame_ended_update(context);
            vcq_run();
            break;
        case GL_CREDITS:
            credits_frame_ended_update(context->credits.state);
            sb_upload();
            break;
        case GL_TITLE:
            title_frame_ended_update(context);
            sb_upload();
            break;
    }

    // Palette RAM always updated even when paused (for the fade effect)
    pb_upload();
    acq_run();

    // Save the bits of hero state that need to persist between level loads
    p1->midpoint_reached = hero->midpoint_reached;
    p1->max_life = hero->max_life;

    gt_tick();

    return result_action;
}

void gl_reset_context(GameContext *context,
                      Hero *hero,
                      Camera *camera,
                      SpriteLoadingContext *sprite_context,
                      uint8_t level)
{
    const GameContext context_initialized = {
        .players = {
            {
                // P1 (only single player for now)
                .hero = hero,
                .camera = camera,
                .active = true,
                .midpoint_reached = false,
                .max_life = HERO_DEFAULT_LIFE,
                .sprite_context = sprite_context
            }
        },
        .game_loop = GL_GAMEPLAY,
        .paused = false,
        .level = level,

        .current_fade_handle = ET_HANDLE_FREE
    };

    *context = context_initialized;
}

void gl_load_credits(GameContext *context) {
    const LevelAttributes *base_level = level_attributes(0);
    level_load(base_level);
    credits_loop_init(context);
    music_start(TRACK_CREDITS);
}

void gl_load_title(GameContext *context) {
    const LevelAttributes *base_level = level_attributes(0);
    level_load(base_level);
    title_loop_init(context);
}

bool gl_fading(const GameContext *context) {
    return et_handle_live(context->current_fade_handle);
}

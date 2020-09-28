#include "game_loop.h"
#include "assert.h"

#include "gamepad.h"
#include "vdp_print.h"

#include "vram_command_queue.h"
#include "vram_level_update.h"
#include "vram_layout.h"
#include "vram_animated_tiles.h"

#include "palette_buffer.h"

#include "music.h"
#include "camera.h"
#include "camera_init.h"
#include "level_loading.h"
#include "sprite_loading.h"
#include "sprite_text.h"

#include "hero.h"
#include "hero_drawing.h"
#include "hero_init.h"
#include "hero_life_meter.h"

#include "debug_block.h"
#include "debug_print.h"
#include "debug_playfield.h"

#include "block_map_table.h"

#include "audio_command_queue.h"

#include "global_timers.h"

static void step_frame(Hero *hero, Camera *camera);

GameLoopAction gl_run_frame(GameContext *context) {
    PlayerContext *p1 = &context->players[0];
    assert(p1->active);

    Hero *hero = p1->hero;
    Camera *camera = p1->camera;

    pad_read(&hero->pad.raw, NULL, &hero->pad_edge.raw, NULL);
    pad_decode_input(hero->pad.raw, &hero->pad);
    pad_decode_input(hero->pad_edge.raw, &hero->pad_edge);

    GameLoopAction debug_action = dbg_frame_action(context);
    if (debug_action) {
        return debug_action;
    }

    if (hero->pad_edge.start) {
        context->paused = !context->paused;

        uint8_t alpha = context->paused ? 0x8 : 0xf;
        pb_alpha_mask_all(alpha);
    }

    // (later: copper updates if applicable)

    if (!context->paused) {
        step_frame(hero, camera);
    }

    // Simple "CPU meter" which just prints the value of the raster counter
    st_write_hex(VDP_CURRENT_RASTER_Y, 320, 50);

    // Finished entire frame worth of processing
    vdp_wait_frame_ended();

    // Palette RAM always updated even when paused (for the fade effect)
    pb_upload();

    // Other video memories only updated during active gameplay
    if (!context->paused) {
        vcq_run();
        sb_upload();
        camera_apply_vram_update(camera, SCROLL_MAP_BASE, block_map_table);
        camera_apply_scroll(camera);
        acq_run();
    }
    
    return GL_ACTION_NONE;
}

static void step_frame(Hero *hero, Camera *camera) {
    // (may later replace with just GameContext)
    SpriteEnvironment sprite_draw_context = {
        .camera = camera,
        .hero = hero
    };

    // 1. Run rideable sprites before anything else (including Hero)
    sa_run_rideable(&sprite_draw_context);

    // 2. Camera follows hero so update hero first, then track with camera
    hero_update_state(hero, camera);
    camera_update(camera, hero);

    sa_run_deferred_draw_tasks(&sprite_draw_context);

    // Sprite loading, if needed
    sprite_level_data_load_new(camera, hero);

    // Note camera must be updated before any sprite drawing happens
    // There's "jitter" otherwise as camera / sprites don't appear in sync
    hero_draw_draw_coin_counter(hero);
    hero_draw_life_meter(hero);
    draw_hero_sprites(hero, 0, camera);

    // 3. Non-rideable sprites run after hero as hero's position isn't influenced by them
    sa_run_non_rideable(&sprite_draw_context);

    // 4. Light sprite actors run last and they cannot alter the position of regular sprite actors
    sa_run_light(&sprite_draw_context);

    // VRAM animation queueing (auto-animated blocks such as muncher plants and lava)
    vram_queue_animations();

    dbg_print_sandbox_instructions();
    dbg_print_status(TEXT_PALETTE_ID, hero, camera);

    gt_tick();
}

#include "gameplay_loop.h"

#include "vdp.h"

#include "hero.h"
#include "hero_life_meter.h"
#include "hero_drawing.h"
#include "debug_print.h"
#include "vram_animated_tiles.h"
#include "extra_task.h"
#include "sprite_actor.h"
#include "global_timers.h"
#include "vram_layout.h"
#include "sprite_loading.h"
#include "camera.h"
#include "debug_playfield.h"
#include "block_map_table.h"
#include "vram_command_queue.h"
#include "sprite_loading_types.h"

static void enable_display(bool alpha_enabled, bool alpha_enable_sprites);
static void end_state_check(GameContext *context);

GameLoopAction gameplay_step_frame(GameContext *context) {
    const PlayerContext *p1 = &context->players[0];
    Hero *hero = p1->hero;
    Camera *camera = p1->camera;

    const SpriteEnvironment sprite_draw_context = {
        .camera = camera,
        .hero = hero,
        .loading_context = p1->sprite_context
    };

    // 1. Run rideable sprites before anything else (including Hero)
    sa_run_rideable(&sprite_draw_context);

    // 2. Camera follows hero so update hero first, then track with camera
    hero_update_state(hero, camera);
    camera_update(camera, hero);

    sa_run_deferred_draw_tasks(&sprite_draw_context);

    // Sprite loading, if needed
    sprite_level_data_load_new(p1->sprite_context, camera, hero);

    // Note camera must be updated before any sprite drawing happens
    // There's "jitter" otherwise as camera / sprites don't appear in sync
    hero_draw_draw_coin_counter(hero);
    hero_draw_life_meter(hero);
    hero_draw(hero, 0, camera);

    // 3. Non-rideable sprites run after hero as hero's position isn't influenced by them
    sa_run_non_rideable(&sprite_draw_context);

    // 4. Light sprite actors run last and they cannot alter the position of regular sprite actors
    sa_run_light(&sprite_draw_context);

    // VRAM animation queueing (auto-animated blocks such as muncher plants and lava)
    vram_update_animations();

    dbg_print_sandbox_instructions();
    dbg_print_status(TEXT_PALETTE_ID, hero, camera);

    end_state_check(context);

    return et_run();
}

void gameplay_frame_ended_update(GameContext *context) {
    Camera *camera = context->players[0].camera;

    enable_display(gl_fading(context) && !context->paused, !context->paused);
    camera_apply_scroll(camera);
    camera_apply_vram_update(camera, SCROLL_MAP_BASE, block_map_table);
}

static void enable_display(bool alpha_enabled, bool alpha_enable_sprites) {
    VDPLayer visible_layers = SCROLL0 | SCROLL1 | SCROLL2 | SPRITES;
    vdp_enable_layers(visible_layers);

    VDPLayer alpha_layers = (alpha_enabled ? SCROLL0 | SCROLL1 | SCROLL2 : 0);
    alpha_layers |= (alpha_enable_sprites ? SPRITES : 0);

    vdp_set_alpha_over_layers(alpha_layers);
}

static void end_state_check(GameContext *context) {
    if (gl_fading(context)) {
        return;
    }

    Hero *hero = context->players[0].hero;

    // Reload level when hero dies..
    if (hero->death_sequence_complete) {
        const uint8_t death_fade_delay = 20;
        ExtraTask *task = level_reload_sequence_task_init(death_fade_delay, true);
        context->current_fade_handle = task->handle;
    }

    // Level complete?
    if (hero->goal_reached) {
        context->level++;
        bool roll_credits = (context->level == LEVEL_COUNT);
        if (roll_credits) {
            context->level = 0;
        }

        const uint8_t fade_delay = 30;
        ExtraTask *task = level_reload_sequence_task_init(fade_delay, false);
        LevelReloadSequenceTask *reload_task = &task->level_reload_sequence;
        reload_task->final_action = (roll_credits ? GL_ACTION_SHOW_CREDITS : GL_ACTION_RELOAD_LEVEL);
        reload_task->play_credits_music = roll_credits;
        context->current_fade_handle = task->handle;
    }
}

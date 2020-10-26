#include "title_loop.h"

#include <stdint.h>

#include "vdp.h"
#include "gamepad.h"

#include "extra_task.h"
#include "title_main_tiles.h"
#include "level_loading.h"
#include "game_loop.h"
#include "palette_buffer.h"
#include "hero.h"
#include "sprite_text.h"

static void upload_graphics(void);
static void state_transition(GameContext *context, TitleState new_state);
static FadeTask *start_fade(GameContext *context, FadeTaskType type, uint16_t mask);

void title_loop_init(GameContext *context) {
    const LevelAttributes *base_level = level_attributes(0);
    level_load(base_level);

    upload_graphics();

    context->game_loop = GL_TITLE;

    static const TitleContext context_initialized = {
        .presentation_delay = 0,
        .state = TITLE_STATE_INITIAL_FADE_IN,
        .state_counter = 0,
        .scale = 0,
        .exiting = false
    };

    context->title = context_initialized;

    state_transition(context, TITLE_STATE_INITIAL_FADE_IN);
}

GameLoopAction title_step_frame(GameContext *context) {
    TitleContext *title_context = &context->title;
    const PadInputDecoded *pad_edge = &context->players[0].hero->pad_edge;

    uint16_t state_counter = context->title.state_counter++;

    switch (context->title.state) {
        case TITLE_STATE_INITIAL_FADE_IN: {
            // Initial "zoom out"
            const int16_t target_scale = 0x200;
            const int16_t scale_delta = 1 * 20;

            title_context->scale += scale_delta;
            if (title_context->scale >= target_scale) {
                title_context->scale = target_scale;
                state_transition(context, TITLE_STATE_DISPLAYING_TITLE);
            }
        } break;
        case TITLE_STATE_DISPLAYING_TITLE: {
            bool should_exit = (pad_edge->start || pad_edge->b);
            title_context->exiting |= should_exit;

            const uint16_t prompt_x = 320;
            const uint16_t prompt_y = 320;
            const uint8_t text_palette_id = 9;

            // Alpha blink prompt text
            uint8_t alpha = state_counter & 0xf;
            alpha ^= (state_counter & 0x10 ? 0xf : 0);
            pb_alpha_mask_palette(text_palette_id, alpha, false);
            st_write("PRESS (START) OR (B)", prompt_x, prompt_y);

            // Staring exit fade while prompt text is visible will be abrupt
            // Wait until it's not visible to start fading
            if (!alpha && title_context->exiting) {
                state_transition(context, TITLE_STATE_FADE_OUT);
            }
        } break;
        case TITLE_STATE_FADE_OUT:
            break;
    }

    return et_run();
}

static FadeTask *start_fade(GameContext *context, FadeTaskType type, uint16_t mask) {
    ExtraTask *fade = fade_task_init(type);
    fade->fade.palette_mask = mask;
    context->current_fade_handle = fade->handle;
    return &fade->fade;
}

static void state_transition(GameContext *context, TitleState new_state) {
    const uint16_t title_palette_mask = 0x8000;

    switch (new_state) {
        case TITLE_STATE_INITIAL_FADE_IN:
            start_fade(context, FADE_IN, title_palette_mask);
            break;
        case TITLE_STATE_DISPLAYING_TITLE:
            break;
        case TITLE_STATE_FADE_OUT: {
            FadeTask *task = start_fade(context, FADE_OUT, title_palette_mask);
            task->final_action = GL_ACTION_RESET_WORLD;
        } break;
    }

    context->title.state = new_state;
    context->title.state_counter = 0;
}

void title_frame_ended_update(GameContext *context) {
    vdp_enable_layers(AFFINE | SCROLL0 | SPRITES);
    vdp_set_alpha_over_layers(SCROLL0 | SPRITES);

    const uint16_t title_width = 256;
    const uint16_t title_height = 64;

    vdp_set_affine_pretranslate(title_width / 2, title_height / 2);
    vdp_set_affine_translate(-SCREEN_ACTIVE_WIDTH / 2, -SCREEN_ACTIVE_HEIGHT / 2);
    vdp_set_affine_matrix(context->title.scale, 0, 0, context->title.scale);
}

static void upload_graphics() {
    vdp_seek_vram(0);
    vdp_fill_vram(0x4000, 0);

    vdp_set_vram_increment(2);

    // Map (256x64, so 32x8 chars)

    const uint8_t map_width = 256 / 8;
    const uint8_t map_height = 64 / 8;

    for (uint32_t y = 0; y < map_height; y++) {
        vdp_seek_vram(y * 128);

        for (uint32_t x = 0; x < map_width / 2; x++) {
            uint8_t map_base = y * map_width + x * 2;
            vdp_write_vram(map_base | (map_base + 1) << 8);
        }
    }

    // Tiles

    const uint8_t title_palette_id = 15;

    vdp_seek_vram(0x0001);

    for (size_t i = 0; i < title_main_tiles_length / 2; i++) {
        // Graphics are 4bit indexed and must be padded to the 8bit
        uint32_t row = ((uint32_t *)title_main_tiles)[i];

        for (uint32_t x = 0; x < 4; x++) {
            uint8_t pixel_lo = (row >> 28) & 0xf;
            pixel_lo |= (pixel_lo ? title_palette_id << 4 : 0);

            uint8_t pixel_hi = (row >> 24) & 0xf;
            pixel_hi |= (pixel_hi ? title_palette_id << 4 : 0);

            vdp_write_vram(pixel_lo | pixel_hi << 8);
            row <<= 8;
        }
    }

    vdp_set_vram_increment(1);
}

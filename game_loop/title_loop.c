#include "title_loop.h"

#include <stdint.h>

#include "vdp.h"
#include "gamepad.h"
#include "math_util.h"

#include "extra_task.h"
#include "title_main_tiles.h"
#include "level_loading.h"
#include "game_loop.h"
#include "palette_buffer.h"
#include "hero.h"
#include "sprite_text.h"
#include "sound_effects.h"

static const uint8_t TEXT_PALETTE_ID = 9;

static void upload_graphics(void);
static void state_transition(GameContext *context, TitleState new_state);
static FadeTask *start_fade(GameContext *context, FadeTaskType type, uint16_t mask);
static int16_t title_scale(uint16_t state_counter, bool *fullscreen_reached, bool *end_reached);

void title_loop_init(GameContext *context) {
    const LevelAttributes *base_level = level_attributes(0);
    level_load(base_level);

    upload_graphics();

    context->game_loop = GL_TITLE;

    static const TitleContext context_initialized = {
        .presentation_delay = 0,
        .state_counter = 0,
        .scale = 0,
        .fullscreen_blink_started = false,
        .selected_menu_option = 0,
        .exiting = false
    };

    context->title = context_initialized;

    state_transition(context, TITLE_STATE_INITIAL_DELAY);
}

GameLoopAction title_step_frame(GameContext *context) {
    TitleContext *title_context = &context->title;
    const PadInputDecoded *pad_edge = &context->players[0].hero->pad_edge;

    uint16_t state_counter = context->title.state_counter++;

    bool should_exit = (pad_edge->start || pad_edge->b);

    switch (context->title.state) {
        case TITLE_STATE_INITIAL_DELAY: {
            pb_alpha_mask_palette(TEXT_PALETTE_ID, 0, false);

            const uint16_t delay = 60 * 1;

            if (state_counter >= delay) {
                state_transition(context, TITLE_STATE_INITIAL_FADE_IN);
            }
        } break;
        case TITLE_STATE_INITIAL_FADE_IN: {
            bool should_exit = false;
            bool fullscreen_reached = false;
            title_context->scale = title_scale(state_counter, &fullscreen_reached, &should_exit);

            bool should_blink = (fullscreen_reached && !title_context->fullscreen_blink_started);
            if (should_blink) {
                // Blink all colors except the black outline
                ExtraTask *task = palette_lerp_task_init(15, ~0x0002);
                task->palette_lerp.type = PALETTE_LERP_FROM;
            }

            title_context->fullscreen_blink_started |= fullscreen_reached;

            if (should_exit) {
                state_transition(context, TITLE_STATE_DISPLAYING_TITLE);
            }
        } break;
        case TITLE_STATE_DISPLAYING_TITLE: {
            const uint16_t prompt_x = 320;
            const uint16_t prompt_y = 320;

            // Alpha blink prompt text
            uint8_t alpha = state_counter & 0xf;
            alpha ^= (state_counter & 0x10 ? 0xf : 0);
            pb_alpha_mask_palette(TEXT_PALETTE_ID, alpha, false);
            st_write("PRESS (START) OR (B)", prompt_x, prompt_y);

            if (should_exit) {
                se_powerup();
                state_transition(context, TITLE_STATE_DISPLAYING_MENU);
            }
        } break;
        case TITLE_STATE_DISPLAYING_MENU: {
            const uint16_t prompt_x = 370;
            const uint16_t prompt_y = 320;
            const uint16_t line_spacing = 20;
            const int16_t cursor_offset_x = -18;

            pb_alpha_mask_palette(TEXT_PALETTE_ID, 0xf, false);

            // Available options
            const uint8_t option_count = 3;
            st_write("LEVEL 1", prompt_x, prompt_y);
            st_write("LEVEL 2", prompt_x, prompt_y + line_spacing);
            st_write("CREDITS", prompt_x, prompt_y + line_spacing * 2);

            // Cursor
            uint16_t cursor_y = prompt_y + title_context->selected_menu_option * line_spacing;
            st_write("*", prompt_x + cursor_offset_x, cursor_y);

            // User selection
            int8_t option = title_context->selected_menu_option;
            option += (pad_edge->down ? 1 : 0);
            option -= (pad_edge->up ? 1 : 0);
            option = MAX(MIN(option, option_count - 1), 0);
            title_context->selected_menu_option = option;

            if (should_exit) {
                se_powerup();
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

    TitleContext *title_context = &context->title;

    switch (new_state) {
        case TITLE_STATE_INITIAL_DELAY:
            break;
        case TITLE_STATE_INITIAL_FADE_IN:
            start_fade(context, FADE_IN, title_palette_mask);
            break;
        case TITLE_STATE_DISPLAYING_TITLE:
            break;
        case TITLE_STATE_DISPLAYING_MENU:
            break;
        case TITLE_STATE_FADE_OUT: {
            bool roll_credits = (title_context->selected_menu_option >= LEVEL_COUNT);
            if (roll_credits) {
                context->level = 0;
            } else {
                context->level = title_context->selected_menu_option;
            }

            FadeTask *task = start_fade(context, FADE_OUT, 0xffff);
            task->final_action = (roll_credits ? GL_ACTION_SHOW_CREDITS : GL_ACTION_RESET_WORLD);;
        } break;
    }

    title_context->state = new_state;
    title_context->state_counter = 0;
    title_context->exiting = false;
}

static int16_t title_scale(uint16_t state_counter, bool *fullscreen_reached, bool *end_reached) {
    int16_t angle = sys_multiply(state_counter, 5);

    const int16_t initial_bounce_complete_angle = (SIN_PERIOD / 2 - 52);
    const int16_t fullscreen_delay_complete_angle = initial_bounce_complete_angle + 160;
    const int16_t scaling_complete_angle = fullscreen_delay_complete_angle + 35;

    // Crude piecewise function but fine for a simple bounce effect on titlescreen
    if (angle >= fullscreen_delay_complete_angle) {
        *end_reached = (angle >= scaling_complete_angle);
        angle = initial_bounce_complete_angle - (angle - fullscreen_delay_complete_angle);
    } else if (angle >= initial_bounce_complete_angle) {
        *fullscreen_reached = true;
        angle = initial_bounce_complete_angle;
    }

    return sin(angle) / 16;
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

#include "credits_loop.h"

#include <stddef.h>

#include "vdp.h"
#include "gamepad.h"

#include "game_loop.h"
#include "affine.h"
#include "sprite_text.h"
#include "extra_task.h"
#include "hero.h"

static void write_title(void);
static void write_tech_attributions(void);
static void write_art_attributions(void);

static void state_update(GameContext *context);
static void state_transition(GameContext *context, CreditsState new_state);
static bool should_exit(GameContext *context);

GameLoopAction credits_step_frame(GameContext *context) {
    CreditsContext *credits_context = &context->credits;
    AffineHeroContext *affine_context = &credits_context->affine_context;

    al_prepare_update(affine_context);
    al_set_copper(affine_context);

    if (credits_context->showing_tech_attributions) {
        write_title();
        write_tech_attributions();
    } else {
        write_art_attributions();
    }

    state_update(context);

    CreditsState state = credits_context->credits_state;
    bool sprites_over_affine = (
        state == CREDITS_STATE_FADE_IN_TECH ||
        state == CREDITS_STATE_DISPLAYING_TECH ||
        state == CREDITS_STATE_FADE_OUT_TEXT ||
        state == CREDITS_STATE_FADE_OUT_TECH ||
        state == CREDITS_STATE_FADE_IN_ART ||
        state == CREDITS_STATE_DISPLAYING_ART ||
        state == CREDITS_STATE_FADE_OUT_ART
    );

    affine_context->enable_sprites = sprites_over_affine;

    return et_run();
}

void credits_loop_init(GameContext *context) {
    al_init(&context->credits.affine_context);
    state_transition(context, CREDITS_STATE_INITIAL_DELAY);

    context->credits.showing_tech_attributions = false;
    context->gameplay_active = false;
}

void credits_frame_ended_update(CreditsState state) {
    vdp_enable_layers(SPRITES);

    VDPLayer alpha_layers = SPRITES;
    bool affine_alpha_enabled = (
        state == CREDITS_STATE_INITIAL_DELAY ||
        state == CREDITS_STATE_FADE_OUT_HERO
    );
    alpha_layers |= (affine_alpha_enabled ? SCROLL0 : 0);
    vdp_set_alpha_over_layers(alpha_layers);

    al_frame_ended_update();
    al_upload();
}

// Fadeout on credits with Start / B button
static bool should_exit(GameContext *context) {
    const PadInputDecoded *pad_edge = &context->players[0].hero->pad_edge;
    return !gl_fading(context) && (pad_edge->start || pad_edge->b);
}

static void state_transition_when_fade_complete(GameContext *context, CreditsState new_state) {
    if (!gl_fading(context)) {
        state_transition(context, new_state);
    }
}

static void exiting_state_transition(GameContext *context, CreditsState new_state, uint16_t delay) {
    if (should_exit(context)) {
        state_transition(context, CREDITS_STATE_FADE_OUT_TEXT);
        return;
    }

    if (context->credits.credits_state_counter >= delay) {
        state_transition(context, new_state);
    }
}

static void state_update(GameContext *context) {
    const uint16_t initial_delay = 60 * 2;
    const uint16_t display_delay = 60 * 10;
    const uint16_t dismiss_delay = 60 * 2;

    uint32_t state_counter = context->credits.credits_state_counter++;

    switch (context->credits.credits_state) {
        case CREDITS_STATE_INITIAL_DELAY:
            if (context->credits.credits_state_counter >= initial_delay) {
                state_transition(context, CREDITS_STATE_FADE_IN_TECH);
            }
            break;
        case CREDITS_STATE_FADE_IN_TECH:
            state_transition_when_fade_complete(context, CREDITS_STATE_DISPLAYING_TECH);
            break;
        case CREDITS_STATE_DISPLAYING_TECH:
            exiting_state_transition(context, CREDITS_STATE_FADE_OUT_TECH, display_delay);
            break;
        case CREDITS_STATE_FADE_OUT_TECH:
            state_transition_when_fade_complete(context, CREDITS_STATE_FADE_IN_ART);
            break;
        case CREDITS_STATE_FADE_IN_ART:
            state_transition_when_fade_complete(context, CREDITS_STATE_DISPLAYING_ART);
            break;
        case CREDITS_STATE_DISPLAYING_ART:
            exiting_state_transition(context, CREDITS_STATE_FADE_OUT_ART, display_delay);
            break;
        case CREDITS_STATE_FADE_OUT_ART:
            state_transition_when_fade_complete(context, CREDITS_STATE_FADE_IN_TECH);
            break;
        case CREDITS_STATE_FADE_OUT_TEXT:
            if (!gl_fading(context) && (state_counter >= dismiss_delay)) {
                state_transition(context, CREDITS_STATE_FADE_OUT_HERO);
            }
            break;
        case CREDITS_STATE_FADE_OUT_HERO:
            break;
    }
}

static FadeTask *start_fade(GameContext *context, FadeTaskType type, uint16_t mask) {
    ExtraTask *fade = fade_task_init(type);
    fade->fade.palette_mask = mask;
    context->current_fade_handle = fade->handle;
    return &fade->fade;
}

static void state_transition(GameContext *context, CreditsState new_state) {
    const uint16_t hero_palette_mask = 0x0100;
    const uint16_t text_palette_mask = 0x0200;

    switch (new_state) {
        case CREDITS_STATE_INITIAL_DELAY:
            start_fade(context, FADE_IN, hero_palette_mask);
            break;
        case CREDITS_STATE_FADE_IN_TECH:
            start_fade(context, FADE_IN, text_palette_mask);
            break;
        case CREDITS_STATE_FADE_IN_ART:
            start_fade(context, FADE_IN, text_palette_mask);
            break;
        case CREDITS_STATE_FADE_OUT_ART:
            start_fade(context, FADE_OUT, text_palette_mask);
            break;
        case CREDITS_STATE_FADE_OUT_TECH:
            start_fade(context, FADE_OUT, text_palette_mask);
            break;
        case CREDITS_STATE_FADE_OUT_TEXT:
            start_fade(context, FADE_OUT, text_palette_mask);
            break;
        case CREDITS_STATE_FADE_OUT_HERO: {
            FadeTask *fade = start_fade(context, FADE_OUT, hero_palette_mask);
            fade->final_action = GL_ACTION_RESET_WORLD;
        } break;
        case CREDITS_STATE_DISPLAYING_TECH: case CREDITS_STATE_DISPLAYING_ART:
            break;
    }

    CreditsContext *credits_context = &context->credits;
    credits_context->credits_state = new_state;
    credits_context->credits_state_counter = 0;

    // Keep the current text on screen if exiting
    if (new_state != CREDITS_STATE_FADE_OUT_TEXT) {
        bool showing_tech_attributions = (
            new_state == CREDITS_STATE_FADE_OUT_TECH ||
            new_state == CREDITS_STATE_DISPLAYING_TECH ||
            new_state == CREDITS_STATE_FADE_IN_TECH
        );
        credits_context->showing_tech_attributions = showing_tech_attributions;
    }
}

static void write_title() {
    const char *title = "* SUPER MIYAMOTO SPRINT (V 0.1.0) *";
    const char *subtitle = "A GAME FOR:";
    const char *repo_link = "HTTPS://WWW.GITHUB.COM/DAN-RODRIGUES/ICESTATION-32";

    st_write(title, 268, 4);
    st_write(subtitle, 268 + 104, 4 + 17);
    st_write(repo_link, 268 - 48, 4 + 17 * 2);
}

typedef struct {
    const char *author;
    const char *work;
} Attribution;

static const Attribution tech_attributions[] = {
    { .author = "CLAIRE WOLF", .work = "YOSYS" },
    { .author = "DAVID SHAH", .work = "NEXTPNR" },

    { .author = "1BITSQUARED", .work = "ICEBREAKER BOARD" },
    { .author = "RADIONA.ORG", .work = "ULX3S BOARD" },

    { .author = "CHARLES PAPON", .work = "VEXRISCV CPU" },
    { .author = "CLAIRE WOLF", .work = "PICORV32 CPU" },

    { .author = "AND MANY MORE", .work = "CHECK THE REPO!" }
};

static const Attribution art_attributions[] = {
    { .author = "LARSIUSPRIME", .work = "MIYAMOTO SPRITES" },
    { .author = "ICEGOOM", .work = "TERRAIN GRAPHICS" },
    { .author = "GAMMA V", .work = "BACKGROUND GRAPHICS" },
    { .author = "GRAFXKID", .work = "ENEMY SPRITES" },
    { .author = "CLINT BELLANGER", .work = "(THIS FONT)" },
    { .author = "JALASTRAM", .work = "SOUND EFFECTS" },
    { .author = "FUSIONROCKER", .work = "MUSIC" },
    { .author = "SQUARE HOLIC", .work = "MUSIC" },

    { .author = "AND MANY MORE", .work = "CHECK THE REPO!" }
};

static const size_t art_attribution_count = sizeof(art_attributions) / sizeof(Attribution);
static const size_t tech_attribution_count = sizeof(tech_attributions) / sizeof(Attribution);

static void write_attributions(int16_t base_y, const Attribution *attributions, size_t count, const char *title) {
    const uint32_t base_x = 336;
    const uint32_t offset = 32;
    const uint32_t indentation = 20;
    const uint32_t line_spacing = 17;
    const uint32_t spacing = 44;

    st_write(title, base_x + 16, base_y);

    uint32_t attribution_base_y = base_y + offset;

    for (uint32_t i = 0; i < count; i++) {
        const Attribution *attribution = &attributions[i];
        st_write(attribution->author, base_x, attribution_base_y);
        st_write(attribution->work, base_x + indentation, attribution_base_y + line_spacing);

        attribution_base_y += spacing;
    }
}

static void write_tech_attributions() {
    const uint32_t base_y = 96;
    write_attributions(base_y, tech_attributions, tech_attribution_count, "* FPGA HEROES *");
}

static void write_art_attributions() {
    const uint32_t base_y = 32;
    write_attributions(base_y, art_attributions, art_attribution_count, "* ART HEROES *");
}

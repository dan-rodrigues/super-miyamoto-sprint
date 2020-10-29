#include "affine.h"

#include "vdp.h"
#include "vdp_regs.h"
#include "assert.h"
#include "math_util.h"
#include "copper.h"

#include "hero.h"
#include "hero_drawing.h"
#include "palette_buffer.h"

#define HERO_AFFINE_MAX_TILES 2

static const int16_t AFFINE_Q_1 = 0x400;

static uint16_t buffer[256];
static int16_t scale_table[480];

static void draw_8x8(uint8_t x, uint8_t y, const uint16_t *data);
static void precompute_scale_table(void);
static void rotation(int16_t angle, int16_t *scaled_sin, int16_t *scaled_cos);

static void perspective_matrix(uint32_t line,
                               int16_t sin, int16_t cos,
                               int16_t *a, int16_t *b, int16_t *c, int16_t *d);

void al_upload() {
    vdp_set_vram_increment(2);

    for (uint32_t y = 0; y < 32; y++) {
        vdp_seek_vram(y * 128);

        const uint16_t *buffer_base = &buffer[y * 8];

        vdp_write_vram(buffer_base[0]);
        vdp_write_vram(buffer_base[1]);
        vdp_write_vram(buffer_base[2]);
        vdp_write_vram(buffer_base[3]);
        vdp_write_vram(buffer_base[4]);
        vdp_write_vram(buffer_base[5]);
        vdp_write_vram(buffer_base[6]);
        vdp_write_vram(buffer_base[7]);
    }

    vdp_set_vram_increment(1);
}

void al_init(AffineHeroContext *context) {
    // Clear portion of VRAM used for affine layer
    vdp_seek_vram(0);
    vdp_fill_vram(0x4000, 0);

    // Each color in the palette is assigned a single solid 8x8 block
    // This animates better as the pixels are 8x larger this way
    vdp_seek_vram(0x0001);
    vdp_set_vram_increment(2);

    const uint8_t palette_id = 8;

    for (uint32_t i = 0; i < 16; i++) {
        uint8_t pixel = (i ? palette_id * 16 + i : 0);

        for (uint32_t tile = 0; tile < 64 / 2; tile++) {
            vdp_write_vram(pixel | pixel << 8);
        }
    }

    vdp_set_vram_increment(1);

    // Rotation is done per frame but the scale (depths-per-line) is done upfront
    precompute_scale_table();

    context->angle = 0;
    context->animation_acc = 0;
    context->animation_index = 0;
    context->enable_sprites = false;
}

void al_frame_ended_update() {
    const int8_t hero_center_x = 8;
    const int8_t hero_center_y = 4;

    vdp_set_affine_pretranslate(hero_center_x * 8, hero_center_y * 8);
    vdp_set_affine_translate(-SCREEN_ACTIVE_WIDTH / 2, -SCREEN_ACTIVE_HEIGHT / 2 + 32);
}

void al_prepare_update(AffineHeroContext *context) {
    const int16_t angle_delta = 2 * 1;

    context->angle += angle_delta;
    rotation(context->angle, &context->scaled_sin, &context->scaled_cos);

    static const HeroFrame hero_frames[] = { HF_RUN0, HF_RUN1, HF_RUN2 };
    static const size_t hero_frame_count = sizeof(hero_frames) / sizeof(HeroFrame);

    const uint8_t hero_frame_duration = 5;

    if (++context->animation_acc > hero_frame_duration) {
        if (++context->animation_index >= hero_frame_count) {
            context->animation_index = 0;
        }

        context->animation_acc = 0;
    }

    HeroFrame frame = hero_frames[context->animation_index];
    HeroTileArrangement arrangements[HERO_AFFINE_MAX_TILES];
    hero_16x16_arrangements(frame, arrangements, HERO_AFFINE_MAX_TILES);

    // Fix vertical positions, animates weird otherwise
    arrangements[0].y = 16;
    arrangements[1].y = 0;

    for (uint32_t i = 0; i < HERO_AFFINE_MAX_TILES; i++) {
        HeroTileArrangement *arrangement = &arrangements[i];

        draw_8x8(arrangement->x, arrangement->y, arrangement->top_row);
        draw_8x8(arrangement->x + 8, arrangement->y, arrangement->top_row + 0x10);
        draw_8x8(arrangement->x, arrangement->y + 8, arrangement->bottom_row);
        draw_8x8(arrangement->x + 8, arrangement->y + 8, arrangement->bottom_row + 0x10);
    }
}

void al_set_copper(const AffineHeroContext *context) {
    const uint16_t affine_offset_y = 64;
    const uint16_t copper_cutoff_line = affine_offset_y - 8;

    vdp_enable_copper(false);

    // The CPU is not "leading the raster enough" if it's caught in this loop
    // There is a risk of contention when trying to write copper RAM
    while (VDP_CURRENT_RASTER_Y > copper_cutoff_line) {}

    cop_ram_seek(0);

    cop_set_target_x(0);
    cop_wait_target_y(affine_offset_y);
    cop_wait_target_x(2);

    VDPLayer enabled_layers = AFFINE | SCROLL0;
    enabled_layers |= (context->enable_sprites ? SPRITES : 0);
    cop_write(&VDP_LAYER_ENABLE, enabled_layers);

    // Now it is safe to start writing (CPU write pointer is "ahead" of the copper PC)
    vdp_enable_copper(true);

    COPBatchWriteConfig config = {
        .mode = CWM_QUAD,
        .reg = &VDP_MATRIX_A,
        .batch_wait_between_lines = true
    };

    int32_t y = SCREEN_ACTIVE_HEIGHT - affine_offset_y;
    int32_t base = 0;

    while (y > 0) {
        uint32_t lines = MIN(32, (MAX(1, y)));
        config.batch_count = lines;
        cop_start_batch_write(&config);

        for (uint32_t i = 0; i < lines; i++) {
            int16_t a, b, c, d;
            perspective_matrix(base + affine_offset_y + i,
                               context->scaled_sin, context->scaled_cos,
                               &a, &b, &c, &d);
            cop_add_batch_quad(&config, a, b, c, d);
        }

        base += 32;
        y -= 32;

        cop_wait_target_y(base + affine_offset_y);
    }

    cop_jump(0);
}

static void draw_8x8(uint8_t base_x, uint8_t base_y, const uint16_t *data) {
    assert(base_x <= 8);
    assert(base_y <= 24);

    // The underlying data is read only so not fussed about the strict aliasing rule
    const uint32_t *data_32 = (uint32_t *)data;

    for (uint32_t y = 0; y < 8; y++) {
        uint32_t pixel_row = *data_32++;

        for (uint32_t x = 0; x < 4; x++) {
            uint16_t pixel_pair = ((pixel_row & 0x0f) << 8);
            pixel_row >>= 4;
            pixel_pair |= pixel_row & 0x0f;
            pixel_row >>= 4;

            buffer[((base_y + y) * 16 + base_x) / 2 + (3 - x)] = pixel_pair;
        }
    }
}

// Quick and dirty bodge to get 480 lines worth of perspective scales.
// This works for the silly effect on the credits screen. It is not used for gameplay.
//
// A better version of this (i.e. for a Pilotwings-type game) would be equivalent to gluLookAt().
// Instead of outputting a 4x4 matrix derived from an abstract camera, it would output 480 2x2 matrices.
// Each 2x2 matrix can then be set for each individual line, the same way it is done in this file.
static void precompute_scale_table() {
    const uint32_t exp = 11;
    const uint16_t base_divisor = 0x270;

    for (uint32_t i = 0; i < SCREEN_ACTIVE_HEIGHT; i++) {
        scale_table[i] = (AFFINE_Q_1 << exp) / (base_divisor + i * 4);
    }
}

static void rotation(int16_t angle, int16_t *scaled_sin, int16_t *scaled_cos) {
    int16_t sint = sin(-angle);
    int16_t cost = cos(-angle);

    const int16_t scale = 0x600;

    *scaled_sin = sys_multiply(sint, scale) / 0x8000;
    *scaled_cos = sys_multiply(cost, scale) / 0x8000;
}

static void perspective_matrix(uint32_t line,
                               int16_t sin, int16_t cos,
                               int16_t *a, int16_t *b, int16_t *c, int16_t *d)
{
    int16_t line_scale = scale_table[line];
    int16_t sint = sys_multiply(sin, line_scale) / AFFINE_Q_1;
    int16_t cost = sys_multiply(cos, line_scale) / AFFINE_Q_1;

    *a = cost;
    *b = -sint;
    *c = sint;
    *d = cost;
}

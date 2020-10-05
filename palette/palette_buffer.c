#include "palette_buffer.h"

#include <stdbool.h>

#include "vdp.h"
#include "gcc_lib.h"
#include "math_util.h"

#define PALETTE_SIZE 16
#define PALETTE_COUNT 16

static bool bg_color_needs_upload;
static bool palette_needs_upload[PALETTE_SIZE];

static uint16_t palette_preloaded[256];
static uint16_t palette_modified[256];

static bool use_modified_palette = false;

// It's assumed a (dummy) color 0 is included in data
void pb_preload_single_palette(uint8_t palette_id, const uint16_t *data) {
    memcpy(&palette_preloaded[palette_id * 16 + 1], data + 1, (PALETTE_SIZE - 1) * 2);
}

void pb_preload_single_color(uint8_t color_id, uint16_t color) {
    palette_preloaded[color_id] = color;
}

void pb_preload_bg_color(uint16_t color) {
    pb_preload_single_color(0, color);
    bg_color_needs_upload = true;
}

void pb_queue_all(bool including_bg_color) {
    memset(palette_needs_upload, true, sizeof(palette_needs_upload));
    bg_color_needs_upload |= including_bg_color;
}

void pb_queue_palette(uint8_t palette_id) {
    palette_needs_upload[palette_id] = true;
}

void pb_alpha_mask_multiple(uint16_t palette_mask, uint8_t alpha, bool including_bg_color) {
    uint32_t palette_id = 0;

    while (palette_mask) {
        if (palette_mask & 1) {
            pb_alpha_mask_palette(palette_id, alpha, including_bg_color);
        }

        palette_id++;
        palette_mask >>= 1;
    }
}

void pb_alpha_mask_palette(uint8_t palette_id, uint8_t alpha, bool including_bg_color) {
    uint16_t alpha_mask = alpha << 12;
    uint32_t color_id_base = (including_bg_color ? 0 : 1);
    uint32_t palette_base = palette_id * 16;

    for (uint32_t i = color_id_base; i < PALETTE_SIZE; i++) {
        uint16_t color = (palette_preloaded[palette_base + i] & 0x0fff) | alpha_mask;
        palette_modified[palette_base + i] = color;
    }

    palette_needs_upload[palette_id] = true;
    bg_color_needs_upload |= ((palette_id == 0) && including_bg_color);
    use_modified_palette = true;
}

void pb_alpha_mask_all(uint8_t alpha, bool including_bg_color) {
    pb_alpha_mask_multiple(0xffff, alpha, including_bg_color);
}

static uint8_t lerp_channel(uint8_t from, uint8_t to, uint8_t step) {
    uint8_t delta = to - from;
    uint8_t scaled_distance = (sys_multiply(delta, step) + 8) / 0x10;
    return from + scaled_distance;
}

void pb_lerp_palette_to_white(uint8_t palette_id, uint16_t mask, uint8_t step) {
    for (uint32_t i = 1; i < 16; i++) {
        if (!((mask >> i) & 1)) {
            continue;
        }

        pb_lerp_to_white(palette_id * 0x10 + i, step);
    }
}

void pb_lerp_to_white(uint8_t color_id, uint8_t step) {
    uint16_t color = palette_preloaded[color_id];

    uint8_t r = color & 0xf;
    color &= ~0xf;
    color |= lerp_channel(r, 0xf, step);

    uint8_t g = color >> 4 & 0xf;
    color &= ~(0xf << 4);
    color |= lerp_channel(g, 0xf, step) << 4;

    uint8_t b = color >> 8 & 0xf;
    color &= ~(0xf << 8);
    color |= lerp_channel(b, 0xf, step) << 8;

    palette_modified[color_id] = color;
    use_modified_palette = true;
    pb_queue_palette(color_id / 0x10);
}

void pb_reset() {
    memset(palette_needs_upload, false, sizeof(palette_needs_upload));
    bg_color_needs_upload = false;
    use_modified_palette = false;
}

void pb_upload() {
    const uint16_t *source = use_modified_palette ? palette_modified : palette_preloaded;

    for (uint32_t i = 0; i < PALETTE_COUNT; i++) {
        if (!palette_needs_upload[i]) {
            continue;
        }

        vdp_write_palette_range(i * PALETTE_SIZE + 1, PALETTE_SIZE - 1, &source[i * PALETTE_SIZE + 1]);
        palette_needs_upload[i] = false;
    }

    if (bg_color_needs_upload) {
        vdp_set_single_palette_color(0, source[0]);
    }

    pb_reset();
}

#include "palette_buffer.h"

#include <stdbool.h>

#include "vdp.h"
#include "gcc_lib.h"

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
}

void pb_queue_all() {
    memset(palette_needs_upload, true, sizeof(palette_needs_upload));
    bg_color_needs_upload = true;
}

void pb_alpha_mask_all(uint8_t alpha) {
    uint16_t alpha_mask = alpha << 12;

    for (uint32_t i = 0; i < PALETTE_COUNT * PALETTE_SIZE; i++) {
        palette_modified[i] = (palette_preloaded[i] & 0x0fff) | alpha_mask;
    }

    use_modified_palette = true;
    pb_queue_all();
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

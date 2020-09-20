#include "vram_animated_tiles.h"

#include <stdbool.h>

#include "vram_command_queue.h"
#include "animated_tiles.h"
#include "vram_layout.h"

static uint8_t animation_counter;

static void queue(uint16_t target_tile, uint16_t source_tile_base, uint32_t tile_offset);

void vram_queue_animations() {
    // Munching plants

    const uint32_t mucher_tile = 0x5c;
    const uint32_t muncher_source_tile = 0x138;

    bool frame_toggle = (animation_counter / 8) % 2;
    queue(mucher_tile, muncher_source_tile, (frame_toggle ? 0 : 4));

    // Coins

    const uint32_t coin_tile = 0x6c;
    const uint32_t coin_source_base = 0x0cc;

    uint32_t coin_source_tile = ((animation_counter / 8) % 4) * 0x10;
    queue(coin_tile, coin_source_base, coin_source_tile);

    animation_counter++;
}

void vram_queue_animated_frame_raw(uint16_t vram_base, uint16_t source_tile_base) {
    const uint16_t tile_count = 2;
    const uint16_t row_length = tile_count * 0x10;

    VRAMUploadCommand top_cmd = {
        .address = vram_base,
        .length = row_length,
        .increment = 1,
        .is_inline = false,
        .data = animated_tiles + source_tile_base * 0x10
    };

    vcq_add(&top_cmd);

    VRAMUploadCommand bottom_cmd = {
        .address = vram_base + 0x10 * 0x10,
        .length = row_length,
        .increment = 1,
        .is_inline = false,
        .data = animated_tiles + (source_tile_base + 2) * 0x10
    };

    vcq_add(&bottom_cmd);

}
static void queue(uint16_t target_tile, uint16_t source_tile_base, uint32_t tile_offset) {
    const uint16_t tile_count = 4;
    const uint16_t total_length = tile_count * 0x10;

    VRAMUploadCommand cmd = {
        .address = SCROLL_TILE_BASE + target_tile * 0x10,
        .length = total_length,
        .increment = 1,
        .is_inline = false,
        .data = animated_tiles + (source_tile_base + tile_offset) * 0x10
    };

    vcq_add(&cmd);
}

#include "vram_level_update.h"

#include "vdp.h"

#include "block.h"
#include "vram_address.h"

void vram_upload_level_column(uint16_t map_base, const uint16_t *block_map, uint16_t x) {
    vdp_seek_vram(vram_address(map_base, x, 0));
    vdp_set_vram_increment(64);

    const uint8_t screen_height_tiles = 54;
    const uint8_t screen_height_blocks = screen_height_tiles / 2;

    for (uint32_t y = 0; y < screen_height_blocks; y++) {
        // Block at this location
        uint16_t block = block_lookup(x / 2, y);
        // Map data for this block
        uint32_t index = block * 4;
        index += (x % 2) * 2;
        vdp_write_vram(block_map[index]);
        vdp_write_vram(block_map[index + 1]);
    }

    vdp_set_vram_increment(1);
}

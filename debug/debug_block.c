#include "debug_block.h"

#include "vdp.h"
#include "vdp_print.h"

#include "block_map_table.h"

void draw_block_array(uint16_t map_base) {
    vdp_seek_vram(map_base);
    vdp_set_vram_increment(1);

    for (uint32_t y = 0; y < 32; y++) {
        for (uint32_t x = 0; x < 16; x++) {
            // 2x2 block arrangement needed
            uint16_t address = map_base;
            address += x * 2;
            address += y * 64 * 2;
            vdp_seek_vram(address);

            // UL/LL/UR/LR
            uint16_t block_index = (y * 16 + x) * 4;
            vdp_write_vram(block_map_table[block_index]);
            vdp_write_vram(block_map_table[block_index + 2]);
            vdp_seek_vram(address + 64);
            vdp_write_vram(block_map_table[block_index + 1]);
            vdp_write_vram(block_map_table[block_index + 3]);
        }
    }
}

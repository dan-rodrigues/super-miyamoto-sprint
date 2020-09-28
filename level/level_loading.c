#include "level_loading.h"

#include "vdp.h"

#include "vram_layout.h"
#include "palette_init.h"
#include "sprite_buffer.h"
#include "vram_command_queue.h"
#include "sprite_actor.h"
#include "block.h"
#include "sprite_loading.h"
#include "palette_buffer.h"
#include "audio_command_queue.h"
#include "debug_print.h"
#include "vram_animated_tiles.h"
#include "global_timers.h"

#include "bg_hills_no_clouds.h"
#include "bg_hills_clouds.h"

#include "fg_bg_tiles.h"

#include "spr00_tiles.h"
#include "spr01_tiles.h"
#include "spr02_tiles.h"
#include "spr04_tiles.h"
#include "spr06_07_tiles.h"

void level_load(const LevelAttributes *attributes) {
    VDPLayer visible_layers = SCROLL0 | SCROLL1 | SCROLL2 | SPRITES;

#ifdef DEBUG_PRINT
    visible_layers |= SCROLL1;
#endif

    vdp_enable_layers(visible_layers);

    sb_reset();
    sa_reset();
    vcq_reset();
    pb_reset();
    acq_reset();
    gt_reset();

    // Foregound is a 512x512 map tiled repeatedly
    vdp_set_wide_map_layers(SCROLL0);
    vdp_set_alpha_over_layers(0);

    vdp_set_vram_increment(1);

    // Clear all VRAM

    vdp_seek_vram(0);
    vdp_fill_vram(0x8000, 0x00f8);

    // Foreground transparent tile

    const uint16_t transparent_map = 0x40f8;
    vdp_seek_vram(SCROLL_MAP_BASE);
    vdp_fill_vram(0x2000, transparent_map);

    // S0: Foreground tiles

    vdp_set_layer_map_base(0, SCROLL_MAP_BASE);
    vdp_set_layer_tile_base(0, SCROLL_TILE_BASE);

    // These tiles are shared between both BG/FG
    vdp_seek_vram(SCROLL_TILE_BASE);
    vdp_write_vram_block(fg_bg_tiles, 0x2000);

    // S1: Background tiles

    vdp_set_layer_map_base(1, BG_MAP_BASE);
    vdp_set_layer_tile_base(1, BG_TILE_BASE);

    // S2: Background tiles, secondary

    vdp_set_layer_map_base(2, BG_SECONDARY_MAP_BASE);
    vdp_set_layer_tile_base(2, BG_TILE_BASE);

    // BG Map:

    vdp_seek_vram(BG_MAP_BASE);
    vdp_write_vram_block(bg_hills_no_clouds, bg_hills_no_clouds_length);
    vdp_seek_vram(BG_SECONDARY_MAP_BASE);
    vdp_write_vram_block(bg_hills_clouds, bg_hills_clouds_length);

    // Palette

    palette_init(attributes);

    // Sprites

    vdp_set_sprite_tile_base(SPRITE_TILE_BASE);
    vdp_clear_all_sprites();

    vdp_seek_vram(SPRITE_TILE_BASE);
    // 00: Common sprites 1/4
    vdp_write_vram_block(spr00_tiles, spr00_tiles_length);
    // Coin frame, directly picked from animation frames
    vram_queue_animated_frame_raw(SPRITE_TILE_BASE + 0x10 * 0x00e, 0xdc);

    // 01: Comm sprites 2/4
    vdp_write_vram_block(spr01_tiles, spr01_tiles_length);
    // 02: Comm sprites 3/4
    vdp_write_vram_block(spr02_tiles, spr02_tiles_length);

    // 04: Tank enemy (skipping due to double size sprite above)
    vdp_write_vram_block(spr04_tiles, spr04_tiles_length);

    // 06-07: 8x16 font
    vdp_seek_vram(SPRITE_TILE_BASE + 0x1800);
    vdp_write_vram_block(spr06_07_tiles, spr06_07_tiles_length);

    // Load level into RAM

    block_load_level(attributes->block_data, attributes->stride);

    // Sprite level data setup

    sprite_level_data_init(attributes->sprite_data, *attributes->sprite_data_length);
}

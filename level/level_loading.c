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
#include "extra_task.h"
#include "camera_init.h"
#include "hero_init.h"
#include "block_map_table.h"

#include "bg_hills_no_clouds.h"
#include "bg_hills_clouds.h"

#include "fg_bg_tiles.h"

#include "spr00_tiles.h"
#include "spr01_tiles.h"
#include "spr02_tiles.h"
#include "spr04_tiles.h"
#include "spr06_07_tiles.h"

void level_init(const LevelAttributes *attributes, GameContext *context) {
    const PlayerContext *p1 = &context->players[0];

    // Load and initialize various level attributes while screen is inactive
    
    level_load(attributes);

    hero_level_init(p1->hero, attributes, p1->max_life, p1->midpoint_reached);
    camera_init(p1->camera, p1->hero, block_map_table);
    sprite_level_data_perform_initial_load(p1->camera, p1->hero);

    dbg_print_init();

    // Start the gradual fade after the above loading (invisible) is done

    const bool skip_fade_in = false;

    if (!skip_fade_in) {
        ExtraTask *fade = fade_task_init(FADE_IN);
        fade->fade.fade_delta = 2;
        context->current_fade_handle = fade->handle;
    } else {
        pb_alpha_mask_all(0xf, true);
    }

    context->paused = false;
}

void level_load(const LevelAttributes *attributes) {
    sb_reset();
    sa_reset();
    vcq_reset();
    pb_reset();
    acq_reset();
    gt_reset();
    et_reset();

    // Foregound is a 512x512 map tiled repeatedly
    vdp_set_wide_map_layers(SCROLL0);
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

    // Initial animation frames since they may not be visible immediately otherwise
    
    vram_init_animations();
}

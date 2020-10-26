#include "palette_init.h"

#include "vdp.h"

#include "palette_buffer.h"
#include "debug_print.h"

#include "miyamoto_palette.h"
#include "bg_palette.h"
#include "bush_palette.h"
#include "coin_palette.h"
#include "cloud_palette.h"
#include "blue_palette.h"
#include "gray_palette.h"
#include "fgbg_palette.h"
#include "title_main_palette.h"

#include "spr00_palette.h"
#include "spr01_palette.h"
#include "spr02_palette.h"
#include "spr04_palette.h"

void palette_init(const LevelAttributes *attributes) {
    // BG color
    // This is exempt from alpha fades and is uploaded first separately
    const uint16_t background_color = 0xf488;
    pb_preload_bg_color(background_color);
    pb_upload();

    // BG palette
    pb_preload_single_palette(0, bg_palette);
    // Ground palette
    pb_preload_single_palette(2, fgbg_palette);
    // Gray palette
    pb_preload_single_palette(3, gray_palette);
    // BG palette #2:
    pb_preload_single_palette(4, cloud_palette);
    // Bush palette
    pb_preload_single_palette(5, bush_palette);
    // Coin palette
    pb_preload_single_palette(6, coin_palette);
    // Blue coin palette
    pb_preload_single_palette(7, blue_palette);

    // Sprites:

    // 8: Hero
    pb_preload_single_palette(8, miyamoto_palette);

    // Common sprites:

    // 9: Gray (moving platform)
    pb_preload_single_palette(9, spr00_palette);
    // 10: Blue / orange, various enemies
    pb_preload_single_palette(10, spr01_palette);
    // 11: Green enemy
    pb_preload_single_palette(11, spr02_palette);
    // 12: Tank
    pb_preload_single_palette(12, spr04_palette);

    // 15: Title
    pb_preload_single_palette(15, title_main_palette);

#ifdef DEBUG_PRINT
    pb_preload_single_color(TEXT_PALETTE_ID * 0x10 + 1, 0xffff);
#endif
}

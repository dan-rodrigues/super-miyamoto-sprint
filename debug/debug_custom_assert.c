#include "debug_custom_assert.h"

#include "vdp.h"
#include "vdp_print.h"
#include "font.h"

#include "vram_layout.h"

void custom_assert_handler(const char *file, int line, const char *message) {
#ifdef ASSERT_ENABLED
    vp_print_init();

    vdp_enable_layers(SCROLL0);
    vdp_set_wide_map_layers(SCROLL0);
    vdp_set_alpha_over_layers(0);

    vdp_set_vram_increment(1);
    vdp_seek_vram(0);
    vdp_fill_vram(0x8000, 0x0000);

    vdp_set_layer_map_base(0, TEXT_MAP_BASE);
    vdp_set_layer_tile_base(0, TEXT_TILE_BASE);
    upload_font(TEXT_TILE_BASE);
    vdp_set_layer_scroll(0, 0, 0);

    vdp_set_single_palette_color(0, 0xf000);
    vdp_set_single_palette_color(TEXT_PALETTE_ID * 0x10 + 1, 0xffff);

    vp_printf(2, 2, TEXT_PALETTE_ID, SCROLL0, TEXT_MAP_BASE,
              "Assertion failed: %s:%d",
              file, line);
    vp_printf(2, 4, TEXT_PALETTE_ID, SCROLL0, TEXT_MAP_BASE,
              "%s",
              message);
#endif

    while(true) {};
}

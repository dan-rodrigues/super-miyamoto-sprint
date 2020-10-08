#include "debug_print.h"

#include "vdp.h"
#include "font.h"
#include "vdp_print.h"
#include "hero.h"

#include "basic_enemy_sprite.h"
#include "impact_sprite.h"
#include "smoke_sprite.h"

#include "vram_layout.h"

#ifdef DEBUG_PRINT

static const char *dbg_actor_title(const SpriteActor *actor) {
    const void *main = actor->main;

    if (main == basic_enemy_sprite_main)
        return "basic enemy";
    if (main == impact_sprite_main)
        return "impact";
    if (main == smoke_sprite_main)
        return "smoke (small)";

    return "(unrecognized)";
}

#endif

void dbg_print_init() {
#ifdef DEBUG_PRINT
    vp_print_init();
    vdp_set_layer_map_base(1, TEXT_MAP_BASE);
    vdp_set_layer_tile_base(1, TEXT_TILE_BASE);
    upload_font(TEXT_TILE_BASE);
    vdp_set_layer_scroll(SCROLL1, 0, 0);
#endif
}

void dbg_print_status(uint8_t palette_id, Hero *hero, Camera *camera) {
    uint32_t x = hero->position.x / 16;
    uint32_t y = hero->position.y / 16;
    uint16_t block = block_lookup(x, y);
    dbg_printf(2, 2, "Collision block: %4x, is solid: %d", block, block_get_attributes(block) & BLOCK_CAN_STAND);
    dbg_printf(2, 3, "Hero X/Y: (%d, %d)    ", hero->position.x, hero->position.y);
    dbg_printf(2, 4, "Scroll[0] X/Y: (%d, %d)    ", camera->scroll.x, camera->scroll.y);
}

void dbg_printf(uint8_t x, uint8_t y, const char *fmt, ...) {
#ifdef DEBUG_PRINT
    va_list args;
    va_start(args, fmt);
    vp_printf_list(x, y, TEXT_PALETTE_ID, SCROLL1, TEXT_MAP_BASE, fmt, args);
    va_end(args);
#endif
}

#ifdef DEBUG_PRINT
static const uint8_t ACTOR_X = 6;
static const uint8_t ACTOR_Y = 6;
#endif

void dbg_print_actor(const SpriteActor *actor) {
#ifdef DEBUG_PRINT
    SpriteActorHandle handle = actor->handle;

    const char *label = actor->debug_label ? actor->debug_label : "";

    dbg_printf(ACTOR_X, ACTOR_Y + handle.index,
               "actor[%02x]: %s %s",
               handle.index, dbg_actor_title(actor), label);
#endif
}

void dbg_actor_clear(const SpriteActor *actor) {
#ifdef DEBUG_PRINT
    vp_clear_row(ACTOR_Y + actor->handle.index, SCROLL1, TEXT_MAP_BASE, true);
#endif
}

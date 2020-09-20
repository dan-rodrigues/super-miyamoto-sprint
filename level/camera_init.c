#include "camera_init.h"

#include "hero.h"
#include "vram_layout.h"
#include "math_util.h"

void camera_init(Camera *camera, const Hero *hero, const uint16_t *block_map) {
    CameraScroll offset_scroll = {hero->position.x - 400, -48};
    
    if (offset_scroll.x < 0) {
        offset_scroll.x = 0;
    }

    Camera camera_initialized = {
        .scroll = offset_scroll,
        .x_scroll_last_update = offset_scroll.x,
        .y_scroll_last_update = offset_scroll.y,
        .chasing_hero = false
    };

    // This MIGHT be a confusion of responsibilities
    camera_fullscreen_vram_update(SCROLL_MAP_BASE, block_map, &camera_initialized);

    *camera = camera_initialized;
}

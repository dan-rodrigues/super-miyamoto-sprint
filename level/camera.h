#ifndef camera_h
#define camera_h

#include <stdint.h>
#include <stdbool.h>

#include "camera_types.h"
#include "hero_types.h"

struct CameraScroll {
    int16_t x, y;
};

struct LevelVRAMUpdate {
    uint16_t x_level_source;
    bool pending_update;
};

struct Camera {
    // Current position, used to set screen scroll
    CameraScroll scroll;

    bool chasing_hero;
    bool chasing_from_left;
    bool locked_onto_hero;
    bool hero_moving_left;

    // The last X position where VRAM was updated (low 3 bits ignored)
    int16_t x_scroll_last_update;
    // (Only would be used for vertical scroll updates)
    int16_t y_scroll_last_update;

    // Pending updates to VRAM for the current frame
    LevelVRAMUpdate vram_update;
};

void camera_fullscreen_vram_update(uint16_t map_base, const uint16_t *block_map, const Camera *camera);

bool camera_hero_beyond_right_bound(const Camera *camera, const Hero *hero, int16_t bound);
bool camera_hero_beyond_left_bound(const Camera *camera, const Hero *hero, int16_t bound);

void camera_update(Camera *camera, const Hero *hero);

void camera_apply_scroll(const Camera *camera);
void camera_apply_vram_update(Camera *camera, uint16_t map_base, const uint16_t *block_map);

bool camera_point_visible(int32_t x, int32_t y, const Camera *camera);

#endif /* camera_h */

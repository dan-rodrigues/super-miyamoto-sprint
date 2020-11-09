#include "camera.h"

#include "vdp.h"
#include "assert.h"

#include "hero.h"
#include "vram_level_update.h"

bool camera_hero_beyond_right_bound(const Camera *camera, const Hero *hero, int16_t bound) {
    return (hero->position.x - camera->scroll.x) > bound;
}

bool camera_hero_beyond_left_bound(const Camera *camera, const Hero *hero, int16_t bound) {
    return (hero->position.x - camera->scroll.x) < bound;
}

// Updates camera and vram_update as needed

void camera_update(Camera *camera, const Hero *hero) {
    LevelVRAMUpdate *vram_update = &camera->vram_update;

    vram_update->pending_update = false;

    const int16_t tracking_target_screen_x = SCREEN_ACTIVE_WIDTH / 2;

    // Target bounds to try and keep the hero within
    const int16_t x_padding = SCREEN_ACTIVE_WIDTH / 3;
    const int16_t x_right_bound = SCREEN_ACTIVE_WIDTH - x_padding;
    const int16_t x_left_bound = x_padding;
    const int16_t camera_chase_speed = 4;

    bool hero_changed_direction = false;
    if (hero->velocity.x != 0) {
        hero_changed_direction = camera->hero_moving_left != (hero->velocity.x < 0);
    }

    // If hero walks outside the bound, enter tracking mode
    if (hero_changed_direction) {
        camera->chasing_hero = false;
        camera->locked_onto_hero = false;
    } else if (!camera->chasing_hero) {
        if (camera_hero_beyond_right_bound(camera, hero, x_right_bound)) {
            camera->chasing_hero = true;
            camera->chasing_from_left = false;
        } else if (camera_hero_beyond_left_bound(camera, hero, x_left_bound)) {
            camera->chasing_hero = true;
            camera->chasing_from_left = true;
        }

        camera->locked_onto_hero = false;
    }

    if (camera->chasing_hero) {
        // Quickly move the camera until the target position is reached
        if (!camera->chasing_from_left && camera_hero_beyond_right_bound(camera, hero, tracking_target_screen_x)) {
            camera->scroll.x += camera_chase_speed;
            camera->locked_onto_hero = !camera_hero_beyond_right_bound(camera, hero, tracking_target_screen_x);
        } else if (camera->chasing_from_left && camera_hero_beyond_left_bound(camera, hero, tracking_target_screen_x)) {
            camera->scroll.x -= camera_chase_speed;
            camera->locked_onto_hero = !camera_hero_beyond_left_bound(camera, hero, tracking_target_screen_x);
        }

        if (camera->locked_onto_hero) {
            // Remain locked onto the hero's exact position
            camera->scroll.x = hero->position.x - tracking_target_screen_x;
        }

        if (camera->scroll.x < 0) {
            camera->scroll.x = 0;
            camera->locked_onto_hero = false;
            camera->chasing_hero = false;
        }
        // (right level bounds check..)
    }

    // Direction changes need to be tracked to stop camera tracking if needed
    if (hero->velocity.x != 0) {
        camera->hero_moving_left = hero->velocity.x < 0;
    }

    // If the camera position changed, did it change enough to require a VRAM update?
    if ((camera->scroll.x & 0xfff8) != (camera->x_scroll_last_update & 0xfff8)) {
        bool camera_moving_left = camera->scroll.x < camera->x_scroll_last_update;

        // Need to write columns in the offscreen region which is just about to come into view
        if (camera_moving_left) {
            vram_update->x_level_source = camera->scroll.x / 8 - 1;
        } else {
            vram_update->x_level_source = camera->scroll.x / 8 + SCREEN_ACTIVE_WIDTH / 8 + 1;
        }

        // Update context to guard against future unnecessary updates
        camera->x_scroll_last_update = camera->scroll.x;
        vram_update->pending_update = true;
    }
}

void camera_apply_scroll(const Camera *camera) {
    // This assumes certain layers and scroll offsets (to be tweaked)
    const int16_t bg_y_offset = -56;
    vdp_set_layer_scroll(0, camera->scroll.x, camera->scroll.y);
    vdp_set_layer_scroll(1, camera->scroll.x / 4, camera->scroll.y / 4 + bg_y_offset);
    vdp_set_layer_scroll(2, camera->scroll.x / 8, camera->scroll.y / 8 + bg_y_offset);
}

void camera_apply_vram_update(Camera *camera, uint16_t map_base, const uint16_t *block_map) {
    if (camera->vram_update.pending_update) {
        vram_upload_level_column(map_base, block_map, camera->vram_update.x_level_source);
        camera->vram_update.pending_update = false;
    }
}

// Initial left-to-right sweep to load a screenfull of level graphics

void camera_fullscreen_vram_update(uint16_t map_base, const uint16_t *block_map, const Camera *camera) {
    for (uint32_t x = 0; x < 128; x++) {
        vram_upload_level_column(map_base, block_map, camera->scroll.x / 8 + x - 1);
    }
}

// Checks if point is visible for the given camera

bool camera_point_visible(int32_t x, int32_t y, const Camera *camera) {
    // No reason to pass y!=0 for now
    assert(!y);

    return (x >= camera->scroll.x) && x < (camera->scroll.x + SCREEN_ACTIVE_WIDTH);
}

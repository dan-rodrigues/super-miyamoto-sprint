#include "hero_init.h"

#include "hero.h"

void hero_level_init(Hero *hero, const LevelAttributes *attributes) {
    SpritePosition hero_position = {
        .x = attributes->start_position.x,
        .y = attributes->start_position.y,
        .x_fraction = 0,
        .y_fraction = 0
    };

    Hero hero_initialized = {
        .life = HERO_DEFAULT_LIFE,
        .coins = 0,
        .invulnerability_counter = 0,

        .pad = PAD_INPUT_DECODED_NO_INPUT,
        .pad_edge = PAD_INPUT_DECODED_NO_INPUT,
        
        .frame = HERO_IDLE_FRAME,
        .frame_counter = 0,
        .visible = true,

        .grounded = false,
        .quick_turning = false,
        .against_solid_block = false,
        .animation_counter = 0,
        .direction = RIGHT,
        .position = hero_position,
        .velocity = {0, 0},
        .ducking = false,
        .ridden_sprite_handle = SA_HANDLE_FREE,
        .kick_timer = 0,
        .carried_sprite_handle = SA_HANDLE_FREE,

        .climbing = false,
        .moved_while_climbing = false,

        .sprint_charge_counter = 0,
        .sprinting = false,
        .sprint_jumping = false,

        .airborne_bounce_pending = false,
        .damage_pending = false
    };

    *hero = hero_initialized;
}

#include "hero_init.h"

#include "hero.h"

void hero_level_init(Hero *hero, const LevelAttributes *attributes) {
    static const Hero hero_initialized = {
        .life = 0,
        .life_max = 0,
        .death_timer = 0,
        .dead = false,

        .coins = 0,
        .invulnerability_counter = 0,

        .pad = {},
        .pad_edge = {},
        
        .frame = HF_RUN0,
        .uploaded_frame = HF_UNDEFINED,
        .frame_counter = 0,
        .visible = true,

        .grounded = false,
        .quick_turning = false,
        .against_solid_block = false,
        .animation_counter = 0,
        .direction = RIGHT,
        .position = {},
        .velocity = { 0, 0 },
        .ducking = false,
        .platform_sprite_handle = {},

        .vehicle_sprite_handle = {},
        .pending_vehicle_entry = false,

        .kick_timer = 0,
        .carried_sprite_handle = {},

        .climbing = false,
        .moved_while_climbing = false,

        .sprint_charge_counter = 0,
        .sprinting = false,
        .sprint_jumping = false,

        .airborne_bounce_pending = false,
        .damage_pending = false
    };

    *hero = hero_initialized;

    SpritePosition hero_position = {
        .x = attributes->start_position.x,
        .y = attributes->start_position.y,
        .x_fraction = 0,
        .y_fraction = 0
    };
    hero->position = hero_position;

    hero->life = HERO_DEFAULT_LIFE;
    hero->life_max = HERO_DEFAULT_LIFE;

    hero->frame = HERO_IDLE_FRAME;
    hero->platform_sprite_handle = SA_HANDLE_FREE;
    hero->vehicle_sprite_handle = SA_HANDLE_FREE;
    hero->carried_sprite_handle = SA_HANDLE_FREE;
}

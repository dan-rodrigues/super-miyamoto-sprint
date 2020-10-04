#include "hero_init.h"

#include "hero.h"

void hero_level_init(Hero *hero,
                     const LevelAttributes *attributes,
                     uint8_t max_life,
                     bool midpoint_position)
{
    static const Hero hero_initialized = {
        .life = 0,
        .max_life = 0,
        .death_timer = 0,
        .dead = false,
        .death_sequence_complete = false,
        .visible = true,

        .midpoint_reached = false,
        .goal_reached = false,

        .coins = 0,
        .invulnerability_counter = 0,

        .pad = {},
        .pad_edge = {},
        
        .frame = HF_RUN0,
        .uploaded_frame = HF_UNDEFINED,
        .uploaded_transluscent_palette = false,
        .frame_counter = 0,
        .transluscent = true,

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

    const LevelPosition *initial_position = midpoint_position
        ? &attributes->mid_position : &attributes->start_position;

    SpritePosition hero_position = {
        .x = initial_position->x,
        .y = initial_position->y,
        .x_fraction = 0,
        .y_fraction = 0
    };
    hero->position = hero_position;

    hero->life = max_life;
    hero->max_life = max_life;
    hero->midpoint_reached = midpoint_position;

    hero->frame = HERO_IDLE_FRAME;
    hero->platform_sprite_handle = SA_HANDLE_FREE;
    hero->vehicle_sprite_handle = SA_HANDLE_FREE;
    hero->carried_sprite_handle = SA_HANDLE_FREE;
}

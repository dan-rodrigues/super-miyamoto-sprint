#ifndef tank_sprite_h
#define tank_sprite_h

#include <stdbool.h>

#include "sprite_actor_types.h"
#include "sprite_position.h"

typedef enum {
    TANK_STATE_PATROL_ACCEL,
    TANK_STATE_PATROL_DECEL,
    TANK_STATE_CHASE_HERO
} TankDrivenState;

typedef struct {
    TankDrivenState enemy_driver_state;
    uint8_t current_state_duration;
    uint8_t frames_since_direction_change;
    
    SpriteOffset hero_offset;
    uint8_t hero_collision_grace;

    uint8_t launch_counter;
    uint8_t missiles_launched;
    bool firing, launch_pending;

    uint8_t exahust_sprite_acc;
    uint8_t animation_acc;
    uint8_t animation_index;

    SpriteActorHandle driver_enemy_handle;
} TankSprite;

void tank_sprite_main(SpriteActor *self, const SpriteEnvironment *env);
SpriteActor *tank_sprite_init(const SpritePosition *position, bool include_driver);

void tank_sprite_hero_position_context(SpriteActor *self, const Hero *hero);
void tank_sprite_hero_drive_update(SpriteActor *self, Hero *hero, SpriteVehicleHeroContext *hero_context);

#endif /* tank_sprite_h */

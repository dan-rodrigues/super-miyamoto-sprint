#ifndef hero_h
#define hero_h

#include <stdint.h>
#include <stdbool.h>

#include "gamepad.h"

#include "hero_types.h"

#include "sprite_buffer.h"
#include "sprite_position.h"
#include "block.h"

#include "sprite_actor.h"

extern const uint8_t HERO_DEFAULT_LIFE;
extern const uint8_t HERO_INVULNERABILITY_COUNT;
extern const uint8_t HERO_SPRINT_CHARGE_TIME;

extern const HeroFrame HERO_IDLE_FRAME;

enum HeroFrame {
    RUN0, RUN1, RUN2,
    SPRINT0, SPRINT1, SPRINT2,
    CARRY0, CARRY1, CARRY2,

    KICK,

    DRIVING,

    CLIMBING0, CLIMBING1,
    
    RUN_TURNING,

    JUMP_RISING, JUMP_FALLING,
    JUMP_SPRINTING,

    LOOKING_UP,

    LOOKING_SCREEN_DIRECT,
    LOOKING_SCREEN_LEFT,
    LOOKING_SCREEN_RIGHT,

    DUCKING,

    PEACE_SIGN
};

struct Hero {
    HeroFrame frame;
    int32_t frame_counter;
    uint8_t animation_counter;

    uint8_t coins;

    PadInputDecoded pad, pad_edge;

    bool against_solid_block;

    bool climbing;
    bool moved_while_climbing;
    
    bool ducking;
    bool grounded;

    uint8_t sprint_charge_counter;
    bool sprinting;
    bool sprint_jumping;
    bool quick_turning;

    bool airborne_bounce_pending;

    bool damage_pending;
    bool damage_causes_hop;

    uint8_t life;
    uint8_t life_max;
    uint8_t death_timer;
    bool dead;
    
    uint8_t invulnerability_counter;
    bool visible;

    SpritePosition position;
    SpriteVelocity velocity;
    SpriteDirection direction;

    SpriteActorHandle vehicle_sprite_handle;
    SpriteVehicleHeroContext vehicle_context;
    bool pending_vehicle_entry;

    SpriteActorHandle platform_sprite_handle;

    SpriteActorHandle carried_sprite_handle;
    uint8_t kick_timer;

    SpriteBoundingBoxAbs bounding_box_abs;
};

struct HeroTileOffset {
    int8_t x, y;
};

#define HERO_FRAME_MAX_TILES 3
#define HERO_FRAME_TILES_END -1

struct HeroFrameLayout {
    // This field temporary for as long as dictionary-lookup is used
    const HeroFrame frame;

    const int8_t x_flip_offset;
    const uint8_t x_flip;

    const int16_t tiles[HERO_FRAME_MAX_TILES];
    const HeroTileOffset *tile_offsets;
    const HeroTileOffset offset;
};

void hero_update_state(Hero *hero, const Camera *camera);

const SpriteBoundingBox *hero_sprite_bounding_box(const Hero *hero);
bool hero_has_draw_priority(const Hero *hero);

// Carried sprites

bool hero_carrying_sprite(const Hero *hero, SpriteActorHandle handle);

struct HeroSpriteCarryContext {
    SpritePosition position;
    SpriteDirection direction;
} ;

HeroSpriteCarryContext hero_sprite_carry_context(const Hero *hero);

enum HeroSpriteCarryUpdateResult {
    HERO_SPRITE_CARRY_UPDATE_NOT_CARRYING,
    HERO_SPRITE_CARRY_UPDATE_DROPPED,
    HERO_SPRITE_CARRY_UPDATE_KICKED,
    HERO_SPRITE_CARRY_UPDATE_KICKED_UP,
    HERO_SPRITE_CARRY_UPDATE_CARRYING
};

HeroSpriteCarryUpdateResult hero_sprite_carry_update(Hero *hero, const SpriteActor *actor);

// Sprite interaction

void hero_platform_interaction(Hero *hero, SpriteActor *platform);

void hero_set_airborne_bounce(Hero *hero);
void hero_damage(Hero *hero);

// Vehicles

struct HeroVehicleControl {
    bool move_left, move_right;
    bool move_fast;
    bool eject;
    bool jump;
    bool action;
};

void hero_enter_vehicle(Hero *hero, SpriteActor *actor);
bool hero_in_vehicle(const Hero *hero, const SpriteActor *actor);
bool hero_in_any_vehicle(const Hero *hero);
bool hero_vehicle_control_state(const Hero *hero, HeroVehicleControl *control);

#endif /* hero_h */

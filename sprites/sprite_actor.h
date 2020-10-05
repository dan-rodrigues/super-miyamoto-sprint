#ifndef sprite_actor_h
#define sprite_actor_h

#include <stddef.h>

#include "sprite_position.h"
#include "sprite_actor_types.h"

#include "smoke_sprite.h"
#include "basic_enemy_sprite.h"
#include "impact_sprite.h"
#include "layered_enemy_sprite.h"
#include "platform_sprite.h"
#include "ball_enemy_sprite.h"
#include "glitter_sprite.h"
#include "tank_sprite.h"
#include "missile_sprite.h"
#include "tank_driver_sprite.h"
#include "spark_sprite.h"
#include "moving_smoke_sprite.h"
#include "jumping_enemy_sprite.h"
#include "jetpack_enemy_sprite.h"
#include "goal_sprite.h"
#include "midpoint_sprite.h"

#define SPRITE_ACTOR_MAX 32
#define SPRITE_ACTOR_LIGHT_MAX 48

struct SpriteActor {
    SpriteActorMain main;
    SpriteActorHandle handle;
    int8_t level_data_index;

    SpritePosition position;
    SpriteVelocity velocity;
    SpriteDirection direction;

    bool rideable;
    bool can_ride;
    SpriteActorHandle ridden_sprite_handle;

    bool killed;
    bool interacts_with_sprites;
    bool touch_hurts_hero;
    bool stomp_hurts_hero;
    bool stompable;
    bool can_be_carried;
    bool dies_upon_stomp;
    bool falls_off_when_killed;
    bool kills_other_sprites;
    bool immune_to_projectiles;

    bool thud_sound_upon_hitting_wall;
    bool invert_velocity_upon_hitting_wall;
    
    // This can be considered duplicate state if there's only ever 1 hero
    // In this case the hero should be the source of truth
    // For multiplayer there will be multiple heroes though may be worth keeping this
    bool carried;

    // Used for sprite<->sprite collision only
    SpriteBoundingBox bounding_box;

    // Used for sprite<->light sprite collision only
    SpriteBoundingBoxAbs bounding_box_abs;

#ifdef DEBUG_PRINT
    const char *debug_label;
#endif

    // Sprite definitions
    union {
        BasicEnemy basic_enemy;
        LayeredEnemy layered_enemy;
        PlatformSprite platform;
        BallEnemy ball_enemy;
        TankSprite tank;
        TankDriverSprite tank_driver;
        JumpingEnemySprite jumping_enemy;
        JetpackEnemySprite jetpack_enemy;
        GoalSprite goal;
        MidpointSprite midpoint;
        // ...
    };
};

struct SpriteEnvironment {
    const Camera *camera;

    // There is only one Hero for now
    // This may be extended later if a sprite must interact with multiple Hero instances
    Hero *hero;
} ;

void sa_reset(void);

SpriteActor *sa_alloc(void);
SpriteActor *sa_alloc_after(SpriteActorHandle handle);

void sa_init(SpriteActor *actor, SpriteActorMain main);
bool sa_level_index_live(int8_t level_index);
void sa_free(SpriteActor *actor);

bool sa_live(const SpriteActor *actor);
bool sa_handle_live(SpriteActorHandle handle);

bool sa_handle_equal(SpriteActorHandle h1, SpriteActorHandle h2);
void sa_handle_clear(SpriteActorHandle *handle);

void sa_run_non_rideable(const SpriteEnvironment *env);
void sa_run_rideable(const SpriteEnvironment *env);

SpriteActor *sa_get(SpriteActorHandle handle);
SpriteActor *sa_index(uint8_t index);
bool sa_iterate_all(SpriteActor *caller, SpriteActorIteratorCallback callback);

struct SpriteDeferredDrawTask {
    SpriteActor *actor;
    SpriteDeferredDrawFunction function;
};

void sa_add_deferred_draw_task(SpriteActor *actor, SpriteDeferredDrawFunction function);
void sa_run_deferred_draw_tasks(const SpriteEnvironment *environment);

// Vehicles:

struct SpriteVehicleHeroContext {
    SpriteBoundingBox sprite_box;

    SpriteBoundingBox horizontal_block_boxes[2];
    SpriteBoundingBox vertical_block_boxes[2];
    uint8_t box_count;
};

void sa_hero_vehicle_update(SpriteActor *vehicle, Hero *hero, SpriteVehicleHeroContext *hero_context);

// Light actors:

struct SpriteActorLight {
    SpriteActorLightMain main;
    SpriteActorLightHandle handle;

    SpritePosition position;
    SpriteVelocity velocity;
    SpriteDirection direction;

    bool killed;
    bool interacts_with_sprites;
    bool falls_off_when_killed;
    bool kills_other_sprites;
    bool hurts_hero;

    // This is ONLY used for sprite<->sprite collision
    SpriteBoundingBox bounding_box;

    // Sprite definitions
    union {
        SmokeSprite smoke;
        MovingSmokeSprite moving_smoke;
        ImpactSprite impact;
        GlitterSprite glitter;
        MissileSprite missile;
        SparkSprite spark;
        // ...
    };
};

SpriteActorLight *sa_alloc_light(void);
void sa_init_light(SpriteActorLight *actor, SpriteActorLightMain main);
void sa_free_light(SpriteActorLight *actor);

bool sa_live_light(const SpriteActorLight *actor);
bool sa_handle_live_light(SpriteActorLightHandle handle);

void sa_run_light(const SpriteEnvironment *env);

#endif /* sprite_actor_h */

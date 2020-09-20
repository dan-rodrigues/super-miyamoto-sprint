#ifndef sprite_actor_h
#define sprite_actor_h

#include <stddef.h>

#include "sprite_position.h"
#include "sprite_actor_types.h"

#include "smoke_sprite.h"
#include "basic_enemy_sprite.h"
#include "impact_sprite.h"
#include "enemy_generator_sprite.h"
#include "layered_enemy_sprite.h"
#include "platform_sprite.h"
#include "ball_enemy_sprite.h"
#include "glitter_sprite.h"

#define SPRITE_ACTOR_MAX 32
#define SPRITE_ACTOR_LIGHT_MAX 32

extern const SpriteActorHandle SA_HANDLE_FREE;

struct SpriteActorHandle {
    uint8_t index;
    uint8_t generation;
};

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

    bool thud_sound_upon_hitting_wall;
    
    // This can be considered duplicate state if there's only ever 1 hero
    // In this case the hero should be the source of truth
    // For multiplayer there will be multiple heroes though may be worth keeping this
    bool carried;

    // This is ONLY used for sprite<->sprite collision
    SpriteBoundingBox bounding_box;

    // Debug
    const char *debug_label;

    // Sprite definitions
    union {
        BasicEnemy basic_enemy;
        EnemyGeneratorSprite enemy_generator;
        LayeredEnemy layered_enemy;
        PlatformSprite platform;
        BallEnemy ball_enemy;
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

struct SpriteDeferredDrawTask {
    SpriteActor *actor;
    SpriteDeferredDrawFunction function;
};

void sa_add_deferred_draw_task(SpriteActor *actor, SpriteDeferredDrawFunction function);
void sa_run_deferred_draw_tasks(SpriteEnvironment *environment);

// Light actors:

struct SpriteActorLightHandle {
    uint8_t index;
    uint8_t generation;
};

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

    // This is ONLY used for sprite<->sprite collision
    SpriteBoundingBox bounding_box;

    // Sprite definitions
    union {
        SmokeSprite smoke;
        ImpactSprite impact;
        GlitterSprite glitter;
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

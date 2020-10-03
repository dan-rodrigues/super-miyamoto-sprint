#include "sprite_actor.h"

#include "assert.h"
#include "debug_print.h"
#include "sprite_loading.h"

#define DEFERRED_DRAW_TASK_MAX 4

static SpriteDeferredDrawTask deferred_draw_tasks[DEFERRED_DRAW_TASK_MAX];
static uint32_t deferred_draw_index;

static SpriteActor actors[SPRITE_ACTOR_MAX];
static SpriteActorLight actors_light[SPRITE_ACTOR_LIGHT_MAX];

static SpriteActor *sa_alloc_impl(uint32_t base_index);

void sa_reset() {
    for (uint32_t i = 0; i < SPRITE_ACTOR_MAX; i++) {
        actors[i].handle = SA_HANDLE_FREE;
    }
    for (uint32_t i = 0; i < SPRITE_ACTOR_LIGHT_MAX; i++) {
        actors_light[i].handle = SA_LIGHT_HANDLE_FREE;
    }

    deferred_draw_index = 0;
}

// The handle is already set and should not be overwritten

void sa_init(SpriteActor *actor, SpriteActorMain main) {
    actor->main = main;
    actor->level_data_index = SPRITE_INDEX_UNDEFINED;

    actor->rideable = false;
    actor->can_ride = false;
    actor->ridden_sprite_handle = SA_HANDLE_FREE;
    
    actor->killed = false;
    actor->interacts_with_sprites = true;
    actor->stomp_hurts_hero = false;
    actor->dies_upon_stomp = true;
    actor->falls_off_when_killed = true;
    actor->kills_other_sprites = false;
    actor->can_be_carried = false;
    actor->carried = false;
    actor->touch_hurts_hero = true;
    actor->stompable = true;
    actor->thud_sound_upon_hitting_wall = false;
    actor->invert_velocity_upon_hitting_wall = true;
    actor->immune_to_projectiles = false;

    actor->position.x_full = 0;
    actor->position.y_full = 0;
    actor->velocity.x = 0;
    actor->velocity.y = 0;

    actor->debug_label = NULL;

    actor->direction = LEFT;
}

// It is up to the caller to do any followup initialization for a specific actor
SpriteActor *sa_alloc() {
    SpriteActor *actor = sa_alloc_impl(0);
    assert(actor);
    return actor;
}

SpriteActor *sa_alloc_after(SpriteActorHandle handle) {
    SpriteActor *actor = sa_alloc_impl(handle.index);
    assert(actor);
    return actor;
}

static SpriteActor *sa_alloc_impl(uint32_t base_index) {
    for (uint32_t i = base_index; i < SPRITE_ACTOR_MAX; i++) {
        SpriteActor *actor = &actors[i];
        if (!sa_live(actor)) {
            actor->handle.index = i;
            return actor;
        }
    }

    return NULL;
}

static void sa_run(const SpriteEnvironment *env, bool rideable_only) {
    for (uint32_t i = 0; i < SPRITE_ACTOR_MAX; i++) {
        SpriteActor *actor = &actors[i];
        if (!sa_live(actor)) {
            continue;
        }
        if (rideable_only != actor->rideable) {
            continue;
        }
        if (!sa_remove_if_outside_bounds(actor, env->camera)) {
            continue;
        }

        dbg_print_actor(actor);

        actor->main(actor, env);
    }
}

void sa_run_non_rideable(const SpriteEnvironment *env) {
    sa_run(env, false);
}

void sa_run_rideable(const SpriteEnvironment *env) {
    sa_run(env, true);
}

void sa_free(SpriteActor *actor) {
    dbg_actor_clear(actor);

    assert(sa_live(actor));
    
    actor->handle.index = SA_UNDEFINED;
    actor->handle.generation++;
}

bool sa_live(const SpriteActor *actor) {
    return (actor->handle.index != SA_UNDEFINED);
}

bool sa_handle_live(SpriteActorHandle handle) {
    uint8_t index = handle.index;
    if (index == SA_UNDEFINED) {
        return false;
    }

    const SpriteActor *actor = &actors[index];
    if (actor->handle.index == SA_UNDEFINED) {
        return false;
    }

    return (actors[index].handle.generation == handle.generation);
}

bool sa_handle_equal(SpriteActorHandle h1, SpriteActorHandle h2) {
    return (h1.index == h2.index && h1.generation == h2.generation);
}

void sa_handle_clear(SpriteActorHandle *handle) {
    handle->index = SA_UNDEFINED;
}

bool sa_level_index_live(int8_t level_index) {
    for (uint32_t i = 0; i < SPRITE_ACTOR_MAX; i++) {
        SpriteActor *actor = &actors[i];
        if (sa_live(actor) && actor->level_data_index == level_index) {
            return true;
        }
    }

    return false;
}

SpriteActor *sa_index(uint8_t index) {
    SpriteActor *actor = &actors[index];
    return (sa_live(actor) ? actor : NULL);
}

SpriteActor *sa_get(SpriteActorHandle handle) {
    assert(sa_handle_live(handle));

    return sa_index(handle.index);
}

// Deferred draw handling:

void sa_add_deferred_draw_task(SpriteActor *actor, SpriteDeferredDrawFunction function) {
    assert(deferred_draw_index < DEFERRED_DRAW_TASK_MAX);

    SpriteDeferredDrawTask task = {
        .actor = actor,
        .function = function
    };

    deferred_draw_tasks[deferred_draw_index++] = task;
}

void sa_run_deferred_draw_tasks(SpriteEnvironment *environment) {
    for (uint32_t i = 0; i < deferred_draw_index; i++) {
        SpriteDeferredDrawTask task = deferred_draw_tasks[i];
        task.function(task.actor, environment);
    }

    deferred_draw_index = 0;
}

// Light actors:

void sa_init_light(SpriteActorLight *actor, SpriteActorLightMain main) {
    actor->main = main;

    actor->killed = false;
    actor->interacts_with_sprites = true;
    actor->falls_off_when_killed = true;
    actor->kills_other_sprites = false;
    actor->hurts_hero = false;

    actor->position.x_full = 0;
    actor->position.y_full = 0;
    actor->velocity.x = 0;
    actor->velocity.y = 0;

    actor->direction = LEFT;
}

SpriteActorLight *sa_alloc_light() {
    for (uint32_t i = 0; i < SPRITE_ACTOR_LIGHT_MAX; i++) {
        SpriteActorLight *actor = &actors_light[i];
        if (!sa_live_light(actor)) {
            actor->handle.index = i;
            return actor;
        }
    }

    return NULL;
}

void sa_free_light(SpriteActorLight *actor) {
    assert(sa_live_light(actor));

    actor->handle.index = SA_UNDEFINED;
    actor->handle.generation++;
}

bool sa_live_light(const SpriteActorLight *actor) {
    return (actor->handle.index != SA_UNDEFINED);
}

bool sa_handle_live_light(SpriteActorLightHandle handle) {
    uint8_t index = handle.index;
    if (index == SA_UNDEFINED) {
        return false;
    }

    const SpriteActorLight *actor = &actors_light[index];
    if (actor->handle.index == SA_UNDEFINED) {
        return false;
    }

    return (actors_light[index].handle.generation == handle.generation);
}

void sa_run_light(const SpriteEnvironment *env) {
    // Precompute absolute bounding boxes of regular sprites as they shouldn't change due to light actors
    // Regular sprites can't benefit from this since they can change each others positions unlike light actors
    for (uint32_t i = 0; i < SPRITE_ACTOR_MAX; i++) {
        SpriteActor *actor = &actors[i];
        if (!sa_live(actor)) {
            continue;
        }

        sa_bounding_box_abs(&actor->position, &actor->bounding_box, &actor->bounding_box_abs);
    }

    for (uint32_t i = 0; i < SPRITE_ACTOR_LIGHT_MAX; i++) {
        SpriteActorLight *actor = &actors_light[i];
        if (!sa_live_light(actor)) {
            continue;
        }
        if (!sa_within_live_bounds(&actor->position, env->camera)) {
            sa_free_light(actor);
            continue;
        }

        actor->main(actor, env);
    }
}

// Vehicle (could have separate file for this, pretty specific)

// Not going to spend any extra struct fields for this dynamic dispatch
// Just using the main function pointer as the "class" ID as they're unique anyway

void sa_hero_vehicle_update(SpriteActor *vehicle, Hero *hero, SpriteVehicleHeroContext *hero_context) {
    const uintptr_t tank = (uintptr_t)tank_sprite_main;
    uintptr_t vehicle_main = (uintptr_t)vehicle->main;

    if (vehicle_main == tank) {
        tank_sprite_hero_drive_update(vehicle, hero, hero_context);
    } else {
        fatal_error("This actor isn't a vehicle");
    }
}

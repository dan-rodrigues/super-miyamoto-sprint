#ifndef sprite_actor_types_h
#define sprite_actor_types_h

#include <stdint.h>
#include <stdbool.h>

#include "hero_types.h"
#include "camera_types.h"
#include "sprite_actor_handle.h"

typedef struct SpriteEnvironment SpriteEnvironment;
typedef struct SpriteActor SpriteActor;
typedef void (*SpriteActorMain)(SpriteActor *self, const SpriteEnvironment *env);
typedef bool (*SpriteActorIteratorCallback)(SpriteActor *actor, SpriteActor *caller);
typedef struct SpriteVehicleHeroContext SpriteVehicleHeroContext;

// These happen to be the same for now but are kept separate
typedef void (*SpriteDeferredDrawFunction)(SpriteActor *self, const SpriteEnvironment *env);
typedef struct SpriteDeferredDrawTask SpriteDeferredDrawTask;

typedef struct SpriteActorLight SpriteActorLight;
typedef void (*SpriteActorLightMain)(SpriteActorLight *self, const SpriteEnvironment *env);

#endif /* sprite_actor_types_h */

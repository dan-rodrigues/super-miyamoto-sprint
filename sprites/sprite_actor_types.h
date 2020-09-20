#ifndef sprite_actor_types_h
#define sprite_actor_types_h

#include <stdint.h>

#include "hero_types.h"
#include "camera_types.h"

typedef struct SpriteEnvironment SpriteEnvironment;
typedef struct SpriteActor SpriteActor;
typedef void (*SpriteActorMain)(SpriteActor *self, const SpriteEnvironment *env);

// These happen to be the same for now but are kept separate
typedef void (*SpriteDeferredDrawFunction)(SpriteActor *self, const SpriteEnvironment *env);
typedef struct SpriteDeferredDrawTask SpriteDeferredDrawTask;

typedef struct SpriteActorLight SpriteActorLight;
typedef struct SpriteActorHandle SpriteActorHandle;
typedef struct SpriteActorLightHandle SpriteActorLightHandle;
typedef void (*SpriteActorLightMain)(SpriteActorLight *self, const SpriteEnvironment *env);

#endif /* sprite_actor_types_h */

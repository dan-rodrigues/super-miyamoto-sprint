#ifndef sprite_actor_handle_h
#define sprite_actor_handle_h

#include <stdint.h>

typedef struct {
    uint8_t index;
    uint8_t generation;
} SpriteActorHandle;

typedef struct {
    uint8_t index;
    uint8_t generation;
} SpriteActorLightHandle;

extern const SpriteActorHandle SA_HANDLE_FREE;
extern const SpriteActorLightHandle SA_LIGHT_HANDLE_FREE;

extern const uint8_t SA_UNDEFINED;

#endif /* sprite_actor_handle_h */

#include "sprite_actor_handle.h"

const uint8_t SA_UNDEFINED = UINT8_MAX;

const SpriteActorHandle SA_HANDLE_FREE = {.index = SA_UNDEFINED, .generation = 0};
const SpriteActorLightHandle SA_LIGHT_HANDLE_FREE = {.index = SA_UNDEFINED, .generation = 0};

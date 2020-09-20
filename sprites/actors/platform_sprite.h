#ifndef platform_sprite_h
#define platform_sprite_h

#include <stdint.h>
#include <stdbool.h>

#include "sprite_actor_types.h"
#include "sprite_position.h"

typedef enum {
    PLATFORM_MOTION_AUTO_X,
    PLATFORM_MOTION_AUTO_Y,
    PLATFORM_MOTION_NONE,
    PLATFORM_MOTION_GAMEPAD
} PlatformSpriteMotion;

typedef struct {
    SpriteOffset frame_offset;
    PlatformSpriteMotion motion;
    int16_t lower_target_bound;
    int16_t upper_target_bound;
    bool travelling_towards_lower_bound;
} PlatformSprite;

void platform_sprite_main(SpriteActor *self, const SpriteEnvironment *env);
SpriteActor *platform_sprite_init(const SpritePosition *position, PlatformSpriteMotion motion);

#endif /* platform_sprite_h */

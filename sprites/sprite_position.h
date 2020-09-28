#ifndef sprite_position_h
#define sprite_position_h

#include <stdint.h>
#include <stdbool.h>

#include "sprite_position_types.h"
#include "sprite_actor_types.h"

extern const int32_t Q_1;

enum SpriteDirection {
    LEFT, RIGHT
};

struct SpritePosition {
    union {
        // LE byte ordering assumed
        struct {
            uint16_t x_fraction;
            int16_t x;
            uint16_t y_fraction;
            int16_t y;
        };
        struct {
            int32_t x_full, y_full;
        };
    };
};

struct SpriteOffset {
    int16_t x, y;
};

struct SpriteVelocity {
    int32_t x, y;
};

struct SpriteBoundingBoxOffset {
    int8_t x, y;
};

struct SpriteBoundingBoxSize {
    int8_t width, height;
};

struct SpriteBoundingBox {
    SpriteBoundingBoxOffset offset;
    SpriteBoundingBoxSize size;
};

struct SpriteBoundingBoxAbs {
    int32_t top, left, bottom, right;
};

SpriteDirection sa_direction_flipped(SpriteDirection direction);

void sa_apply_velocity(const SpriteVelocity *velocity, SpritePosition *position);
void sa_apply_velocity_with_gravity(SpriteActor *actor, const SpriteBoundingBox *box);
void sa_apply_horizontal_block_interaction_updates(SpriteActor *actor, const SpriteBoundingBox *box);

bool sa_perform_default_movement(SpriteActor *actor);
bool sa_perform_carryable_movement(SpriteActor *actor, const Hero *hero);

bool sa_within_live_bounds(const SpritePosition *position, const Camera *camera);
bool sa_remove_if_outside_bounds(SpriteActor *actor, const Camera *camera);

void sa_bounding_box_abs(const SpritePosition *position, const SpriteBoundingBox *box,
                                 SpriteBoundingBoxAbs *abs);

void sa_bounding_box_overlap(const SpriteBoundingBoxAbs *a1,
                             const SpriteBoundingBoxAbs *a2,
                             SpriteBoundingBoxAbs *result);

void sa_bounding_box_center(const SpriteBoundingBoxAbs *box_abs, SpritePosition *center);

bool sa_platform_ride_check(SpriteActor *platform,
                            SpriteVelocity *velocity,
                            SpritePosition *position,
                            SpriteActorHandle *rider_handle);
void sa_platform_dismount(const SpriteActor *platform, SpriteVelocity *velocity, SpriteActorHandle *rider_handle);

void sa_apply_offset(const SpriteOffset *offset, SpritePosition *position);

void sa_grounded_update(SpriteActor *actor, int32_t displacement);

void sa_horizontal_acceleration(SpriteVelocity *velocity, SpriteDirection direction, int32_t accel, int32_t max_speed);
void sa_horizontal_deceleration(SpriteVelocity *velocity, int32_t decel);

int32_t sa_velocity_from_speed(int32_t speed, SpriteDirection direction);

#endif /* sprite_position_h */

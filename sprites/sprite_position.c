#include "sprite_position.h"

#include "math_util.h"
#include "assert.h"

#include "sprite_actor.h"
#include "sprite_block_interaction.h"
#include "vdp.h"
#include "camera.h"
#include "sound_effects.h"

#include "hero.h"

const int32_t Q_1 = 0x10000;

static bool onscreen(int32_t screen_x,
                     int32_t screen_y,
                     int32_t padding_x,
                     int32_t padding_top,
                     int32_t padding_bottom);

void sa_apply_velocity(const SpriteVelocity *velocity, SpritePosition *position) {
    position->x_full += velocity->x;
    position->y_full += velocity->y;
}

void sa_apply_velocity_with_gravity(SpriteActor *actor, const SpriteBoundingBox *box) {
    // Based on values for Hero
    const int32_t gravity_accel = Q_1 / 4;
    const int32_t fall_speed_max = 5 * Q_1;

    // Only apply gravity if sprite isn't on a platform

    SpriteActorHandle platform_handle = actor->ridden_sprite_handle;
    if (!sa_handle_live(platform_handle)) {
        actor->velocity.y += gravity_accel;
        actor->velocity.y = MIN(actor->velocity.y, fall_speed_max);
    } else {
        const PlatformSprite *platform = &sa_get(platform_handle)->platform;
        sa_apply_offset(&platform->frame_offset, &actor->position);
    }

    sa_apply_velocity(&actor->velocity, &actor->position);

    // Inside-bottom check (based on hero)

    SpriteBlockInteractionResult bottom_result;
    if (sa_inside_block_bottom(&actor->position, box, &bottom_result)) {
        actor->position.y += bottom_result.overlap;

        if (actor->velocity.y < 0) {
            actor->velocity.y = 0;
        }
    }

    // Grounding check since we're also applying gravity here

    if (sa_block_ground_test(&actor->position, &actor->velocity, box, NULL)) {
        // (This is based on Hero, could potentially reuse this)
        int32_t displacement = actor->position.y & 15;
        actor->position.y = actor->position.y & ~15;
        actor->position.y_fraction = 0;

        sa_grounded_update(actor, displacement);
    }
}

void sa_grounded_update(SpriteActor *actor, int32_t displacement) {
    // An inert carryable sprite has a default bounce when it hits the ground
    bool should_bounce = actor->can_be_carried && (actor->velocity.y > Q_1);
    if (should_bounce) {
        // The sprite may have "entered the ground" so compensate for this
        actor->position.y -= displacement;
        actor->velocity.y = -actor->velocity.y / 4;
    } else {
        actor->velocity.y = 0;
    }

    // Carryable sprites also maintain horizontal inertia unless they're on ground
    // (optional extra is a little smoke effect at the base when sliding)
    if (actor->can_be_carried) {
        const int32_t horizontal_decel = Q_1 / 2;

        int32_t decel = (actor->velocity.x > 0) ? horizontal_decel : -horizontal_decel;
        actor->velocity.x -= decel;

        if (SIGN(decel) != SIGN(actor->velocity.x)) {
            actor->velocity.x = 0;
        }
    }
}

void sa_apply_horizontal_block_interaction_updates(SpriteActor *actor,
                                                   const SpriteBoundingBox *box)
{
    SpriteBlockInteractionResult left_result, right_result;
    sa_inside_block_horizontal(&actor->position, box, &left_result, &right_result);

    bool mirror_x_velocity = false;

    if (left_result.solid) {
        actor->position.x -= left_result.overlap;
        actor->direction = LEFT;
        mirror_x_velocity = (actor->velocity.x > 0);
    } else if (right_result.solid) {
        actor->position.x += right_result.overlap;
        actor->direction = RIGHT;
        mirror_x_velocity = (actor->velocity.x < 0);
    }

    if (mirror_x_velocity) {
        actor->velocity.x = -actor->velocity.x;
        if (actor->thud_sound_upon_hitting_wall) {
            se_thud();
        }
    }
}

bool sa_within_live_bounds(const SpritePosition *position, const Camera *camera) {
    const int32_t live_bounds = 64;
    // Allowed distance off bottom of screen
    const int32_t padding_bottom = 16;
    // Allowed distance off top of screen (i.e. item that is kicked up)
    const int32_t padding_top = 300;

    int32_t x = position->x - camera->scroll.x;
    int32_t y = position->y - camera->scroll.y;

    return onscreen(x, y, live_bounds, padding_top, padding_bottom);
}

// (light variant needed at some point)
bool sa_remove_if_outside_bounds(SpriteActor *actor, const Camera *camera) {
    bool in_bounds = sa_within_live_bounds(&actor->position, camera);

    if (!in_bounds) {
        sa_free(actor);
    }

    return in_bounds;
}

// Returns true is a default movement was done
// The actor shouldn't try its own movement if so

bool sa_perform_default_movement(SpriteActor *actor) {
    if (!(actor->killed && actor->falls_off_when_killed)) {
        return false;
    }

    // Sprite is dead and falling off the screen
    const int32_t max_fall_speed = Q_1 * 4;
    const int32_t fall_accel = Q_1 / 8;

    actor->velocity.x = 0;
    actor->velocity.y = MIN(max_fall_speed, actor->velocity.y + fall_accel);
    sa_apply_velocity(&actor->velocity, &actor->position);

    return true;
}

bool sa_perform_carryable_movement(SpriteActor *actor, const Hero *hero) {
    // Is this particular hero carrying this sprite?
    if (!hero_carrying_sprite(hero, actor->handle)) {
        return false;
    }
    // ..if so, its own flag should be set by the time we get here
    assert(actor->carried);

    // Position "in hero's hands", wherever that happens to be
    HeroSpriteCarryContext context = hero_sprite_carry_context(hero);
    actor->position = context.position;
    actor->direction = context.direction;

    actor->velocity.x = 0;
    actor->velocity.y = 0;

    return true;
}

void sa_bounding_box_abs(const SpritePosition *position,
                         const SpriteBoundingBox *box,
                         SpriteBoundingBoxAbs *abs)
{
    abs->top = position->y + box->offset_y;
    abs->bottom = position->y + box->offset_y + box->height;
    abs->left = position->x + box->offset_x;
    abs->right = position->x + box->offset_x + box->width;
}

void sa_bounding_box_overlap(const SpriteBoundingBoxAbs *a1,
                             const SpriteBoundingBoxAbs *a2,
                             SpriteBoundingBoxAbs *result)
{
    result->top = MAX(a1->top, a2->top);
    result->bottom = MIN(a1->bottom, a2->bottom);
    result->left = MAX(a1->left, a2->left);
    result->right = MIN(a1->right, a2->right);
}

void sa_bounding_box_center(const SpriteBoundingBoxAbs *box_abs, SpritePosition *center) {
    center->x = box_abs->left + (box_abs->right - box_abs->left) / 2;
    center->y = box_abs->top + (box_abs->bottom - box_abs->top) / 2;
}

// Updates the sprite (or Hero) assuming that it overlaps with a rideable platform
// It may or may not start riding the platform based on other checks
// This function should only be called if the sprites are actually colliding

bool sa_platform_ride_check(SpriteActor *platform,
                            SpriteVelocity *velocity,
                            SpritePosition *position,
                            SpriteActorHandle *rider_handle)
{
    bool should_ride = (velocity->y >= 0);

    // Sprite's feet must be within this number of pixels from the top of the platform
    // The faster sprites can fall onto platform, the higher this needs to be..
    // ..or else sprites will just fall through the platform
    const int16_t floor_padding = 7;
    int16_t platform_height = platform->bounding_box.height;
    should_ride &= ((position->y + platform_height) - platform->position.y - floor_padding) < 0;

    if (should_ride) {
        *rider_handle = platform->handle;

        // Fix Y position to the platform

        // Note velocity->y is not set here, it is left to the caller
        // This makes bounce effects etc. less complicated

        position->y = platform->position.y - platform_height + 1;
    } else if (sa_handle_equal(*rider_handle, platform->handle)) {
        sa_platform_dismount(platform, velocity, rider_handle);
    }

    return should_ride;
}

void sa_platform_dismount(const SpriteActor *platform, SpriteVelocity *velocity, SpriteActorHandle *rider_handle) {
    // This applies the platforrm velocity when rider dismounts
    // May or may not take effect depending on the sprite
    velocity->x += platform->velocity.x;
    sa_handle_clear(rider_handle);
}

void sa_apply_offset(const SpriteOffset *offset, SpritePosition *position) {
    position->x += offset->x;
    position->y += offset->y;
}

static bool onscreen(int32_t screen_x,
                     int32_t screen_y,
                     int32_t padding_x,
                     int32_t padding_top,
                     int32_t padding_bottom)
{
    return
        (screen_x + padding_x >= 0) &&
        (screen_x - padding_x < SCREEN_ACTIVE_WIDTH) &&
        (screen_y + padding_top >= 0) &&
        (screen_y - padding_bottom < SCREEN_ACTIVE_HEIGHT);
}

#include <stddef.h>

#include "hero.h"

#include "math_util.h"
#include "assert.h"
#include "sound_effects.h"

#include "smoke_sprite.h"
#include "sprite_block_interaction.h"
#include "sprite_collision.h"
#include "debug_print.h"

const HeroFrame HERO_IDLE_FRAME = RUN2;

const uint8_t HERO_SPRINT_CHARGE_TIME = 60;
const uint8_t HERO_INVULNERABILITY_COUNT = 60 * 2;
const uint8_t HERO_DEFAULT_LIFE = 3;

typedef struct {
    bool vine;
    bool hurt;
    bool grounded;
} HeroBlockAttributesProcessedContext;

static const HeroFrame HERO_IDLE_CARRYING_FRAME = CARRY2;
static const HeroFrame HERO_JUMP_CARRYING_FRAME = CARRY0;

static const SpriteBoundingBox *block_horizontal_bounding_box(const Hero *hero);
static const SpriteBoundingBox *block_vertical_bounding_box(const Hero *hero);

static void precompute_bounding_box(Hero *hero);
static void update_frame(Hero *hero);
static bool riding_platform(Hero *hero);
static void sprite_carry_check(Hero *hero);
static void handle_damage(Hero *hero);

static void handle_standard_movement(Hero *hero);
static void handle_climbing_movement(Hero *hero);
static void handle_vehicle_movement(Hero *hero);

static void horizontal_block_displacement(Hero *hero, const SpriteBlockInteractionResult *result, bool left);

static void update_from_block_interaction(Hero *hero, const HeroBlockAttributesProcessedContext *context);
static void perform_extra_block_interaction(Hero *hero, const SpriteBlockInteractionResult *result, HeroBlockAttributesProcessedContext *context, const Camera *camera);

static void handle_standard_motion_update(Hero *hero) {
    const PadInputDecoded *pad = &hero->pad;
    const PadInputDecoded *pad_edge = &hero->pad_edge;

    SpriteVelocity *velocity = &hero->velocity;
    SpritePosition *position = &hero->position;

    // Gravity

    const int32_t gravity_accel_high = Q_1 / 2;
    const int32_t gravity_accel_low = Q_1 / 6;
    const int32_t fall_speed_max = 5 * Q_1;

    // Holding button B sustains the jump for longer

    if (!hero->climbing && !riding_platform(hero)) {
        bool sustain_jump = (pad->b || pad->a || pad->l);
        int32_t gravity_delta = (sustain_jump ? gravity_accel_low : gravity_accel_high);

        velocity->y += gravity_delta;
        velocity->y = MIN(velocity->y, fall_speed_max);
    }

    // Airborne bounce (such as bouncing off an enemy)

    if (hero->airborne_bounce_pending) {
        const int32_t bounce_initial_speed = 5 * Q_1;

        velocity->y = -bounce_initial_speed;
        hero->airborne_bounce_pending = false;
    }

    // Jumping

    if ((hero->grounded || hero->climbing) && pad_edge->b) {
        const int32_t jump_initial_speed = 5 * Q_1;

        int32_t jump_running_added_speed = ABS(velocity->x) / 8;

        velocity->y = -(jump_initial_speed + jump_running_added_speed);
        hero->sprint_jumping = (hero->sprinting && !hero->climbing);
        hero->climbing = false;

        se_jump();
    }

    // Update hero position according to newly set velocity

    sa_apply_velocity(velocity, position);

    // ..then apply and offsets from ridden platform if applicable
    if (riding_platform(hero)) {
        const PlatformSprite *platform = &sa_get(hero->platform_sprite_handle)->platform;
        sa_apply_offset(&platform->frame_offset, &hero->position);
    }
}

static void iterate_block_interaction(
                                      Hero *hero,
                                      const Camera *camera,
                                      const SpriteBoundingBox *horizontal_box,
                                      const SpriteBoundingBox *vertical_box,
                                      HeroBlockAttributesProcessedContext *block_context)
{
    SpriteVelocity *velocity = &hero->velocity;
    SpritePosition *position = &hero->position;

    const PadInputDecoded *pad = &hero->pad;

    // Did hero bump up against a solid block?
    SpriteBlockInteractionResult block_result;
    bool head_inside = sa_inside_block_bottom(&hero->position, vertical_box, &block_result);

    if (head_inside) {
        // Immediately move hero below the block
        position->y += block_result.overlap;

        if (velocity->y < 0) {
            velocity->y = 0;
        }

        if (!hero->climbing) {
            se_thud();
        }
    }

    // Head block interaction
    perform_extra_block_interaction(hero, &block_result, block_context, camera);

    // Can be grounded by standing on a level block..
    bool level_grounded = sa_block_ground_test(&hero->position, &hero->velocity, vertical_box, &block_result);
    block_context->grounded = level_grounded;

    // ..or a rideable sprite
    bool sprite_grounded = riding_platform(hero);

    bool grounded = level_grounded | sprite_grounded;
    hero->grounded |= grounded;

    // Foot block interaction
    perform_extra_block_interaction(hero, &block_result, block_context, camera);

    if (level_grounded) {
        position->y = position->y & ~15;
        position->y_fraction = 0;

        velocity->y = 0;
        hero->sprint_jumping = false;

        hero->climbing = false;
    }

    // Is hero horizontally inside a solid block?

    // This must come AFTER the above grounding displacement
    // Jumping into the corner of a block will otherwise stop the hero abruptly

    SpriteBlockInteractionResult left_result, right_result;
    SpriteBlockInteractionResult left_result_padded, right_result_padded;

    SpriteBoundingBox padded_box = *horizontal_box;
    padded_box.size.width += 2;
    padded_box.offset.x -= 1;

    sa_inside_block_horizontal(&hero->position, horizontal_box, &left_result, &right_result);
    sa_inside_block_horizontal(&hero->position, &padded_box, &left_result_padded, &right_result_padded);

    // Displace hero horizontally from a block, if needed

    horizontal_block_displacement(hero, &right_result, false);
    horizontal_block_displacement(hero, &left_result, true);

    // Is hero trying to walk into a solid wall?

    bool walking_into_block_from_left = (left_result.solid || left_result_padded.solid) && pad->right;
    bool walking_into_block_from_right = (right_result.solid || right_result_padded.solid || right_result_padded.level_bound_touched) && pad->left;

    hero->against_solid_block |= (walking_into_block_from_left || walking_into_block_from_right);

    // Left/right block interaction
    perform_extra_block_interaction(hero, &left_result, block_context, camera);
    perform_extra_block_interaction(hero, &right_result, block_context, camera);
}

void hero_update_state(Hero *hero, const Camera *camera) {
    const PadInputDecoded *pad = &hero->pad;

    handle_damage(hero);
    sprite_carry_check(hero);

    HeroBlockAttributesProcessedContext block_context = {
        .vine = false,
        .hurt = false,
        .grounded = false
    };

    // Movement depending on current state

    if (hero_in_any_vehicle(hero)) {
        handle_vehicle_movement(hero);
    } else if (hero->climbing) {
        handle_climbing_movement(hero);
    } else {
        handle_standard_movement(hero);
    }

    handle_standard_motion_update(hero);

    if (hero_in_any_vehicle(hero)) {
        SpriteActor *vehicle = sa_get(hero->vehicle_sprite_handle);
        vehicle->position = hero->position;
    }

    hero->grounded = false;
    hero->against_solid_block = false;

    // Block interaction using a variable number of bounding boxes:

    uint32_t box_count = 0;
    const SpriteBoundingBox *horizontal_boxes;
    const SpriteBoundingBox *vertical_boxes;

    if (hero_in_any_vehicle(hero)) {
        horizontal_boxes = hero->vehicle_context.horizontal_block_boxes;
        vertical_boxes = hero->vehicle_context.vertical_block_boxes;
        box_count = hero->vehicle_context.box_count;
    } else {
        horizontal_boxes = block_horizontal_bounding_box(hero);
        vertical_boxes = block_vertical_bounding_box(hero);
        box_count = 1;
    }

    for (uint32_t i = 0; i < box_count; i++) {
        iterate_block_interaction(hero, camera, &horizontal_boxes[i], &vertical_boxes[i], &block_context);
    }

    // All block processing should be done by now

    update_from_block_interaction(hero, &block_context);

    // Ducking?

    if (hero->grounded) {
        hero->ducking = pad->down;
    }

    // Frame and animation:

    bool quick_turn = (pad->right && hero->direction == LEFT);
    quick_turn |= (pad->left && hero->direction == RIGHT);
    hero->quick_turning = quick_turn;

    // Smoke effect when sliding (these should be "secondary" actors at some point)

    if (quick_turn && hero->grounded) {
        SpritePosition smoke_position = hero->position;
        smoke_position.x -= 1;
        smoke_position.y += 4;
        smoke_sprite_init(&smoke_position, SMOKE_SMALL);
    }

    // Invulnerability update

    if (hero->invulnerability_counter > 0) {
        hero->invulnerability_counter--;
        hero->visible = hero->invulnerability_counter % 2;
    } else {
        hero->visible = true;
    }

    // Precompute bounding box ONCE for this state update
    // No other position hero updates should occur beyond this point
    precompute_bounding_box(hero);

    // Frame selection based on (updated) state
    update_frame(hero);
}

static void handle_standard_movement(Hero *hero) {
    const PadInputDecoded *pad = &hero->pad;

    SpriteVelocity *velocity = &hero->velocity;

    // Walking left/right

    const int32_t walk_max_speed = 1 * Q_1;
    const int32_t run_max_speed = 2 * Q_1;
    const int32_t sprint_max_speed = 2 * Q_1 + Q_1 / 2;

    const int32_t walk_correction_delta = Q_1 / 6;
    const int32_t run_correction_delta = Q_1 / 4;
    const int32_t speed_down_delta = Q_1 / 10;

    const int32_t walk_idle_delta = Q_1 / 12;
    const int32_t run_idle_delta = Q_1 / 10;

    int32_t max_speed = pad->y ? run_max_speed : walk_max_speed;
    if (hero->sprinting) {
        max_speed = sprint_max_speed;
    }

    int32_t idle_delta = pad->y ? run_idle_delta : walk_idle_delta;
    int32_t correction_delta = pad->y ? run_correction_delta : walk_correction_delta;

    // Does hero need directional input update?

    bool handle_horizontal_movement = false;
    bool invert = false;

    if (!hero->ducking || !hero->grounded) {
        if (pad->left && !pad->right) {
            handle_horizontal_movement = true;
            invert = true;
        } else if (pad->right && !pad->left) {
            handle_horizontal_movement = true;
        }
    } else {
        if (pad->left != pad->right) {
            hero->direction = pad->left ? LEFT : RIGHT;
        }
    }

    // Sprint if hero has been running for long enough

    if (handle_horizontal_movement && pad->y) {
        if (hero->grounded) {
            hero->sprint_charge_counter = MIN(hero->sprint_charge_counter + 1, HERO_SPRINT_CHARGE_TIME);
        } else if (!hero->sprinting) {
            // Jumping while running will clear the sprint counter *unless* already sprinting
            hero->sprint_charge_counter = 0;
        }
    } else {
        hero->sprint_charge_counter = 0;
    }

    hero->sprinting = (hero->sprint_charge_counter == HERO_SPRINT_CHARGE_TIME);

    // Handle directional update if needed

    if (handle_horizontal_movement) {
        // Strip velocity of its direction (which is just the sign bit)
        // This leaves us with a directionaless speed component
        int32_t x_speed = invert ? -velocity->x : velocity->x;

        if (x_speed < max_speed) {
            // Player can speed up
            x_speed += x_speed < 0 ? correction_delta : idle_delta;
            // ..up to a given max speed
            x_speed = MIN(max_speed, x_speed);
        } else {
            // Player moving faster than max speed, so gently reduce to max speed.
            x_speed -= speed_down_delta;
            // ..but make sure hero doesn't go *below* the max speed
            x_speed = MAX(max_speed, x_speed);
        }

        if (x_speed > 0) {
            hero->direction = invert ? LEFT : RIGHT;
        }

        velocity->x = invert ? -x_speed : x_speed;
    } else {
        hero->sprint_charge_counter = 0;

        // Horizontal updates if there is no input otherwise
        int32_t idle_delta = hero->ducking ? correction_delta : walk_idle_delta;

        int32_t abs_velocity = ABS(velocity->x);
        abs_velocity -= idle_delta;
        abs_velocity = (SIGN(abs_velocity) ? 0 : abs_velocity);
        velocity->x = (SIGN(velocity->x) ? -abs_velocity : abs_velocity);
    }
}

static void handle_climbing_movement(Hero *hero) {
    const PadInputDecoded *pad = &hero->pad;

    SpriteVelocity *velocity = &hero->velocity;

    velocity->x = 0;
    velocity->y = 0;

    const int32_t climb_speed = Q_1;

    if (pad->up) {
        velocity->y = -climb_speed;
    }
    if (pad->down) {
        velocity->y = climb_speed;
    }

    if (pad->left) {
        velocity->x -= climb_speed;
        hero->direction = LEFT;
    }
    if (pad->right) {
        velocity->x += climb_speed;
        hero->direction = RIGHT;
    }

    hero->moved_while_climbing = (pad->up || pad->down || pad->left || pad->right);

    hero->sprint_charge_counter = 0;
    hero->sprint_jumping = false;
}

static void handle_vehicle_movement(Hero *hero) {
    SpriteActor *vehicle = sa_get(hero->vehicle_sprite_handle);

    if (hero->pending_vehicle_entry) {
        // Snap hero's position to wherever the vehicle is right now
        hero->position = vehicle->position;
        hero->velocity = vehicle->velocity;
        hero->pending_vehicle_entry = false;
    }

    // Let vehicle do the updating
    // possibly pass in control context here to save the effort of doing it again later
    sa_hero_vehicle_update(vehicle, hero, &hero->vehicle_context);

    if (hero->velocity.x) {
        hero->direction = (hero->velocity.x < 0 ? LEFT : RIGHT);
    }
}

static void update_frame(Hero *hero) {
    bool carrying = sa_handle_live(hero->carried_sprite_handle);

    bool show_kick_frame = false;
    if (hero->kick_timer) {
        hero->kick_timer--;
        show_kick_frame = true;
    }

    if (hero_in_any_vehicle(hero)) {
        hero->frame = DRIVING;
    } else if (hero->ducking) {
        hero->frame = DUCKING;
    } else if (hero->climbing) {
        const int32_t climb_frame_duration = Q_1 * 8;

        if (hero->moved_while_climbing) {
            hero->frame_counter -= Q_1;
        }

        if (hero->frame_counter < 0) {
            hero->frame_counter = climb_frame_duration;
            hero->animation_counter++;
        }

        hero->frame = (hero->animation_counter % 2) ? CLIMBING0 : CLIMBING1;
    } else if (show_kick_frame) {
        hero->frame = KICK;
    } else if (hero->grounded) {
        const int32_t hero_default_frame_duration = 4 * Q_1;
        const int32_t hero_sprinting_frame_duration = 2 * Q_1;

        if (hero->animation_counter > 2) {
            hero->animation_counter = 0;
        }

        if (hero->quick_turning) {
            hero->frame = (carrying ? CARRY1 : RUN_TURNING);
            return;
        } else if (hero->against_solid_block || hero->velocity.x == 0) {
            hero->frame = (carrying ? HERO_IDLE_CARRYING_FRAME : HERO_IDLE_FRAME);
            hero->frame_counter = hero_default_frame_duration;
            hero->animation_counter = 0;
            return;
        }

        // Frame counter updating
        // This can be repurposed as a general purpose animation-loop for various frames

        if (hero->frame_counter < 0) {
            hero->frame_counter = hero->sprinting ? hero_sprinting_frame_duration : hero_default_frame_duration;

            bool last_frame = hero->animation_counter >= 2;
            hero->animation_counter = !last_frame ? hero->animation_counter + 1 : 0;
        } else {
            int32_t frame_counter_delta = ABS(hero->velocity.x);
            hero->frame_counter -= frame_counter_delta;
        }

        // Frame selection

        static const HeroFrame run_frames[] = {RUN0, RUN1, RUN2};
        static const HeroFrame sprint_frames[] = {SPRINT0, SPRINT1, SPRINT2};
        static const HeroFrame carry_frames[] = {CARRY0, CARRY1, CARRY2};

        const HeroFrame *frames = run_frames;
        if (carrying) {
            frames = carry_frames;
        } else if (hero->sprinting) {
            frames = sprint_frames;
        }

        hero->frame = frames[hero->animation_counter];
    } else {
        // Airborne
        if (carrying) {
            hero->frame = HERO_JUMP_CARRYING_FRAME;
        } else if (hero->sprint_jumping) {
            hero->frame = JUMP_SPRINTING;
        } else {
            hero->frame = (hero->velocity.y < 0 ? JUMP_RISING : JUMP_FALLING);
        }
    }
}

static void handle_damage(Hero *hero) {
    if (!hero->damage_pending) {
        return;
    }

    if (hero->life > 0) {
        // Hero is hurt but not dead
        hero->life--;
        hero->invulnerability_counter = HERO_INVULNERABILITY_COUNT;

        // Hero may need to hop depending on type of damage
        if (hero->damage_causes_hop) {
            const int32_t damage_hop_speed = Q_1 * 3;
            hero->velocity.y = -damage_hop_speed;
        }

        // Temporarily restore hero life, can't die just yet
        if (hero->life <= 0) {
            hero->life = HERO_DEFAULT_LIFE;
        }
    } else {
        // Hero is dead
        // TODO: handle
    }

    hero->damage_pending = false;
    hero->damage_causes_hop = false;

    se_hero_hurt();
}

static void horizontal_block_displacement(Hero *hero, const SpriteBlockInteractionResult *result, bool left) {
    if (result->solid || result->level_bound_touched) {
        const int32_t block_horizontal_ejection_delta = 1;

        // Push hero to the right, rather than teleporting
        hero->position.x += (left ? -block_horizontal_ejection_delta : block_horizontal_ejection_delta);

        // Only zero velocity if hero was moving into the block from the left
        if ((left ? -hero->velocity.x : hero->velocity.x) < 0) {
            hero->velocity.x = 0;
        }

        hero->sprinting = false;
        hero->sprint_charge_counter = 0;
    }
}

const SpriteBoundingBox *hero_sprite_bounding_box(const Hero *hero) {
    static const SpriteBoundingBox hero_box = {
        .offset = { -8, -20},
        .size = { 12, 20}
    };

    return &hero_box;
}

static const SpriteBoundingBox *block_horizontal_bounding_box(const Hero *hero) {
    static const SpriteBoundingBox full_size_box = {
        .offset = { -7, -20 },
        .size = { 7 + 6, 16 }
    };

    static const SpriteBoundingBox ducking_box = {
        .offset = { -7, -8 },
        .size = { 7 + 6, 4 }
    };

    if (!hero->ducking) {
        return &full_size_box;
    } else {
        return &ducking_box;
    }
}

static const SpriteBoundingBox *block_vertical_bounding_box(const Hero *hero) {
    static const SpriteBoundingBox full_size_box = {
        .offset = { -4, -24 },
        .size = {4 + 3, 24 }
    };

    static const SpriteBoundingBox ducking_box = {
        .offset = { -4, -13 },
        .size = { 4 + 3, 13 }
    };

    if (!hero->ducking) {
        return &full_size_box;
    } else {
        return &ducking_box;
    }
}

// Sprite carrying

void sprite_carry_check(Hero *hero) {
    if (!sa_handle_live(hero->carried_sprite_handle)) {
        return;
    }

    SpriteActor *actor = sa_get(hero->carried_sprite_handle);
    if (!actor->killed && actor->can_be_carried) {
        return;
    }

    // This sprite shouldn't be carried anymore

    actor->carried = false;
    sa_handle_clear(&hero->carried_sprite_handle);
}

// It might be worth later sending a should-carry event to hero
// otherwise sprite will be positioned on hero 1 frame before hero actually appears to hold it

HeroSpriteCarryUpdateResult hero_sprite_carry_update(Hero *hero, const SpriteActor *actor) {
    if (!actor->can_be_carried) {
        return HERO_SPRITE_CARRY_UPDATE_NOT_CARRYING;
    }

    const PadInputDecoded *pad = &hero->pad;

    // redundant work? could pass in a collides bool
    bool collides = sa_has_hero_collision(actor, hero);

    bool carried_sprite_live = sa_handle_live(hero->carried_sprite_handle);

    bool carrying_this_sprite = sa_handle_equal(actor->handle, hero->carried_sprite_handle);
    carrying_this_sprite &= carried_sprite_live;

    bool carrying_other_sprite = !carrying_this_sprite;
    carrying_other_sprite &= carried_sprite_live;

    bool should_carry = (pad->y && !hero->climbing);

    // If another sprite is already being carried, all other carryable sprites are ignored
    if (carrying_other_sprite) {
        return HERO_SPRITE_CARRY_UPDATE_NOT_CARRYING;
    }

    if (carrying_this_sprite && !should_carry) {
        sa_handle_clear(&hero->carried_sprite_handle);

        // Drop it or kick it depending on inputs
        bool kick_left_right = (pad->left && hero->direction == LEFT);
        kick_left_right |= (pad->right && hero->direction == RIGHT);

        bool kick_up = pad->up;

        if (kick_up || kick_left_right) {
            const uint8_t kick_frame_duration = 15;
            hero->kick_timer = kick_frame_duration;
        }

        if (kick_up) {
            return HERO_SPRITE_CARRY_UPDATE_KICKED_UP;
        } else if (kick_left_right) {
            return HERO_SPRITE_CARRY_UPDATE_KICKED;
        } else {
            return HERO_SPRITE_CARRY_UPDATE_DROPPED;
        }
    }

    if (should_carry && (actor->carried || collides)) {
        // Hero is carrying this sprite now
        hero->carried_sprite_handle = actor->handle;
        return HERO_SPRITE_CARRY_UPDATE_CARRYING;
    } else {
        // This might drop a different sprite but it shouldn't be carried regardless
        sa_handle_clear(&hero->carried_sprite_handle);
        return HERO_SPRITE_CARRY_UPDATE_NOT_CARRYING;
    }
}

bool hero_carrying_sprite(const Hero *hero, SpriteActorHandle handle) {
    return sa_handle_equal(handle, hero->carried_sprite_handle);
}

HeroSpriteCarryContext hero_sprite_carry_context(const Hero *hero) {
    const SpriteOffset stand_offset = {12, -3};
    const SpriteOffset crouch_offset = {12, -1};

    SpriteOffset carry_offset = (hero->ducking ? crouch_offset : stand_offset);

    if (hero->direction == LEFT) {
        carry_offset.x = -carry_offset.x - 4;
    }

    SpritePosition position = hero->position;
    position.x += carry_offset.x;
    position.y += carry_offset.y;

    HeroSpriteCarryContext context = {
        .position = position,
        .direction = hero->direction
    };

    return context;
}

void hero_platform_interaction(Hero *hero, SpriteActor *platform) {
    if (sa_has_hero_collision(platform, hero)) {
        bool riding = sa_platform_ride_check(platform, &hero->velocity, &hero->position, &hero->platform_sprite_handle);

        if (riding) {
            hero->velocity.y = 0;
        }
    } else if (sa_handle_equal(hero->platform_sprite_handle, platform->handle)) {
        sa_platform_dismount(platform, &hero->velocity, &hero->platform_sprite_handle);
    }
}

// Interactive events which set a pending event to be handled on next state update

void hero_set_airborne_bounce(Hero *hero) {
    hero->airborne_bounce_pending = true;
}

void hero_damage(Hero *hero) {
    if (!hero->invulnerability_counter) {
        hero->damage_pending = true;
    }
}

// Vehicles:

void hero_enter_vehicle(Hero *hero, SpriteActor *actor) {
    if (hero_in_vehicle(hero, actor)) {
        return;
    }

    hero->vehicle_sprite_handle = actor->handle;
    hero->pending_vehicle_entry = true;
}

bool hero_vehicle_control_state(const Hero *hero, HeroVehicleControl *control) {
    const PadInputDecoded *pad = &hero->pad;
    const PadInputDecoded *pad_edge = &hero->pad_edge;

    if (pad_edge->a || pad_edge->l) {
        control->eject = true;
        return true;
    }

    control->move_fast = pad->y;

    control->eject = false;
    control->action = pad_edge->x || pad_edge->r;
    control->move_left = pad->left;
    control->move_right = pad->right;
    control->jump = pad_edge->b && hero->grounded;

    return (control->action || control->move_left || control->move_right);
}

bool hero_in_vehicle(const Hero *hero, const SpriteActor *actor) {
    return sa_handle_equal(hero->vehicle_sprite_handle, actor->handle);
}

bool hero_in_any_vehicle(const Hero *hero) {
    return sa_handle_live(hero->vehicle_sprite_handle);
}

// Misc., needs grouping

bool hero_has_draw_priority(const Hero *hero) {
    // Hero only appears above other sprites if it's "inside" a vehicle
    return !sa_handle_live(hero->vehicle_sprite_handle);
}

static void precompute_bounding_box(Hero *hero) {
    const SpriteBoundingBox *box;
    if (hero_in_any_vehicle(hero)) {
        box = &hero->vehicle_context.sprite_box;
    } else {
        box = hero_sprite_bounding_box(hero);
    }

    sa_bounding_box_abs(&hero->position, box, &hero->bounding_box_abs);
}

static bool riding_platform(Hero *hero) {
    if (sa_handle_live(hero->platform_sprite_handle)) {
        return true;
    } else {
        // Platform doesn't exist (anymore)
        sa_handle_clear(&hero->platform_sprite_handle);
        return false;
    }
}

// Block behaviour processing

static void collect_coin(Hero *hero, const SpriteBlockPositionedInteraction *interaction, const Camera *camera);

static void perform_extra_block_interaction(Hero *hero, const SpriteBlockInteractionResult *result, HeroBlockAttributesProcessedContext *context, const Camera *camera) {
    // For the two attributes..
    for (uint32_t i = 0; i < 2; i++) {
        const SpriteBlockPositionedInteraction *interaction = &result->interactions[i];
        BlockInteractionAttributes attributes = interaction->attributes;

        if (attributes & BLOCK_CLIMB) {
            context->vine = true;
            return;
        }
        if (attributes & BLOCK_DAMAGE) {
            context->hurt = true;
            return;
        }
        if (attributes & BLOCK_COIN) {
            collect_coin(hero, interaction, camera);
            return;
        }
    }
}

// Only do this processing ONCE per frame regardless of how many block interactions are run
// i.e. don't want to "double climb" a vine or anything else that influences hero position

static void update_from_block_interaction(Hero *hero, const HeroBlockAttributesProcessedContext *context) {
    if (context->vine) {
        bool should_climb = hero->pad.up;
        should_climb &= !sa_handle_live(hero->carried_sprite_handle);
        should_climb &= !hero_in_any_vehicle(hero);

        if (should_climb) {
            hero->climbing = true;
        }
    } else {
        hero->climbing = false;
    }

    if (context->hurt) {
        hero_damage(hero);
    }
}

static void collect_coin(Hero *hero, const SpriteBlockPositionedInteraction *interaction, const Camera *camera) {
    // (Eventually adding gameplay effect of collecting coins)
    hero->coins = MIN(99, hero->coins + 1);

    erase_block(interaction->x, interaction->y, camera);

    // Glitter effect
    SpritePosition position = {
        .x = (interaction->x & ~0xf) + 8,
        .y = (interaction->y & ~0xf) + 8
    };
    glitter_sprite_init(&position);

    se_coin();
}

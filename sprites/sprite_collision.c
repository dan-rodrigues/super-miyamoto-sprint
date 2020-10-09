#include "sprite_collision.h"

#include "hero.h"
#include "sprite_actor.h"
#include "impact_sprite.h"
#include "sprite_loading.h"
#include "sound_effects.h"

static bool has_collision(const SpritePosition *p1, const SpriteBoundingBox *b1,
                          const SpritePosition *p2, const SpriteBoundingBox *b2);

static bool has_collision_1_abs(const SpriteBoundingBoxAbs *p1,
                                const SpritePosition *p2, const SpriteBoundingBox *b2);

static bool has_collision_2_abs(const SpriteBoundingBoxAbs *p1,
                                const SpriteBoundingBoxAbs *p2);

static void projectile_kill(SpriteActor *actor, const SpriteEnvironment *env);

bool sa_has_hero_collision(const SpriteActor *actor, const Hero *hero) {
    if (hero->dead) {
        return false;
    }

    return has_collision_1_abs(&hero->bounding_box_abs, &actor->position, &actor->bounding_box);
}

bool sa_has_collision(const SpriteActor *s1, const SpriteActor *s2) {
    const SpriteBoundingBox b1 = s1->bounding_box;
    const SpriteBoundingBox b2 = s2->bounding_box;

    return has_collision(&s1->position, &b1, &s2->position, &b2);
}

SpriteHeroCollisionResult sa_hero_standard_collision(SpriteActor *actor, const SpriteEnvironment *env) {
    Hero *hero = env->hero;

    HeroSpriteCarryUpdateResult carry_result = hero_sprite_carry_update(hero, actor);
    bool carrying = (carry_result == HERO_SPRITE_CARRY_UPDATE_CARRYING);

    if (carrying) {
        actor->carried = true;
        return SA_HERO_COLLISION_CARRYING;
    } else {
        actor->carried = false;
    }

    bool kicked = (carry_result == HERO_SPRITE_CARRY_UPDATE_KICKED);
    kicked |= (carry_result == HERO_SPRITE_CARRY_UPDATE_KICKED_UP);

    if (kicked) {
        // Draw impact sprite at the kick point
        HeroSpriteCarryContext carry_context = hero_sprite_carry_context(hero);
        impact_sprite_init(&carry_context.position);
        se_stomp();
        
        if (carry_result == HERO_SPRITE_CARRY_UPDATE_KICKED) {
            return SA_HERO_COLLISION_CARRIED_KICK;
        } else {
            return SA_HERO_COLLISION_CARRIED_KICK_UP;
        }
    }

    if (!sa_has_hero_collision(actor, hero)) {
        return SA_HERO_COLLISION_NONE;
    }

    // Short sprites can hurt hero without this check
    bool hero_high_enough = true;
    if (actor->bounding_box.size.height >= 16) {
        hero_high_enough = hero->position.y < (actor->position.y - (actor->bounding_box.size.height / 2));
    }

    bool hero_falling = hero->velocity.y > 0;

    if (actor->stompable && !actor->stomp_hurts_hero && (hero_high_enough && hero_falling)) {
        // Hero wins

        if (actor->dies_upon_stomp) {
            sa_kill_sprite(actor, env);
        }

        hero_set_airborne_bounce(hero);
        impact_sprite_init(&hero->position);
        se_stomp();

        return SA_HERO_COLLISION_STOMP;
    } else if (actor->touch_hurts_hero) {
        // Enemy wins

        hero_damage(hero);

        if (hero->velocity.y > 0) {
            hero->damage_causes_hop |= actor->stomp_hurts_hero;
        }

        return SA_HERO_COLLISION_HURT;
    } else {
        return SA_HERO_COLLISION_INTERSECT;
    }
}

void sa_kill_sprite(SpriteActor *actor, const SpriteEnvironment *env) {
    actor->killed = true;
    sprite_level_data_prevent_index_reloading(env->loading_context, actor->level_data_index);
}

// Handle platform ride flag setting for other sprites
// This is assumed to run before the main actor code does

void sa_other_sprite_platform_check(SpriteActor *platform) {
    // Precompute platform's absolute bounding box
    SpriteBoundingBoxAbs box_abs;
    sa_bounding_box_abs(&platform->position, &platform->bounding_box, &box_abs);

    for (uint32_t i = 0; i < SPRITE_ACTOR_MAX; i++) {
        SpriteActor *other = sa_index(i);
        if (!other || !other->can_ride || other->killed) {
            continue;
        }
        if (!has_collision_1_abs(&box_abs, &other->position, &other->bounding_box)) {
            if (sa_handle_equal(other->ridden_sprite_handle, platform->handle)) {
                sa_handle_clear(&other->ridden_sprite_handle);
            }

            continue;
        }

        bool riding = sa_platform_ride_check(platform,
                                             &other->velocity,
                                             &other->position,
                                             &other->ridden_sprite_handle);

        if (riding) {
            sa_grounded_update(other, 0);
        }
    }
}

// Handle sprite-to-sprite collision assuming the caller is "solid"
// This allows sprites to bump into each other and turn away

bool sa_other_sprite_collision(SpriteActor *actor, const SpriteEnvironment *env) {
    if (actor->killed || !actor->interacts_with_sprites) {
        return false;
    }

    // Precompute caller's absolute bounding box
    SpriteBoundingBoxAbs box_abs;
    sa_bounding_box_abs(&actor->position, &actor->bounding_box, &box_abs);

    for (uint32_t i = 0; i < SPRITE_ACTOR_MAX; i++) {
        SpriteActor *other = sa_index(i);
        if (!other || !other->interacts_with_sprites || other->killed) {
            continue;
        }
        if (actor->handle.index == other->handle.index) {
            continue;
        }
        if (!has_collision_1_abs(&box_abs, &other->position, &other->bounding_box)) {
            continue;
        }

        if (!other->immune_to_projectiles && (actor->carried || actor->kills_other_sprites)) {
            // Impact sprite to be drawn at center of overlapping region
            SpriteBoundingBoxAbs overlap_box, other_box_abs;
            sa_bounding_box_abs(&other->position, &other->bounding_box, &other_box_abs);
            sa_bounding_box_overlap(&box_abs, &other_box_abs, &overlap_box);

            SpritePosition overlap_center;
            sa_bounding_box_center(&overlap_box, &overlap_center);

            impact_sprite_init(&overlap_center);

            projectile_kill(other, env);
            se_stomp();

            // If the other sprite is also killer, this one dies too
            // Some sprites may be carryable but not killed in this case
            if (actor->carried || other->carried || other->kills_other_sprites) {
                projectile_kill(actor, env);
            }

            continue;
        }

        // The bottom direction reversal is only done by one sprite, not both
        if (actor->handle.index > other->handle.index) {
            continue;
        }

        // Certain other sprites shouldn't be turned around
        if (other->kills_other_sprites) {
            // Don't turn a killer sprite around, it'll probably kill this one later
            continue;
        }

        // There's a collision between 2 sprites
        // Update direction according to current positions
        // This is benign if the directions are already correct

        bool actor_goes_left = actor->position.x <= other->position.x;
        actor->direction = (actor_goes_left ? LEFT : RIGHT);
        other->direction = (!actor_goes_left ? LEFT : RIGHT);

        bool invert_actor_velocity = (actor_goes_left != (actor->velocity.x < 0));
        bool invert_other_velocity = (actor_goes_left != (other->velocity.x > 0));

        if (invert_actor_velocity) {
            actor->velocity.x = -actor->velocity.x;
        }
        if (invert_other_velocity) {
            other->velocity.x = -other->velocity.x;
        }
    }

    return false;
}

bool sa_light_sprite_collision(SpriteActorLight *actor, const SpriteEnvironment *env) {
    // Precompute caller's absolute bounding box
    SpriteBoundingBoxAbs box_abs;
    sa_bounding_box_abs(&actor->position, &actor->bounding_box, &box_abs);

    for (uint32_t i = 0; i < SPRITE_ACTOR_MAX; i++) {
        SpriteActor *other = sa_index(i);
        if (!other || !other->interacts_with_sprites || other->killed) {
            continue;
        }
        if (!has_collision_2_abs(&box_abs, &other->bounding_box_abs)) {
            continue;
        }
        if (other->immune_to_projectiles) {
            continue;
        }

        projectile_kill(other, env);

        return true;
    }

    return false;
}

bool sa_hero_standard_collision_light(SpriteActorLight *actor, Hero *hero) {
    if (hero->dead) {
        return false;
    }

    if (!actor->hurts_hero) {
        return false;
    }
    if (!has_collision_1_abs(&hero->bounding_box_abs, &actor->position, &actor->bounding_box)) {
        return false;
    }

    hero_damage(hero);

    return true;
}

static bool has_collision(const SpritePosition *p1, const SpriteBoundingBox *b1,
                          const SpritePosition *p2, const SpriteBoundingBox *b2)
{
    SpriteBoundingBoxAbs p1_abs;
    sa_bounding_box_abs(p1, b1, &p1_abs);

    return has_collision_1_abs(&p1_abs, p2, b2);
}

static bool has_collision_1_abs(const SpriteBoundingBoxAbs *p1,
                                const SpritePosition *p2, const SpriteBoundingBox *b2)
{
    int32_t s2_top = p2->y + b2->offset.y;
    int32_t s2_bottom = p2->y + b2->offset.y + b2->size.height;
    int32_t s2_left = p2->x + b2->offset.x;
    int32_t s2_right = p2->x + b2->offset.x + b2->size.width;

    return
        p1->left < s2_right &&
        p1->right > s2_left &&
        p1->top < s2_bottom &&
        p1->bottom > s2_top;
}

static bool has_collision_2_abs(const SpriteBoundingBoxAbs *p1,
                                const SpriteBoundingBoxAbs *p2)
{
    return
        p1->left < p2->right &&
        p1->right > p2->left &&
        p1->top < p2->bottom &&
        p1->bottom > p2->top;
}

static void projectile_kill(SpriteActor *actor, const SpriteEnvironment *env) {
    const int32_t kill_launch_speed = Q_1;
    actor->velocity.y = -kill_launch_speed;

    actor->falls_off_when_killed = true;
    sa_kill_sprite(actor, env);
}

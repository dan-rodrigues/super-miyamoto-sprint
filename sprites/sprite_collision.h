#ifndef sprite_collision_h
#define sprite_collision_h

#include <stdbool.h>

#include "sprite_collision_types.h"
#include "sprite_actor_types.h"
#include "hero_types.h"

enum SpriteHeroCollisionResult {
    SA_HERO_COLLISION_NONE = 0,
    SA_HERO_COLLISION_STOMP,
    SA_HERO_COLLISION_HURT,
    SA_HERO_COLLISION_CARRYING,

    SA_HERO_COLLISION_CARRIED_KICK,
    SA_HERO_COLLISION_CARRIED_KICK_UP,

    SA_HERO_COLLISION_INTERSECT
};

bool sa_has_collision(const SpriteActor *s1, const SpriteActor *s2);
bool sa_has_hero_collision(const SpriteActor *actor, const Hero *hero);

SpriteHeroCollisionResult sa_hero_standard_collision(SpriteActor *actor, Hero *hero);

bool sa_other_sprite_collision(SpriteActor *actor);
void sa_other_sprite_platform_check(SpriteActor *platform);

bool sa_light_sprite_collision(SpriteActorLight *actor);
bool sa_hero_standard_collision_light(SpriteActorLight *actor, Hero *hero);

void sa_kill_sprite(SpriteActor *actor);

#endif /* sprite_collision_h */

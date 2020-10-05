#include "tank_driver_sprite.h"

#include "sprite_actor.h"
#include "sprite_collision.h"
#include "sprite_drawing.h"

static void draw(SpriteActor *self, const SpriteEnvironment *env);
static bool driven_tank_liveness_check(SpriteActor *actor);

void tank_driver_sprite_main(SpriteActor *self, const SpriteEnvironment *env) {
    if (!driven_tank_liveness_check(self)) {
        sa_free(self);
        return;
    }

    sa_hero_standard_collision(self, env->hero);
    sa_perform_default_movement(self);
    draw(self, env);
}

static bool tank_driven_by_driver(SpriteActor *other, SpriteActor *self) {
    // Bit dirty comparing the main function directly, can abstract to some "actor ID" if needed
    if ((uintptr_t)other->main != (uintptr_t)tank_sprite_main) {
        return false;
    }

    return sa_handle_equal(other->tank.driver_enemy_handle, self->handle);
}

static bool driven_tank_liveness_check(SpriteActor *self) {
    return sa_iterate_all(self, tank_driven_by_driver);
}

static void draw(SpriteActor *self, const SpriteEnvironment *env) {
    const uint8_t palette = 12;
    const uint16_t live_tile = 0x100;
    const uint16_t killed_tile = 0x120;

    uint16_t tile = (self->killed ? killed_tile : live_tile);

    sa_draw_standard_16x16(self, env, tile, palette);
}

SpriteActor *tank_driver_sprite_init(const SpritePosition *position) {
    SpriteActor *actor = sa_alloc();
    sa_init(actor, tank_driver_sprite_main);

    actor->position = *position;
    actor->direction = LEFT;

    actor->falls_off_when_killed = true;
    actor->dies_upon_stomp = true;
    actor->stompable = true;

    static const SpriteBoundingBox sprite_box = {
        .offset = { -7, -14 },
        .size = { 13, 18 }
    };
    actor->bounding_box = sprite_box;

    return actor;
}

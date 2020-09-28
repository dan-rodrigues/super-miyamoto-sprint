#include "tank_driver_sprite.h"

#include "sprite_actor.h"
#include "sprite_collision.h"
#include "sprite_drawing.h"

static void draw(SpriteActor *self, const SpriteEnvironment *env);

void tank_driver_sprite_main(SpriteActor *self, const SpriteEnvironment *env) {
    sa_hero_standard_collision(self, env->hero);
    sa_perform_default_movement(self);
    draw(self, env);
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
        .offset = { -6, -12 },
        .size = { 12, 12 }
    };
    actor->bounding_box = sprite_box;

    return actor;
}

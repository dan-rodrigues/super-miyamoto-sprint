#ifndef hero_drawing_h
#define hero_drawing_h

#include <stdint.h>

#include "hero_types.h"

#include "camera.h"

void draw_hero_sprites(Hero *hero, int16_t sprite_tile, Camera *camera);

#endif /* hero_drawing_h */

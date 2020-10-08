#ifndef hero_drawing_h
#define hero_drawing_h

#include <stdint.h>

#include "hero_types.h"

#include "camera.h"

void hero_draw(Hero *hero, int16_t sprite_tile, const Camera *camera);

typedef struct {
    uint8_t x, y;
    const uint16_t *top_row, *bottom_row;
} HeroTileArrangement;
void hero_16x16_arrangements(HeroFrame frame, HeroTileArrangement *arrangements, size_t limit);

#endif /* hero_drawing_h */

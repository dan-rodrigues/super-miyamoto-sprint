#ifndef hero_life_meter_h
#define hero_life_meter_h

#include <stdint.h>

#include "hero_types.h"

void hero_draw_life_meter(const Hero *hero);
void hero_draw_draw_coin_counter(const Hero *hero);
void hero_increase_hit_point_max(Hero *hero);

#endif /* hero_life_meter_h */

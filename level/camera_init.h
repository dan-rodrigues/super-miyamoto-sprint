#ifndef camera_init_h
#define camera_init_h

#include "camera.h"
#include "hero_types.h"

void camera_init(Camera *camera, const Hero *hero, const uint16_t *block_map);

#endif /* camera_init_h */

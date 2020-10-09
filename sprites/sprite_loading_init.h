#ifndef sprite_loading_init_h
#define sprite_loading_init_h

#include <stdint.h>
#include <stddef.h>

#include "sprite_loading_types.h"
#include "camera_types.h"
#include "hero_types.h"

void sprite_level_data_init(SpriteLoadingContext *context,
                            const void *sprite_data,
                            size_t sprite_data_length);

void sprite_level_data_perform_initial_load(SpriteLoadingContext *context,
                                            const Camera *camera,
                                            const Hero *hero);

#endif /* sprite_loading_init_h */

#ifndef sprite_loading_h
#define sprite_loading_h

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "camera_types.h"
#include "hero_types.h"

extern const int8_t SPRITE_INDEX_UNDEFINED;

void sprite_level_data_init(const void *sprite_data, size_t sprite_data_length);

void sprite_level_data_perform_initial_load(const Camera *camera, const Hero *hero);
void sprite_level_data_load_new(const Camera *camera, const Hero *hero);
void sprite_level_data_prevent_index_reloading(int8_t level_index);

#endif /* sprite_loading_h */

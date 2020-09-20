#ifndef vram_level_update_h
#define vram_level_update_h

#include <stdint.h>

void vram_upload_level_column(uint16_t map_base, const uint16_t *block_map, uint16_t x);

#endif /* vram_level_update_h */

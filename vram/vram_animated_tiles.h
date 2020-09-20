#ifndef vram_animated_tiles_h
#define vram_animated_tiles_h

#include <stdint.h>

void vram_queue_animations(void);
void vram_queue_animated_frame_raw(uint16_t vram_base, uint16_t source_tile_base);

#endif /* vram_animated_tiles_h */

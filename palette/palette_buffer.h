#ifndef palette_buffer_h
#define palette_buffer_h

#include <stdint.h>

void pb_preload_bg_color(uint16_t color);
void pb_preload_single_color(uint8_t color_id, uint16_t color);
void pb_preload_single_palette(uint8_t palette_id, const uint16_t *data);

void pb_alpha_mask_all(uint8_t alpha);

void pb_lerp_palette_to_white(uint8_t palette_id, uint16_t mask, uint8_t step);
void pb_lerp_to_white(uint8_t color_id, uint8_t step);

void pb_queue_all(void);
void pb_upload(void);
void pb_reset(void);

#endif /* palette_buffer_h */

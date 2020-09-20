#ifndef sprite_buffer_h
#define sprite_buffer_h

#include <stdint.h>

void sb_reset(void);
void sb_write(uint32_t x, uint32_t y, uint32_t g);
void sb_write_priority(uint32_t x, uint32_t y, uint32_t g);
void sb_upload(void);

#endif /* sprite_buffer_h */

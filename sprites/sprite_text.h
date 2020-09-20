#ifndef sprite_text_h
#define sprite_text_h

#include <stdint.h>

void st_write(const char *text, uint32_t x, uint32_t y);
void st_write_decimal(uint8_t value, uint32_t x, uint32_t y);

#endif /* sprite_text_h */

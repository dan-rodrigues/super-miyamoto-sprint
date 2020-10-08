#ifndef sprite_text_attributes_h
#define sprite_text_attributes_h

#include <stdbool.h>
#include <stdint.h>

extern const uint8_t ST_BASE_TILE_BLANK;

typedef struct {
    uint8_t base_tile;
    uint8_t width : 4;
    bool wide : 1;
} SpriteTextCharAttributes;

const SpriteTextCharAttributes *st_char_attributes(char c);

#endif /* sprite_text_attributes_h */

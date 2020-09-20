#ifndef debug_print_h
#define debug_print_h

#include <stdint.h>

#include "hero_types.h"
#include "camera.h"
#include "vram_layout.h"
#include "sprite_actor_types.h"

void dbg_print_init(void);
void dbg_print_status(uint8_t palette_id, Hero *hero, Camera *camera);

void dbg_printf(uint8_t x, uint8_t y, const char *fmt, ...);

void dbg_print_actor(const SpriteActor *actor);
void dbg_actor_clear(const SpriteActor *actor);

#endif /* debug_print_h */

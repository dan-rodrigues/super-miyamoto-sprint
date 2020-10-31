#ifndef palette_lerp_task_h
#define palette_lerp_task_h

#include <stdint.h>

#include "extra_task_types.h"

typedef enum {
    PALETTE_LERP_TO,
    PALETTE_LERP_FROM
} PaletteLerpType;

typedef struct {
    uint8_t palette_id;
    uint16_t color_id_mask;
    uint8_t fade_step;
    PaletteLerpType type;
} PaletteLerpTask;

ExtraTask *palette_lerp_task_init(uint8_t palette_id, uint16_t mask);
GameLoopAction palette_lerp_task_main(ExtraTask *self);

#endif /* palette_lerp_task_h */

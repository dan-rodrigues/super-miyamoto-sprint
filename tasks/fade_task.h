#ifndef fade_task_h
#define fade_task_h

#include <stdint.h>

#include "extra_task_types.h"

typedef enum {
    FADE_OUT,
    FADE_IN
} FadeTaskType;

typedef struct {
    uint8_t fade_step;
    FadeTaskType type;
} FadeTask;

ExtraTask *fade_task_init(FadeTaskType type);
GameLoopAction fade_task_main(ExtraTask *self);

#endif /* fade_task_h */

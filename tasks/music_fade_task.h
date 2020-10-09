#ifndef music_fade_task_h
#define music_fade_task_h

#include <stdint.h>

#include "extra_task_types.h"
#include "music.h"

typedef struct {
    int32_t fade_step;
    const Track *next_track;
    GameLoopAction final_action;
} MusicFadeTask;

ExtraTask *music_fade_task_init(void);
GameLoopAction music_fade_task_main(ExtraTask *self);

#endif /* music_fade_task_h */

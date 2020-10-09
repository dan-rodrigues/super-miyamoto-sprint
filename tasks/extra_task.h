#ifndef extra_task_h
#define extra_task_h

#include <stdint.h>
#include <stdbool.h>

#include "extra_task_types.h"
#include "extra_task_handle.h"

#include "game_loop_types.h"

#include "fade_task.h"
#include "level_reload_sequence_task.h"
#include "palette_lerp_task.h"
#include "music_fade_task.h"

struct ExtraTask {
    ExtraTaskMain main;
    ExtraTaskHandle handle;

    union {
        FadeTask fade;
        PaletteLerpTask palette_lerp;
        LevelReloadSequenceTask level_reload_sequence;
        MusicFadeTask music_fade;
    };
};

void et_reset(void);

ExtraTask *et_alloc(void);
void et_free(ExtraTask *task);
void et_cancel(ExtraTaskHandle handle);

bool et_live(const ExtraTask *task);
bool et_handle_live(ExtraTaskHandle handle);

GameLoopAction et_run(void);

#endif /* extra_task_h */

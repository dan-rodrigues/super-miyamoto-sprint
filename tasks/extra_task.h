#ifndef extra_task_h
#define extra_task_h

#include <stdint.h>
#include <stdbool.h>

#include "extra_task_types.h"
#include "extra_task_handle.h"

#include "game_loop_types.h"

#include "fade_task.h"
#include "level_reload_sequence_task.h"

struct ExtraTask {
    ExtraTaskMain main;
    ExtraTaskHandle handle;

    union {
        FadeTask fade;
        LevelReloadSequenceTask level_reload_sequence;
    };
};

void et_reset(void);

ExtraTask *et_alloc(void);
void et_free(ExtraTask *task);
bool et_live(const ExtraTask *task);

GameLoopAction et_run(void);

#endif /* extra_task_h */

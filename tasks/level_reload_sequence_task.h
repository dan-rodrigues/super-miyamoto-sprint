#ifndef level_reload_sequence_task_h
#define level_reload_sequence_task_h

#include <stdint.h>

#include "extra_task_types.h"

typedef enum {
    LEVEL_RELOAD_SEQUENCE_INITIAL,
    LEVEL_RELOAD_SEQUENCE_DELAY,
    LEVEL_RELOAD_SEQUENCE_FADE,
    LEVEL_RELOAD_SEQUENCE_LOAD
} LevelReloadSequenceState;

typedef struct {
    LevelReloadSequenceState state;
    ExtraTask *current_subtask;
    uint8_t delay_counter;
} LevelReloadSequenceTask;

ExtraTask *level_reload_sequence_task_init(void);
GameLoopAction level_reload_sequence_task_main(ExtraTask *self);

#endif /* level_reload_sequence_task_h */

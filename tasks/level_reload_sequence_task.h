#ifndef level_reload_sequence_task_h
#define level_reload_sequence_task_h

#include <stdint.h>

#include "extra_task_types.h"

typedef enum {
    LEVEL_RELOAD_SEQUENCE_DELAY,
    LEVEL_RELOAD_SEQUENCE_FADE,
    LEVEL_RELOAD_SEQUENCE_LOAD
} LevelReloadSequenceState;

typedef struct {
    LevelReloadSequenceState state;
    ExtraTask *current_subtask;
    uint8_t delay_counter;
    uint16_t fade_palette_mask;
    GameLoopAction final_action;
} LevelReloadSequenceTask;

ExtraTask *level_reload_sequence_task_init(uint16_t delay);
GameLoopAction level_reload_sequence_task_main(ExtraTask *self);

#endif /* level_reload_sequence_task_h */
